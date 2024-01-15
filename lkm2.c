#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shantanu Welling");
MODULE_DESCRIPTION("Process Children Kernel Module");

static int target_pid = 0; 

module_param(target_pid, int, 0644);
MODULE_PARM_DESC(target_pid, "Target process ID");

static void print_child_processes(struct task_struct *task) {
    struct task_struct *child;

    list_for_each_entry(child, &task->children, sibling) {
        printk(KERN_INFO "Child PID: %d, State: %ld\n", child->pid, child->__state); 
    }
}

static int __init process_tree_init(void) {
    struct task_struct *task;

    printk(KERN_INFO "Children for Parent PID %d\n", target_pid);

    rcu_read_lock();

    task = pid_task(find_vpid(target_pid), PIDTYPE_PID);
    if (task != NULL) {
        printk(KERN_INFO "Parent PID: %d, State: %ld\n", task->pid, task->__state);
        print_child_processes(task);
    } else {
        printk(KERN_INFO "Process with PID %d not found.\n", target_pid);
    }

    rcu_read_unlock();

    return 0;
}

static void __exit process_tree_exit(void) {
    printk(KERN_INFO "Process Children Module Unloaded\n");
}

module_init(process_tree_init);
module_exit(process_tree_exit);
