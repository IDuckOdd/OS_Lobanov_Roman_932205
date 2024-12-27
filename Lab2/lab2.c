#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>

#define PORT 12345
#define MAX_CONNECTIONS 1

// Глобальные переменные
volatile sig_atomic_t signal_received = 0; // Переменная отслеживание получения SIGHUP

// Обработчик сигнала SIGHUP
void signal_handler(int sig) {
    signal_received = 1;
    printf("Received signal: %d\n", sig);
}

// Функция для работы с одним соединением
void handle_connection(int client_fd) {
    char buffer[1024];
    ssize_t bytes_received;

    // Кол-во полученных байт
    while ((bytes_received = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
        printf("Received %ld bytes\n", bytes_received);
    }

    // Проверка соединения
    if (bytes_received == 0) {
        printf("Client disconnected\n");
    } else if (bytes_received < 0) {
        perror("recv failed");
    }

    close(client_fd);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    fd_set readfds;
    int nfds;
    struct timespec timeout;

    // Настройка обработки сигнала SIGHUP
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGHUP, &sa, NULL);

    // Создание серверный сокет
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { // Работа с TCP
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Настройка адрес и порт
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // Слушает все интерфейсы
    server_addr.sin_port = htons(PORT);

    // Привязываем сокет к адресу
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Начинаем прослушивание
    if (listen(server_fd, MAX_CONNECTIONS) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // Делает сервер неблокирующим
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    printf("Server is listening on port %d...\n", PORT);

    // Главный цикл сервера
    while (1) {
        // Настроим fd_set для pselect
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        nfds = server_fd + 1;  // Максимальный дескриптор + 1

        // Устанавливаем таймаут в 1 секунду
        timeout.tv_sec = 1;
        timeout.tv_nsec = 0;

        // Ожидаем событий
        int ready = pselect(nfds, &readfds, NULL, NULL, &timeout, NULL);

        if (ready == -1) {
            perror("pselect failed");
            exit(EXIT_FAILURE);
        }

        // Проверим, был ли сигнал SIGHUP
        if (signal_received) {
            printf("Handling signal SIGHUP...\n");
            signal_received = 0;
        }

        // Принять новое соединение
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        printf("New connection from %s:%d\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Обрабатываем одно соединение
        handle_connection(client_fd);

        // Закрываем соединение после обработки
        close(client_fd);
    }

    // Закрытие серверного сокета
    close(server_fd);

    return 0;
}
