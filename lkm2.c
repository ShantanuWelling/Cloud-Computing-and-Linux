#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shantanu Welling");
MODULE_DESCRIPTION("Process Children Kernel Module");

static int target_pid = 0; // Input parameter for target process ID

module_param(target_pid, int, 0644); // Set permissions for target_pid
MODULE_PARM_DESC(target_pid, "Target process ID"); // Include in module parameters and state description

static void print_child_processes(struct task_struct *task) {
    struct task_struct *child; // Create a task_struct to store child process

    list_for_each_entry(child, &task->children, sibling) { // Iterate over children of target process
        printk(KERN_INFO "Child PID: %d, State: %ld\n", child->pid, child->__state); // Print child process pid and state
    }
}

static int __init process_tree_init(void) { // Module entry point
    struct task_struct *task; // Create a task_struct to store target process

    printk(KERN_INFO "Children for Parent PID %d\n", target_pid);

    rcu_read_lock(); // Lock the RCU read lock
    // Used by a reader to inform the reclaimer that the reader is entering an RCU read-side critical section

    task = pid_task(find_vpid(target_pid), PIDTYPE_PID); // Find task_struct for target process
    // find_vpid() returns a pid_t with virtual pid as input arg, which is a struct pid. 
    // This is then passed to pid_task(), which returns a task_struct.

    if (task != NULL) { // Check if process exists
        printk(KERN_INFO "Parent PID: %d, State: %ld\n", task->pid, task->__state);
        print_child_processes(task); // Print children of target process
    } else {
        printk(KERN_INFO "Process with PID %d not found.\n", target_pid);
    }

    rcu_read_unlock(); // Unlock the RCU read lock

    return 0;
}

static void __exit process_tree_exit(void) { // Module exit point
    printk(KERN_INFO "Process Children Module Unloaded\n");
}

module_init(process_tree_init);
module_exit(process_tree_exit);
