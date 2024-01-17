#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <asm/current.h>

#define IOCTL_GET_PHYSICAL_ADDRESS _IOR('q', 1, unsigned long)
#define IOCTL_WRITE_TO_PHYSICAL _IOW('q', 2, struct ioctl_data)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shantanu Welling");

struct ioctl_data {
    unsigned long physical_address;
    char value;
};

static char *kernel_memory;

static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) {
    struct ioctl_data user_data;

    switch (ioctl_num) {
        case IOCTL_GET_PHYSICAL_ADDRESS:
            user_data.physical_address = virt_to_phys((void *) ioctl_param);
            if (copy_to_user((void *) ioctl_param, &user_data, sizeof(struct ioctl_data)))
                return -EFAULT;
            break;
        
        case IOCTL_WRITE_TO_PHYSICAL:
            if (copy_from_user(&user_data, (void *) ioctl_param, sizeof(struct ioctl_data)))
                return -EFAULT;
            memcpy((void *) phys_to_virt(user_data.physical_address), &user_data.value, sizeof(char));
            break;

        default:
            return -ENOTTY; // Not a valid ioctl command
    }

    return 0;
}

static struct file_operations fops = {
    .unlocked_ioctl = device_ioctl,
};

static int __init ioctl_example_init(void) {
    // Allocate kernel memory
    kernel_memory = kmalloc(sizeof(char), GFP_KERNEL);
    if (!kernel_memory) {
        printk(KERN_ERR "Failed to allocate kernel memory\n");
        return -ENOMEM;
    }

    // Register character device
    register_chrdev(240, "ioctl_dd", &fops);
    printk(KERN_INFO "ioctl_dd loaded\n");
    return 0;
}

static void __exit ioctl_example_exit(void) {
    // Free allocated memory and unregister character device
    kfree(kernel_memory);
    unregister_chrdev(240, "ioctl_dd");
    printk(KERN_INFO "ioctl_dd unloaded\n");
}

module_init(ioctl_example_init);
module_exit(ioctl_example_exit);