#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shantanu Welling");
MODULE_DESCRIPTION("Memory Stats Kernel Module");

static int target_pid = 0; // Input parameter for target process ID
module_param(target_pid, int, 0644); // Permissions for input parameter
MODULE_PARM_DESC(target_pid, "Target process ID"); // Description of input parameter

static void print_memory_stats(struct task_struct *task) {
    struct mm_struct *mm; // Process memory map
    unsigned long vsize = 0; // Virtual address space size
    unsigned long psize = 0; // Physical address space size

    mm = get_task_mm(task); // Get process memory map
    if (!mm) { // If process does not have a memory map, print error
        printk(KERN_INFO "Process with PID %d does not have a memory map.\n", target_pid);
        return;
    }

    printk(KERN_INFO "Memory stats for process with PID %d:\n", target_pid);

    down_read(&mm->mmap_lock);             // Acquire mmap semaphore for reading memory map
    vsize = mm->total_vm << (PAGE_SHIFT-10); // Get virtual address space size, shift to get in KB
    psize = get_mm_rss(mm) << (PAGE_SHIFT-10);// Get physical address space size, shift to get in KB
    up_read(&mm->mmap_lock);               // Release mmap semaphore
    mmput(mm);                             // Release memory map
    printk(KERN_INFO "Allocated Virtual Address Space: %lu KB\n", vsize);
    printk(KERN_INFO "Mapped Physical Address Space: %lu KB\n", psize);
}

static int __init memory_stats_module_init(void) { // Module init function
    struct task_struct *task; // Task struct for target process

    rcu_read_lock(); // Acquire read lock
    task = pid_task(find_vpid(target_pid), PIDTYPE_PID); // Find task struct for target process
    if (task != NULL) { // If task struct is found, print memory stats
        print_memory_stats(task);
    } else {
        printk(KERN_INFO "Process with PID %d not found.\n", target_pid);
    }
    rcu_read_unlock(); // Release read lock

    return 0;
}

static void __exit memory_stats_module_exit(void) { // Module exit function
    printk(KERN_INFO "Memory Stats Module Unloaded\n");
}

module_init(memory_stats_module_init);
module_exit(memory_stats_module_exit);
