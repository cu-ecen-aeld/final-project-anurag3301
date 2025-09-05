#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

#define DIM 4

static int cols[] = {522, 534, 539, 529};
static int rows[] = {521, 523, 517, 518};
static int row_irqs[DIM];

struct debounce_data{
    int gpio;
    int irq;
    struct delayed_work work;
};

static struct debounce_data debounce_rows[DIM];

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

        pr_info("Debounced IRQ on row GPIO %d — starting matrix scan\n", data->gpio);

        for (int col_idx = 0; col_idx < DIM; col_idx++) {
            for (int c = 0; c < DIM; c++) {
                gpio_set_value(cols[c], c == col_idx ? 0 : 1);
            }

            udelay(5);  // allow settling

            for (int row_idx = 0; row_idx < DIM; row_idx++) {
                int row_val = gpio_get_value(rows[row_idx]);
                if (row_val == 0) {
                    pr_info("Key pressed at row %d, col %d (GPIO Row %d, Col %d)\n",
                            row_idx, col_idx, rows[row_idx], cols[col_idx]);
                }
            }
        }

        for (int c = 0; c < DIM; c++) {
            gpio_set_value(cols[c], 0);
        }
    }

    enable_irq(data->irq);
}


static int __init keypad_init(void){
    int ret;
    
    pr_info("Setting up keypad\n");

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
    for(int i=0; i<DIM; i++){
        gpio_set_value(cols[i], 0);
        gpio_free(cols[i]);

        cancel_delayed_work_sync(&debounce_rows[i].work);
        free_irq(debounce_rows[i].irq, &debounce_rows[i]);
        gpio_free(debounce_rows[i].gpio);
    }
}


module_init(keypad_init);
module_exit(keypad_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("anurag3301");
MODULE_DESCRIPTION("Key keypad keypress using char device");
