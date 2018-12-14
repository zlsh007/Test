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

struct timer_list bucket_tl;
static DEFINE_SPINLOCK(driver_lock);
#define BUCKET_TIMEOUT      5*HZ

static void timeout_fn(unsigned long bi)
{

        printk("timeout fun\n");
}

static void driver_init_timer(void)
{
        unsigned long flags;
        pr_info("Init timer\n");
        init_timer(&bucket_tl);
        bucket_tl.data = 0;
        bucket_tl.function = timeout_fn;
        bucket_tl.expires = jiffies + BUCKET_TIMEOUT;
        add_timer(&bucket_tl);
        spin_lock_irqsave(&driver_lock, flags);
        printk("Before schedule\n");
        schedule();
        printk("After schedule\n");
        spin_unlock_irqrestore(&driver_lock, flags);
}

int driver_test_init(void)
{
        driver_init_timer();
        return 0;
}

void driver_test_exit(void)
{
        printk("exit\n");
}

MODULE_LICENSE("Dual BSD/GPL");   
module_init(driver_test_init);
module_exit(driver_test_exit);
