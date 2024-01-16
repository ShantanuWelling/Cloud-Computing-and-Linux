#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROCFS_NAME "hello_procfs"

// Module information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shantanu Welling");
MODULE_DESCRIPTION("Hello World /proc Filesystem Module");

// Function to display the "Hello World!" message when reading from /proc/hello_procfs
static int hello_procfs_show(struct seq_file *m, void *v) {
    seq_printf(m, "Hello World!\n"); // Print the message to the buffer
    return 0;
}

// Open function for the /proc/hello_procfs file which calls single open for simple virtual file
static int hello_procfs_open(struct inode *inode, struct file *file) {
    return single_open(file, hello_procfs_show, NULL); 
}

// File operations structure
static const struct proc_ops hello_procfs_fops = {
    .proc_open = hello_procfs_open, // Custom open function
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

// Module initialization function
static int __init hello_procfs_init(void) {
    proc_create(PROCFS_NAME, 0, NULL, &hello_procfs_fops); // Create the /proc/hello_procfs file system entry
    printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
    return 0;
}

// Module cleanup function
static void __exit hello_procfs_exit(void) {
    remove_proc_entry(PROCFS_NAME, NULL); // Remove the /proc/hello_procfs file system entry
    printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}

// Module registration
module_init(hello_procfs_init);
module_exit(hello_procfs_exit);