#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>

#define PROCFS_NAME "get_pgfaults"

// Module information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shantanu Welling");
MODULE_DESCRIPTION("Page Faults /proc Filesystem Module");


// Function to display the total page faults count when reading from /proc/get_pgfaults
static int get_pgfaults_show(struct seq_file *m, void *v) {
    unsigned long * faults = kmalloc(sizeof(unsigned long) * NR_VM_EVENT_ITEMS, GFP_KERNEL); // Allocate memory for the array of VM events' counts
    all_vm_events(faults); // Get all VM events' counts
    seq_printf(m, "Page Faults - %lu\n", faults[PGFAULT]); // Display the total page faults count
    return 0;
}

// Open function for the /proc/get_pgfaults file
static int get_pgfaults_open(struct inode *inode, struct file *file) {
    return single_open(file, get_pgfaults_show, NULL); // Open the file with the custom show function
}

// File operations structure
static const struct proc_ops get_pgfaults_fops = {
    .proc_open = get_pgfaults_open, // Custom open function
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

// Module initialization function
static int __init get_pgfaults_init(void) {
    proc_create(PROCFS_NAME, 0, NULL, &get_pgfaults_fops); // Create the file system entry
    printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
    return 0;
}

// Module cleanup function
static void __exit get_pgfaults_exit(void) {
    remove_proc_entry(PROCFS_NAME, NULL); // Remove the file system entry
    printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}

// Module registration
module_init(get_pgfaults_init);
module_exit(get_pgfaults_exit);