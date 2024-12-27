#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/time.h>

// Информация о модуле
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TSU");
MODULE_DESCRIPTION("Лабораторная 4. Падение Тунгусского метеорита");

// Временная метка падения 17 июня 1908 г. 07:00 (Красноярск) 00:00 (UTC)
// Перевод в UNIX TIME: date -d "1908-06-17 00:00:00 UTC" +%s
#define TUNGUSKA_TIMESTAMP -1942099200

// Функция для вычисления прошедших часов
static long hours_since_tunguska(void)
{
    struct timespec64 current_time;
    ktime_get_real_ts64(&current_time);
    return (current_time.tv_sec - TUNGUSKA_TIMESTAMP) / 3600;
}

// Инициализация модуля
static int __init tgu_module_init(void)
{
    long hours = hours_since_tunguska();
    printk(KERN_INFO "Hours since Tunguska Event: %ld\n", hours);
    return 0;
}

// Завершение работы модуля
static void __exit tgu_module_exit(void)
{
    printk(KERN_INFO "Tomsk State University forever!\n");
}

// Регистрация функций инициализации и завершения
module_init(tgu_module_init);
module_exit(tgu_module_exit);
