#include <linux/module.h>  
#include <linux/types.h>  
#include <linux/fs.h>  
#include <linux/errno.h>  
#include <linux/mm.h>  
#include <linux/sched.h>  
#include <linux/init.h>  
#include <linux/cdev.h>  
#include <asm/io.h>  
#include <asm/uaccess.h>  
#include <linux/timer.h>  
#include <asm/atomic.h>  
#include <linux/slab.h>  
#include <linux/device.h>  
#include <linux/delay.h>
#include <linux/kthread.h>

#define thread_num 2
static struct task_struct *tsk[thread_num];

struct rwsem_waiter {
        struct task_struct *task;
};
struct rwsem_waiter waiter;

static int wake_function(void *data)
{
        struct task_struct *tsk;

        set_current_state(TASK_INTERRUPTIBLE);
        set_cpus_allowed_ptr(current, cpumask_of(1));
        do {
retry:
                tsk = waiter.task;
                if (tsk == NULL) {
                        schedule_timeout(usecs_to_jiffies(10));
                        goto retry;
                }
                smp_mb();
                waiter.task = NULL;
                wake_up_process(tsk);
                schedule_timeout(usecs_to_jiffies(100));
        } while(!kthread_should_stop());

        return 0;
}

static int sleep_function(void *data)
{
        unsigned long cnt = 0;
        struct task_struct *tsk;

        set_cpus_allowed_ptr(current, cpumask_of(0));
again:
        tsk = current;
        waiter.task = tsk;
        if (kthread_should_stop())
                return 0;

        while (true) {
                set_task_state(tsk, TASK_UNINTERRUPTIBLE);
                if (!waiter.task)
                        break;
                schedule();
                //printk("cnt:%lu\n", ++cnt);
        }
        set_task_state(tsk, TASK_RUNNING);
        goto again;

        return 0;
}

int driver_test_init(void)
{
        tsk[0] = kthread_run(sleep_function, NULL, "%s_%d",
                        "thread", 0);
        if (IS_ERR(tsk[0])) {
                pr_info("create kthread_%d failed!\n", 0);  
        } else {
                pr_info("create ktrhead_%d ok!\n", 0);
        }

        ssleep(1);
        tsk[1] = kthread_run(wake_function, NULL, "%s_%d",
                        "thread", 1);
        if (IS_ERR(tsk[1])) {
                pr_info("create kthread_%d failed!\n", 1);  
        } else {
                pr_info("create ktrhead_%d ok!\n", 1);
        }

        return 0;
}

void driver_test_exit(void)
{
        int i, ret;

        for (i = 0; i < thread_num; i++) {
                if (!IS_ERR(tsk[i])) {
                        ret = kthread_stop(tsk[i]);  
                        pr_info("Thread_%d has stopped ,return %d\n",
                                       i, ret);  
                }  
        }
        printk("exit\n");
}

MODULE_LICENSE("Dual BSD/GPL");   
module_init(driver_test_init);
module_exit(driver_test_exit);
