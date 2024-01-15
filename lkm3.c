#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/pgtable.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shantanu Welling");
MODULE_DESCRIPTION("TRANSLATION Module");

static int target_pid = 0;
module_param(target_pid, int, 0644);
MODULE_PARM_DESC(target_pid, "Target process ID");

static unsigned long target_vaddr = 0;
module_param(target_vaddr, ulong, 0644);
MODULE_PARM_DESC(target_vaddr, "Target virtual address");

static void print_physical_address(struct task_struct *task, unsigned long vaddr) {
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    phys_addr_t paddr;

    pgd = pgd_offset(task->mm, vaddr);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        printk(KERN_INFO "Invalid PGD entry\n");
        return;
    }

    p4d = p4d_offset(pgd, vaddr);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) {
        printk(KERN_INFO "Invalid P4D entry\n");
        return;
    }

    pud = pud_offset(p4d, vaddr);
    if (pud_none(*pud) || pud_bad(*pud)) {
        printk(KERN_INFO "Invalid PUD entry\n");
        return;
    }

    pmd = pmd_offset(pud, vaddr);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        printk(KERN_INFO "Invalid PMD entry\n");
        return;
    }

    pte = pte_offset_kernel(pmd, vaddr);
    if (!pte || !pte_present(*pte)) {
        printk(KERN_INFO "Page not present or pte not found\n");
        return;
    }

    paddr = (phys_addr_t)(pte_val(*pte) & PAGE_MASK) + (vaddr & ~PAGE_MASK);

    printk(KERN_INFO "PID: %d, Virtual Address: 0x%lx, Physical Address: 0x%llx\n",
           task->pid, vaddr, (unsigned long long)paddr);
}

static int __init mapping_module_init(void) {
    struct task_struct *task;

    rcu_read_lock();

    task = pid_task(find_vpid(target_pid), PIDTYPE_PID);
    if (task != NULL) {
        printk(KERN_INFO "Searching for mapping in process with PID %d\n", target_pid);
        print_physical_address(task, target_vaddr);
    } else {
        printk(KERN_INFO "Process with PID %d not found.\n", target_pid);
    }

    rcu_read_unlock();

    return 0;
}

static void __exit mapping_module_exit(void) {
    printk(KERN_INFO "Mapping Module Unloaded\n");
}

module_init(mapping_module_init);
module_exit(mapping_module_exit);
