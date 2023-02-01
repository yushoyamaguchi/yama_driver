#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <asm/current.h>
#include <asm/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");

/* /proc/devices等で表示されるデバイス名 */
#define DRIVER_NAME "MyDevice"

/* このデバイスドライバで使うマイナー番号の開始番号と個数(=デバイス数) */
static const unsigned int MINOR_BASE = 0;
static const unsigned int MINOR_NUM  = 2;   /* マイナー番号は 0 ~ 1 */

/* このデバイスドライバのメジャー番号(動的に決める) */
static unsigned int mydevice_major;

/* キャラクタデバイスのオブジェクト */
static struct cdev mydevice_cdev;

/* open時に呼ばれる関数 */
static int mydevice_open(struct inode *inode, struct file *file)
{
    printk("mydevice_open");
    return 0;
}

/* close時に呼ばれる関数 */
static int mydevice_close(struct inode *inode, struct file *file)
{
    printk("mydevice_close");
    return 0;
}

/* read時に呼ばれる関数 */
static ssize_t mydevice_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    printk("mydevice_read");
    buf[0] = 'A';
    return 1;
}

/* write時に呼ばれる関数 */
static ssize_t mydevice_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    printk("mydevice_write");
    return 1;
}

/* 各種システムコールに対応するハンドラテーブル */
struct file_operations s_mydevice_fops = {
    .open    = mydevice_open,
    .release = mydevice_close,
    .read    = mydevice_read,
    .write   = mydevice_write,
};

/* ロード(insmod)時に呼ばれる関数 */
static int mydevice_init(void)
{
    printk("mydevice_init\n");

    int alloc_ret = 0;
    int cdev_err = 0;
    dev_t dev;

    /* 1. 空いているメジャー番号を確保する */
    alloc_ret = alloc_chrdev_region(&dev, MINOR_BASE, MINOR_NUM, DRIVER_NAME);
    if (alloc_ret != 0) {
        printk(KERN_ERR  "alloc_chrdev_region = %d\n", alloc_ret);
        return -1;
    }

    /* 2. 取得したdev( = メジャー番号 + マイナー番号)からメジャー番号を取得して保持しておく */
    mydevice_major = MAJOR(dev);
    dev = MKDEV(mydevice_major, MINOR_BASE);    /* 不要? */

    /* 3. cdev構造体の初期化とシステムコールハンドラテーブルの登録 */
    cdev_init(&mydevice_cdev, &s_mydevice_fops);
    mydevice_cdev.owner = THIS_MODULE;

    /* 4. このデバイスドライバ(cdev)をカーネルに登録する */
    cdev_err = cdev_add(&mydevice_cdev, dev, MINOR_NUM);
    if (cdev_err != 0) {
        printk(KERN_ERR  "cdev_add = %d\n", cdev_err);
        unregister_chrdev_region(dev, MINOR_NUM);
        return -1;
    }

    return 0;
}

/* アンロード(rmmod)時に呼ばれる関数 */
static void mydevice_exit(void)
{
    printk("mydevice_exit\n");

    dev_t dev = MKDEV(mydevice_major, MINOR_BASE);

    /* 5. このデバイスドライバ(cdev)をカーネルから取り除く */
    cdev_del(&mydevice_cdev);

    /* 6. このデバイスドライバで使用していたメジャー番号の登録を取り除く */
    unregister_chrdev_region(dev, MINOR_NUM);
}

module_init(mydevice_init);
module_exit(mydevice_exit);