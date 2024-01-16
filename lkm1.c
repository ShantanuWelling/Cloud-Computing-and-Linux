#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shantanu Welling");
MODULE_DESCRIPTION("List Processes Kernel Module");

static int __init list_processes_init(void) { // Module entry point
    struct task_struct *task; // Create a task_struct to iterate over all processes

    printk(KERN_INFO "List of Processes in Running or Runnable State:\n");
    for_each_process(task) { // Iterate over all processes
        if (task->__state == TASK_RUNNING) { // Check if process is running or runnable
            printk(KERN_INFO "PID: %d, Name: %s, State: %ld\n", task->pid, task->comm, task->__state);
        }
    }

    return 0;
}

static void __exit list_processes_exit(void) { // Module exit point
    printk(KERN_INFO "List Processes Module Unloaded\n");
}

module_init(list_processes_init);
module_exit(list_processes_exit);