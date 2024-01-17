#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/pgtable.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shantanu Welling");
MODULE_DESCRIPTION("TRANSLATION Module");

static int target_pid = 0; // Input parameter for target process ID
module_param(target_pid, int, 0644); // Set permissions for target_pid
MODULE_PARM_DESC(target_pid, "Target process ID"); // Include in module parameters & state description

static unsigned long target_vaddr = 0; // Input parameter for target virtual address
module_param(target_vaddr, ulong, 0644); // Set permissions for target_vaddr
MODULE_PARM_DESC(target_vaddr, "Target virtual address"); // Include in module parameters & state description

static void print_physical_address(struct task_struct *task, unsigned long vaddr) {
    // Print physical address of target virtual address
    pgd_t *pgd; // Variables to store PGD, P4D, PUD, PMD and PTE entry of the multi-level page table
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    phys_addr_t paddr; // Variable to store physical address

    pgd = pgd_offset(task->mm, vaddr); // Get PGD entry for the virtual address
    if (pgd_none(*pgd) || pgd_bad(*pgd)) { // Check if PGD entry is valid
        printk(KERN_INFO "Invalid PGD entry\n");
        return;
    }

    p4d = p4d_offset(pgd, vaddr); // Get P4D entry for the virtual address
    if (p4d_none(*p4d) || p4d_bad(*p4d)) { // Check if P4D entry is valid
        printk(KERN_INFO "Invalid P4D entry\n");
        return;
    }

    pud = pud_offset(p4d, vaddr); // Get PUD entry for the virtual address
    if (pud_none(*pud) || pud_bad(*pud)) { // Check if PUD entry is valid
        printk(KERN_INFO "Invalid PUD entry\n");
        return;
    }

    pmd = pmd_offset(pud, vaddr); // Get PMD entry for the virtual address
    if (pmd_none(*pmd) || pmd_bad(*pmd)) { // Check if PMD entry is valid
        printk(KERN_INFO "Invalid PMD entry\n");
        return;
    }

    pte = pte_offset_kernel(pmd, vaddr); // Get PTE entry for the virtual address
    if(pte_none(*pte) || !pte_present(*pte)) { // Check if PTE entry is valid and present
        printk(KERN_INFO "Page not present or pte not found\n");
        return;
    }
    paddr = (phys_addr_t)(pte_val(*pte) & PAGE_MASK) + (vaddr & ~PAGE_MASK); // Calculate physical address

    printk(KERN_INFO "PID: %d, Virtual Address: 0x%lx, Physical Address: 0x%llx\n",
           task->pid, vaddr, (unsigned long long)paddr);
}

static int __init mapping_module_init(void) { // Module entry point
    struct task_struct *task; // Create a task_struct to store target process

    rcu_read_lock(); // Lock the RCU read lock
    // Used by a reader to inform the reclaimer that the reader is entering an RCU read-side critical section

    task = pid_task(find_vpid(target_pid), PIDTYPE_PID); // Find task_struct for target process
    // find_vpid() returns a pid_t with virtual pid as input arg, which is a struct pid.
    // pid_task() returns a pointer to the task_struct of the process with the given pid.

    if (task != NULL) { // If task_struct is found
        printk(KERN_INFO "Searching for mapping in process with PID %d\n", target_pid);
        print_physical_address(task, target_vaddr); // Find physical address of target virtual address
    } else {
        printk(KERN_INFO "Process with PID %d not found.\n", target_pid);
    }

    rcu_read_unlock(); // Unlock the RCU read lock

    return 0;
}

static void __exit mapping_module_exit(void) { // Module exit point
    printk(KERN_INFO "Mapping Module Unloaded\n");
}

module_init(mapping_module_init);
module_exit(mapping_module_exit);
