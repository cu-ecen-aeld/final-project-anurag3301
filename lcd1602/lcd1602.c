#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/init.h>

#define I2C_BUS_NUM 1  // i2c-1 on Raspberry Pi

static struct i2c_adapter *adapter = NULL;

static int __init i2c_scanner_init(void)
{
    int addr;
    struct i2c_client client;
    struct i2c_board_info board_info = {};
    int ret;

    pr_info("i2c_scanner: Initializing I2C scanner module\n");

    adapter = i2c_get_adapter(I2C_BUS_NUM);
    if (!adapter) {
        pr_err("i2c_scanner: Failed to get I2C adapter %d\n", I2C_BUS_NUM);
        return -ENODEV;
    }

    memset(&board_info, 0, sizeof(struct i2c_board_info));
    strncpy(board_info.type, "i2c_scanner_dummy", I2C_NAME_SIZE);

    for (addr = 0x03; addr <= 0x77; ++addr) {
        client.adapter = adapter;
        client.addr = addr;

        // Send a dummy read to test for device presence
        ret = i2c_smbus_read_byte(&client);
        if (ret >= 0) {
            pr_info("i2c_scanner: Found device at 0x%02X\n", addr);
        }
    }

    i2c_put_adapter(adapter);
    return 0;
}

static void __exit i2c_scanner_exit(void)
{
    pr_info("i2c_scanner: Module exit\n");
}

module_init(i2c_scanner_init);
module_exit(i2c_scanner_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("I2C Bus Scanner Kernel Module");
