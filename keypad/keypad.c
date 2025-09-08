#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/ioctl.h>

#define DIM 4
#define BUF_SIZE 10

#define DEVICE_NAME "keypad"
static DECLARE_WAIT_QUEUE_HEAD(keypad_wq);


#define KEYPAD_MAGIC 'K'
#define KEYPAD_FLUSH_BUFFER _IO(KEYPAD_MAGIC, 0)

static int major_num;
static struct class *keypad_class = NULL;
static struct device *keypad_device = NULL;

static int cols[] = {522, 534, 539, 529};
static int rows[] = {521, 523, 517, 518};
static int row_irqs[DIM];

static char button_map[DIM][DIM] = {{'1', '2', '3', 'A'},
                                    {'4', '5', '6', 'B'},
                                    {'7', '8', '9', 'C'},
                                    {'*', '0', '#', 'D'}};

struct circ_buf{
    uint head;
    uint tail;
    char* buf;
};

struct debounce_data{
    int gpio;
    int irq;
    struct delayed_work work;
};

static struct debounce_data debounce_rows[DIM];
static struct circ_buf key_buffer;

static void dump_buf(void){
    char buf[BUF_SIZE+1] = {0};
    for(int i=key_buffer.head+1, j=0; i%BUF_SIZE!=key_buffer.tail; i=(i+1)%BUF_SIZE, j++){
        buf[j] = key_buffer.buf[i];
    }
    pr_info("Keypad: Buffer: %s\n", buf);
}

static void push_buf(char c){
    key_buffer.buf[key_buffer.tail] = c;
    if(key_buffer.tail == key_buffer.head){
        key_buffer.head = (key_buffer.head+1) % BUF_SIZE;
    }
    key_buffer.tail = (key_buffer.tail+1) % BUF_SIZE;
    wake_up_interruptible(&keypad_wq);
}

char pop_buf(void){
    uint next = (key_buffer.head+1) % BUF_SIZE;
    if(next == key_buffer.tail){
        return '\0';
    }
    key_buffer.head = next; 
    return key_buffer.buf[next];
}

static int is_buffer_empty(void)
{
    uint next = (key_buffer.head+1) % BUF_SIZE;
    return (next == key_buffer.tail);
}

static ssize_t keypad_read(struct file *file, char __user *buf,
                           size_t len, loff_t *off)
{
    char kbuf[1];
    int ret;

    ret = wait_event_interruptible(keypad_wq, !is_buffer_empty());
    if (ret)
        return ret;
    kbuf[0] = pop_buf();
    
    if (kbuf[0] == '\0') {
        return -EAGAIN;
    }
    
    if (copy_to_user(buf, kbuf, 1))
        return -EFAULT;
    
    *off += 1;
    return 1;
}


static irqreturn_t row_irq_handler(int irq, void *dev_id){
    struct debounce_data *data = dev_id;

    disable_irq_nosync(data->irq);

    schedule_delayed_work(&data->work, msecs_to_jiffies(100));  // 100ms debounce
    return IRQ_HANDLED;
}

static void debounce_work_func(struct work_struct *work)
{
    struct debounce_data *data = container_of(work, struct debounce_data, work.work);

    if (gpio_get_value(data->gpio) == 0) {

        for (int col_idx = 0; col_idx < DIM; col_idx++) {
            for (int c = 0; c < DIM; c++) {
                gpio_set_value(cols[c], c == col_idx ? 0 : 1);
            }

            udelay(5);  // allow settling

            for (int row_idx = 0; row_idx < DIM; row_idx++) {
                int row_val = gpio_get_value(rows[row_idx]);
                if (row_val == 0) {
                    pr_info("Keypad: Key pressed at row %d, col %d (GPIO Row %d, Col %d)\n",
                            row_idx, col_idx, rows[row_idx], cols[col_idx]);
                    push_buf(button_map[row_idx][col_idx]);
                    dump_buf();
                }
            }
        }

        for (int c = 0; c < DIM; c++) {
            gpio_set_value(cols[c], 0);
        }
    }

    enable_irq(data->irq);
}

static long keypad_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    switch(cmd){
        case KEYPAD_FLUSH_BUFFER:
            key_buffer.head = 0;
            key_buffer.tail = 1;
            pr_info("Keypad: Buffer flushed!\n");
            break;
        default:
            return -EINVAL;
    }

    return 0;
}


static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read  = keypad_read,
    .unlocked_ioctl = keypad_ioctl,
};

static int __init keypad_init(void){
    int ret;
    
    pr_info("Setting up keypad\n");
    
    key_buffer.buf = kmalloc(BUF_SIZE, GFP_KERNEL);
    key_buffer.head = 0;
    key_buffer.tail = 1;


    major_num = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_num < 0) {
        pr_err("Failed to register char device\n");
        return major_num;
    }

    keypad_class = class_create("keypad_class");
    if (IS_ERR(keypad_class)) {
        pr_err("Failed to create class\n");
        return PTR_ERR(keypad_class);
    }

    keypad_device = device_create(keypad_class, NULL, MKDEV(major_num, 0),
                                NULL, DEVICE_NAME);
    if (IS_ERR(keypad_class)) {
        pr_err("Failed to create device\n");
        return PTR_ERR(keypad_device);
    }


    for(int i=0; i<DIM; i++){
        ret = gpio_request(cols[i], "rpi_gpio");
        if(ret){
            pr_err("Failed to request GPIO %d\n", cols[i]);
            return ret;
        }
        gpio_direction_output(cols[i], 0);
        gpio_set_value(cols[i], 0);

        ret = gpio_request(rows[i], "rpi_gpio");
        if(ret){
            pr_err("Failed to request GPIO %d\n", rows[i]);
            return ret;
        }
        gpio_direction_input(rows[i]);
        
        debounce_rows[i].gpio = rows[i];
        debounce_rows[i].irq = gpio_to_irq(rows[i]);
        INIT_DELAYED_WORK(&debounce_rows[i].work, debounce_work_func);

        ret = request_irq(debounce_rows[i].irq, row_irq_handler, IRQF_TRIGGER_FALLING,
                          "rpi_gpio", &debounce_rows[i]);
        if (ret) {
            pr_err("Failed to request IRQ %d for GPIO %d\n", row_irqs[i], rows[i]);
            return ret;
        }

        pr_info("Registered IRQ %d for GPIO %d\n", row_irqs[i], rows[i]);
    }

    return 0;
}

static void __exit keypad_exit(void){
    pr_info("Keypad exit\n");
    kfree(key_buffer.buf);
    for(int i=0; i<DIM; i++){
        gpio_set_value(cols[i], 0);
        gpio_free(cols[i]);

        cancel_delayed_work_sync(&debounce_rows[i].work);
        free_irq(debounce_rows[i].irq, &debounce_rows[i]);
        gpio_free(debounce_rows[i].gpio);
    }

    device_destroy(keypad_class, MKDEV(major_num, 0));
    class_destroy(keypad_class);
    unregister_chrdev(major_num, DEVICE_NAME);

}


module_init(keypad_init);
module_exit(keypad_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("anurag3301");
MODULE_DESCRIPTION("Key keypad keypress using char device");
