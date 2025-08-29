#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/delay.h>

#define I2C_BUS 1
#define LCD_NAME "i2c_lcd"
#define LCD_ADDR 0x27  // Change if your LCD uses different address

// PCF8574 pin mapping
#define RS_BIT 0
#define EN_BIT 2
#define BL_BIT 3
#define D4_BIT 4

static struct i2c_adapter *lcd_i2c_adapter;
static struct i2c_client  *lcd_i2c_client;

// Backlight state
static int lcd_backlight = 1;

// Low-level I2C write
static int lcd_i2c_write(uint8_t *buf, int len)
{
    return i2c_master_send(lcd_i2c_client, buf, len);
}

// Pulse enable
static void lcd_pulse(uint8_t data)
{
    lcd_i2c_write((uint8_t[]){ data | (1<<EN_BIT) }, 1);
    udelay(500);
    lcd_i2c_write((uint8_t[]){ data & ~(1<<EN_BIT) }, 1);
    udelay(100);
}

// Send 4-bit nibble
static void lcd_send_nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t data = (nibble << D4_BIT) | (rs << RS_BIT) | (lcd_backlight << BL_BIT);
    lcd_pulse(data);
}

// Send full byte
static void lcd_send_byte(uint8_t val, uint8_t rs)
{
    lcd_send_nibble(val >> 4, rs);
    lcd_send_nibble(val & 0x0F, rs);
}

// Commands/data
static void lcd_cmd(uint8_t cmd) { lcd_send_byte(cmd, 0); }
static void lcd_data(uint8_t data) { lcd_send_byte(data, 1); }

// Clear display
static void lcd_clear(void)
{
    lcd_cmd(0x01);
    msleep(2);
}

// Set cursor
static void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t addr = col + (row ? 0x40 : 0x00);
    lcd_cmd(0x80 | addr);
}

// Write string
static void lcd_write_string(const char *str)
{
    while (*str)
        lcd_data(*str++);
}

// Initialize LCD
static void lcd_init_hw(void)
{
    msleep(50);
    lcd_send_nibble(0x03, 0);
    msleep(5);
    lcd_send_nibble(0x03, 0);
    udelay(150);
    lcd_send_nibble(0x03, 0);
    udelay(150);
    lcd_send_nibble(0x02, 0);
    udelay(150);

    lcd_cmd(0x28); // 4-bit, 2 lines
    lcd_cmd(0x0C); // Display ON
    lcd_clear();
    lcd_cmd(0x06); // Entry mode
}

// Probe
static int lcd_probe(struct i2c_client *client)
{
    lcd_i2c_client = client;
    lcd_init_hw();
    lcd_set_cursor(0, 0);
    lcd_write_string("Hello, world");
    pr_info("i2c_lcd: LCD initialized\n");
    return 0;
}

// Remove
static void lcd_remove(struct i2c_client *client)
{
    lcd_clear();
    pr_info("i2c_lcd: Driver removed\n");
}

static const struct i2c_device_id lcd_id[] = {
    { LCD_NAME, 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, lcd_id);

// I2C driver structure
static struct i2c_driver lcd_driver = {
    .driver = {
        .name = LCD_NAME,
        .owner = THIS_MODULE,
    },
    .probe = lcd_probe,
    .remove = lcd_remove,
    .id_table = lcd_id,
};

// Module init
static int __init lcd_module_init(void)
{
    int ret;

    ret = i2c_add_driver(&lcd_driver);  // register driver first
    if (ret)
        return ret;

    lcd_i2c_adapter = i2c_get_adapter(I2C_BUS);
    if (!lcd_i2c_adapter)
        return -ENODEV;

    struct i2c_board_info board_info = {
        I2C_BOARD_INFO(LCD_NAME, LCD_ADDR)
    };

    lcd_i2c_client = i2c_new_client_device(lcd_i2c_adapter, &board_info); // create client after driver
    i2c_put_adapter(lcd_i2c_adapter);

    return 0;
}


// Module exit
static void __exit lcd_module_exit(void)
{
    i2c_unregister_device(lcd_i2c_client);
    i2c_del_driver(&lcd_driver);
    pr_info("i2c_lcd: Module unloaded\n");
}

module_init(lcd_module_init);
module_exit(lcd_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Simple I2C LCD1602 Driver showing Hello World");
