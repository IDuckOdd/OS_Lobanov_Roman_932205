#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

// Информация о модуле
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TSU");
MODULE_DESCRIPTION("Лабораторная 1. Модуль ядра Linux");

// Инициализация модуля
static int __init tgu_module_init(void)
{
    printk(KERN_INFO "Welcome to the Tomsk State University\n");
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