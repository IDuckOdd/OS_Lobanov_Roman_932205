#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define PORT 3000 // Порт
#define CON 5 // Кол-во подключений

volatile sig_atomic_t wasSigHup = 0;

void sigHupHandler(int sigNumber) {
    wasSigHup = 1;
}

int main() {
    int serverFD; // Файловый дескриптор для сокета
    int incomingSocketFD = 0; // Файловый дескриптор для входящих соединений
    struct sockaddr_in socketAddress; // Хранение адреса сервера
    int addressLength = sizeof(socketAddress);
    fd_set readfds; // Набор файловых дескрипторов pselect
    struct sigaction sa; // Настройка обработчика сигнала
    sigset_t blockedMask, origMask; // Маски сигналов блокировки и восстановления
    char buffer[1024] = { 0 }; // Буффер чтения данных
    int readBytes; // Кол-во считанных байт
    int maxSd; // Максимальный дескриптор

    // Создание серверного сокета
    if ((serverFD = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("create error");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    socketAddress.sin_family = AF_INET; // IPv4
    socketAddress.sin_addr.s_addr = INADDR_ANY; // Сервер слушает на всех доступных интерфейсах
    socketAddress.sin_port = htons(PORT); // Порт сервера

    // Привязка сокета к адресу
    if (bind(serverFD, (struct sockaddr*)&socketAddress, sizeof(socketAddress)) < 0) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    // Настройка прослушивания порта
    if (listen(serverFD, CON) < 0) {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d \n", PORT);

    // Обработка SIGHUP
    sigaction(SIGHUP, NULL, &sa); // Функция обработчика
    sa.sa_handler = sigHupHandler;
    sa.sa_flags |= SA_RESTART; // Продолжение после обработки
    sigaction(SIGHUP, &sa, NULL);

    // Блокировка сигнала SIGHUP
    sigemptyset(&blockedMask);
    sigemptyset(&origMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

    // Главный цикл
    while (1) { // Убираем ограничение на количество подключений
        FD_ZERO(&readfds); // Набор дескрипторов
        FD_SET(serverFD, &readfds);

        if (incomingSocketFD > 0) {
            FD_SET(incomingSocketFD, &readfds);
        }

        maxSd = (incomingSocketFD > serverFD) ? incomingSocketFD : serverFD; // Максимальный из pselect

        // Вызов pselect
        if (pselect(maxSd + 1, &readfds, NULL, NULL, NULL, &origMask) == -1) { // Блокирует пока не появится событие
            if (errno != EINTR) {
                perror("pselect error");
                exit(EXIT_FAILURE);
            }
            //
            if (errno == EINTR){
                if(wasSigHup == 1){
                    printf("SIGHUP received.\n");
                    wasSigHup = 0;
                }
                continue;
            }
        }

        if (incomingSocketFD > 0 && FD_ISSET(incomingSocketFD, &readfds)) {
            readBytes = read(incomingSocketFD, buffer, sizeof(buffer));

            if (readBytes > 0) {
                printf("Received data: %d bytes\n", readBytes);
            } else {
                if (readBytes == 0) {
                    close(incomingSocketFD);
                    incomingSocketFD = 0;
                } 
                else {
                    perror("read error");
                }// Убираем увеличение счетчика подключений
            }
            continue;
        }

        if (FD_ISSET(serverFD, &readfds)) {
            if ((incomingSocketFD = accept(serverFD, (struct sockaddr*)&socketAddress, (socklen_t*)&addressLength)) < 0) {
                perror("accept error");
                exit(EXIT_FAILURE);
            }

            printf("New connection.\n");
        }
    }

    close(serverFD);

    return 0;
}