#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

// Информация о модуле
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TSU");
MODULE_DESCRIPTION("Лабораторная 4. Падение Тунгусского метеорита");

// Временная метка падения 17 июня 1908 г. 07:00 (Красноярск) 00:00 (UTC)
// Перевод в UNIX TIME: date -d "1908-06-17 00:00:00 UTC" +%s
#define TUNGUSKA_TIMESTAMP -1942099200

// Имя файла в procfs
#define PROC_FILENAME "tsulab"

// Функция для вычисления прошедших часов
static long hours_since_tunguska(void)
{
    struct timespec64 current_time;
    ktime_get_real_ts64(&current_time);
    return (current_time.tv_sec - TUNGUSKA_TIMESTAMP) / 3600;
}

// Функция для вывода данных /proc/tsulab
static int tgu_proc_show(struct seq_file *m, void *v)
{
    long hours = hours_since_tunguska();
    seq_printf(m, "Hours since Tunguska Event: %ld\n", hours);
    return 0;
}

// Функция открытия файла /proc/tsulab
static int tgu_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, tgu_proc_show, NULL);
}

// Описание операций с файлом
static const struct proc_ops tgu_proc_ops = {
    .proc_open    = tgu_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

// Инициализация модуля
static int __init tgu_module_init(void)
{
    proc_create(PROC_FILENAME, 0, NULL, &tgu_proc_ops); // Создание файла
    printk(KERN_INFO "TSU: Module initialized.\n");
    return 0;
}

// Завершение работы модуля
static void __exit tgu_module_exit(void)
{
    remove_proc_entry(PROC_FILENAME, NULL); //Удаление файла
    printk(KERN_INFO "Tomsk State University forever!\n");
}

// Регистрация функций инициализации и завершения
module_init(tgu_module_init);
module_exit(tgu_module_exit);
