#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <asm/current.h>
#include <linux/sched.h>

#define FIRST_MINOR 0 // Minor number of device
#define MINOR_CNT 1   // Number of minor numbers
#define IOCTL_SET_PARENT_PID _IOW('q', 1, pid_t) // Macro to get parent PID

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shantanu Welling");
MODULE_DESCRIPTION("Custom IOCTL Device Driver");

static dev_t dev; // Variable to hold device number
static struct cdev c_dev; // Structure to hold character device
struct class *cl; // Structure to hold device class

static int my_open(struct inode *i, struct file *f)
{ // Custom open function for device file operations
    return 0;
}
static int my_close(struct inode *i, struct file *f)
{ // Custom close function for device file operations
    return 0;
}

static long custom_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    pid_t new_parent_pid; // Variable to store new parent PID
    struct task_struct *new_parent_task, *current_task; // Variables to store new parent and current task_structs

    switch (cmd) {
        case IOCTL_SET_PARENT_PID:
            // Get the new parent PID from user space
            if (copy_from_user(&new_parent_pid, (pid_t __user *)arg, sizeof(pid_t)))
                return -EFAULT;

            // Find the task_struct of the new parent process
            new_parent_task = pid_task(find_vpid(new_parent_pid), PIDTYPE_PID);
            if (!new_parent_task)
                return -ESRCH; // No such process

            // Get the current task_struct
            current_task = get_current();

            // Change the parent process to the new parent
            rcu_read_lock(); // Acquire reader lock
            list_del(&current_task->sibling); // Remove current process from old parent's children list
            task_lock(current_task); // Lock current task_struct for write
            current_task->parent = new_parent_task; // Change parent
            current_task->real_parent = new_parent_task;
            task_unlock(current_task); // Unlock current task_struct
            // Add current process to new parent's children list
            list_add_tail(&current_task->sibling, &new_parent_task->children);

            rcu_read_unlock(); // Release reader lock
            
            printk(KERN_INFO "Parent process changed to PID %d\n", new_parent_pid);
            break;

        default:
            return -ENOTTY; // Not a valid ioctl command
    }

    return 0;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .unlocked_ioctl = custom_ioctl, // IOCTL function
};

static int __init custom_init(void) { // Module initialization function
    int ret; // Variable to store return values
    struct device *dev_ret; // Variable to store device return values
    // Allocate character device region
    if ((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "ioctl_pid")) < 0){
        return ret;
    }
    cdev_init(&c_dev, &fops); // Initialize character device
    if ((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0) // Add character device to system
    {
        return ret;
    }
    if (IS_ERR(cl = class_create(THIS_MODULE, "char"))) // Create device class
    { // If error, delete character device and unregister character device region
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(cl);
    }
    if (IS_ERR(dev_ret = device_create( cl, NULL, dev, NULL, "ioctl_pid_dev"))) // Create device
    { // If error, delete device class, character device and unregister character device region
        cdev_del(&c_dev);
        class_destroy(cl);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(dev_ret);
    }
    // Finished loading module successfully
    printk(KERN_INFO "ioctl_pid loaded\n");
    return 0;
}

static void __exit custom_exit(void) { // Module exit function
    // Delete device, device class, character device and unregister character device region
    device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, MINOR_CNT);
    printk(KERN_INFO "ioctl_pid unloaded\n");
}

module_init(custom_init); 
module_exit(custom_exit);
