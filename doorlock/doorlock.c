#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define GPIO_PIN 533       // GPIO 21
#define DEVICE_NAME "doorlock"

static int major_num;
static struct class *gpio_class = NULL;
static struct device *gpio_device = NULL;

// Write operation: 0 -> LOW, 1 -> HIGH
static ssize_t gpio_write(struct file *file, const char __user *buf,
                          size_t len, loff_t *off)
{
    char kbuf[2];

    if (len > 2)
        return -EINVAL;

    if (copy_from_user(kbuf, buf, len))
        return -EFAULT;

    if (kbuf[0] == '1')
        gpio_set_value(GPIO_PIN, 1);
    else if (kbuf[0] == '0')
        gpio_set_value(GPIO_PIN, 0);
    else
        return -EINVAL;

    return len;
}

// Read operation: returns "0\n" or "1\n"
static ssize_t gpio_read(struct file *file, char __user *buf,
                         size_t len, loff_t *off)
{
    char kbuf[2];
    int value;

    if (*off > 0)
        return 0;

    value = gpio_get_value(GPIO_PIN);
    kbuf[0] = value ? '1' : '0';
    kbuf[1] = '\n';

    if (copy_to_user(buf, kbuf, 2))
        return -EFAULT;

    *off += 2;
    return 2;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = gpio_write,
    .read  = gpio_read,
};

static int __init gpio_driver_init(void)
{
    int ret;

    pr_info("Doorlock: GPIO Driver Init\n");

    ret = gpio_request(GPIO_PIN, "rpi_gpio");
    if (ret) {
        pr_err("Doorlock: Failed to request GPIO %d\n", GPIO_PIN);
        return ret;
    }

    gpio_direction_output(GPIO_PIN, 0);     // Set gpio as output
    gpio_set_value(GPIO_PIN, 0);

    major_num = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_num < 0) {
        pr_err("Doorlock: Failed to register char device\n");
        gpio_free(GPIO_PIN);
        return major_num;
    }

    gpio_class = class_create("gpio_class");
    if (IS_ERR(gpio_class)) {
        unregister_chrdev(major_num, DEVICE_NAME);
        gpio_free(GPIO_PIN);
        pr_err("Doorlock: Failed to create class\n");
        return PTR_ERR(gpio_class);
    }

    // Create device node /dev/gpio_led
    gpio_device = device_create(gpio_class, NULL, MKDEV(major_num, 0),
                                NULL, DEVICE_NAME);
    if (IS_ERR(gpio_device)) {
        class_destroy(gpio_class);
        unregister_chrdev(major_num, DEVICE_NAME);
        gpio_free(GPIO_PIN);
        pr_err("Doorlock: Failed to create device\n");
        return PTR_ERR(gpio_device);
    }

    pr_info("Doorlock: GPIO %d ready, char device /dev/%s with major %d\n",
            GPIO_PIN, DEVICE_NAME, major_num);

    return 0;
}

static void __exit gpio_driver_exit(void)
{
    pr_info("Doorlock: GPIO Driver Exit\n");

    gpio_set_value(GPIO_PIN, 0);
    gpio_free(GPIO_PIN);

    device_destroy(gpio_class, MKDEV(major_num, 0));
    class_destroy(gpio_class);
    unregister_chrdev(major_num, DEVICE_NAME);

    pr_info("Doorlock: GPIO %d released, char device unregistered\n", GPIO_PIN);
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("anurag3301");
MODULE_DESCRIPTION("Door lock Toggle Kernel Module via Char Device with read/write");
