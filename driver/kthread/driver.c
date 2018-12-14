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

#define thread_num 1
static struct task_struct *tsk[thread_num];

static int child_fun(void *data)
{
        do {
                ssleep(3);
                pr_info("%s-%d PID:%d name:%s\n",
                                __func__, __LINE__,
                                current->pid, current->comm);
        } while(!kthread_should_stop());

        return 0;
}

static int thread_function(void *data)
{
        int ret;
        int num = *((int *)data);
        static struct task_struct *child_thread;

        child_thread = kthread_run(child_fun, NULL, "child_thread");
        if (IS_ERR(child_thread))
                pr_err("create child_thread failed!\n");
        else
                pr_info("create child_thread ok!\n");

        do {
                ssleep(1);
                pr_info("%s-%d PID:%d thread_num:%d name:%s\n",
                                __func__, __LINE__,
                                current->pid, num, current->comm);
        } while(!kthread_should_stop());

        ret = kthread_stop(child_thread);
        pr_info("child_thread has stopped, return:%d\n", ret);
        return ret;
}

int driver_test_init(void)
{
        int i;

        for (i = 0; i < thread_num; i++) {
                tsk[i] = kthread_run(thread_function, &i, "%s_%d",
                                "thread", i);
                if (IS_ERR(tsk[i])) {
                        pr_info("create kthread_%d failed!\n", i);  
                } else {
                        pr_info("create ktrhead_%d ok!\n", i);  
                }

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
