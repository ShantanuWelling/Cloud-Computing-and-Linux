#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <asm/current.h>
#include <linux/sched.h>

#define FIRST_MINOR 0 // Minor number of device
#define MINOR_CNT 1   // Number of minor numbers

#define IOCTL_GET_PHYSICAL_ADDRESS _IOR('q', 1, unsigned long) // Macro to get physical address
#define IOCTL_WRITE_TO_PHYSICAL _IOW('q', 2, struct ioctl_data) // Macro to write to physical address

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shantanu Welling");

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

struct ioctl_data { // Structure to store data for ioctl calls
    unsigned long virtual_address; // Virtual address of the memory
    unsigned long physical_address; // Physical address of the memory
    char value; // Value to be written to the memory
};

static unsigned long get_physical_address(unsigned long vaddr) {
    // Get physical address of target virtual address
    pgd_t *pgd; // Variables to store PGD, P4D, PUD, PMD and PTE entry of the multi-level page table
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    phys_addr_t paddr; // Variable to store physical address
    struct  task_struct *task = current; // Get current task_struct
    pgd = pgd_offset(task->mm, vaddr); // Get PGD entry for the virtual address
    if (pgd_none(*pgd) || pgd_bad(*pgd)) { // Check if PGD entry is valid
        printk(KERN_INFO "Invalid PGD entry\n");
        return -1;
    }

    p4d = p4d_offset(pgd, vaddr); // Get P4D entry for the virtual address
    if (p4d_none(*p4d) || p4d_bad(*p4d)) { // Check if P4D entry is valid
        printk(KERN_INFO "Invalid P4D entry\n");
        return -1;
    }

    pud = pud_offset(p4d, vaddr); // Get PUD entry for the virtual address
    if (pud_none(*pud) || pud_bad(*pud)) { // Check if PUD entry is valid
        printk(KERN_INFO "Invalid PUD entry\n");
        return -1;
    }

    pmd = pmd_offset(pud, vaddr); // Get PMD entry for the virtual address
    if (pmd_none(*pmd) || pmd_bad(*pmd)) { // Check if PMD entry is valid
        printk(KERN_INFO "Invalid PMD entry\n");
        return -1;
    }

    pte = pte_offset_kernel(pmd, vaddr); // Get PTE entry for the virtual address
    if(pte_none(*pte) || !pte_present(*pte)) { // Check if PTE entry is valid and present
        printk(KERN_INFO "Page not present or pte not found\n");
        return -1;
    }
    paddr = (phys_addr_t)(pte_val(*pte) & PAGE_MASK) + (vaddr & ~PAGE_MASK); // Calculate physical address

    printk(KERN_INFO "PID: %d, Virtual Address: 0x%lx, Physical Address: 0x%llx\n",
           task->pid, vaddr, (unsigned long long)paddr);
    return (unsigned long) paddr-0x8000000000000000;
}

// device driver ioctl function
static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) {
    struct ioctl_data user_data; // Structure to store data for ioctl calls
    struct ioctl_data *input = (struct ioctl_data *) ioctl_param; // Pointer to input data for ioctl calls
    switch (ioctl_num) {
        case IOCTL_GET_PHYSICAL_ADDRESS: // Case: Get physical address of the virtual address 
            user_data.virtual_address = input->virtual_address;
            user_data.value = input->value;
            user_data.physical_address = get_physical_address(user_data.virtual_address);
            // Return physical address to user space via user_data struct
            if (copy_to_user((void *) ioctl_param, &user_data, sizeof(struct ioctl_data)))
                return -EFAULT;
            break;
        
        case IOCTL_WRITE_TO_PHYSICAL: // Case: Write to physical address
        // Get user_data struct from user space
            if (copy_from_user(&user_data, (void *) ioctl_param, sizeof(struct ioctl_data)))
                return -EFAULT;
            printk(KERN_INFO "Writing %d to physical address 0x%lx\n", user_data.value, user_data.physical_address);
            memcpy((void *) __va(user_data.physical_address), &user_data.value, sizeof(char)); 
            // Write to physical address the value stored in user_data
            break;

        default:
            return -ENOTTY; // Not a valid ioctl command
    }

    return 0;
}

// Custom file operations structure for device driver
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .unlocked_ioctl = device_ioctl, // ioctl function
};

static int __init ioctl_example_init(void) { // Module initialization function
    int ret; // Variable to store return values
    struct device *dev_ret; // Variable to store device return values
    // Allocate character device region
    if ((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "ioctl_mod")) < 0){
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
    if (IS_ERR(dev_ret = device_create( cl, NULL, dev, NULL, "ioctl_mod_dev"))) // Create device
    { // If error, delete device class, character device and unregister character device region
        cdev_del(&c_dev);
        class_destroy(cl);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(dev_ret);
    }
    // Finished loading module successfully
    printk(KERN_INFO "ioctl_mod loaded\n");
    return 0;
}

static void __exit ioctl_example_exit(void) { // Module exit function
    // Delete device, device class, character device and unregister character device region
    device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, MINOR_CNT);
    printk(KERN_INFO "ioctl_mod unloaded\n");
}

module_init(ioctl_example_init);
module_exit(ioctl_example_exit);