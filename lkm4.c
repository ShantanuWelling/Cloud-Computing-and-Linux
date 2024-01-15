#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shantanu Welling");
MODULE_DESCRIPTION("Memory Stats Kernel Module");

static int target_pid = 0;
module_param(target_pid, int, 0644);
MODULE_PARM_DESC(target_pid, "Target process ID");

static void print_memory_stats(struct task_struct *task) {
    struct mm_struct *mm;
    struct vm_area_struct *vma;
    unsigned long vsize = 0;
    unsigned long psize = 0;

    mm = task->mm;
    if (!mm) {
        printk(KERN_INFO "Process with PID %d does not have a memory map.\n", target_pid);
        return;
    }

    printk(KERN_INFO "Memory stats for process with PID %d:\n", target_pid);

    down_read(&mm->mmap_sem);
    for (vma = mm->mmap; vma; vma = vma->vm_next) {
        vsize += vma->vm_end - vma->vm_start;
        psize += vma_pagesize(vma) * (vma->vm_end - vma->vm_start) / PAGE_SIZE;
    }
    up_read(&mm->mmap_sem);

    printk(KERN_INFO "Allocated Virtual Address Space: %lu KB\n", vsize >> 10);
    printk(KERN_INFO "Mapped Physical Address Space: %lu KB\n", psize >> 10);
}

static int __init memory_stats_module_init(void) {
    struct task_struct *task;

    rcu_read_lock();
    task = pid_task(find_vpid(target_pid), PIDTYPE_PID);
    if (task != NULL) {
        printk(KERN_INFO "Collecting memory stats for process with PID %d\n", target_pid);
        print_memory_stats(task);
    } else {
        printk(KERN_INFO "Process with PID %d not found.\n", target_pid);
    }
    rcu_read_unlock();

    return 0;
}

static void __exit memory_stats_module_exit(void) {
    printk(KERN_INFO "Memory Stats Module Unloaded\n");
}

module_init(memory_stats_module_init);
module_exit(memory_stats_module_exit);
