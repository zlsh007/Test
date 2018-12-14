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

struct driver_test {
        wait_queue_head_t   misc_wait;
};
struct driver_test *test;

int driver_test_init(void)
{
        int cnt = 0;

        DEFINE_WAIT(wait);
        test = kmalloc(sizeof(*test), GFP_ATOMIC);
        init_waitqueue_head(&test->misc_wait);
        for (;;) {
                if (cnt > 10)
                        break;
                prepare_to_wait(&test->misc_wait, &wait, TASK_UNINTERRUPTIBLE);
                wake_up(&test->misc_wait);
                ssleep(5);
                smp_mb();
                printk("---A---\n");
                schedule();
                printk("---B---\n");
                cnt++;
        }
        finish_wait(&test->misc_wait, &wait);
        return 0;
}
void driver_test_exit(void)
{
        wake_up(&test->misc_wait);
        printk("exit\n");
}

MODULE_LICENSE("Dual BSD/GPL");   
module_init(driver_test_init);
module_exit(driver_test_exit);
