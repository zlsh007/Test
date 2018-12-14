#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>

struct slab_obj {
        int aa;
        int bb;
        int cc;
};

typedef struct slab_obj *slab_obj_t;

slab_obj_t memblk = NULL;

struct kmem_cache *myslabobj;

static void mm_create(void)
{
        myslabobj =
                kmem_cache_create("my_slab_obj", sizeof(struct slab_obj), 0,
                                SLAB_HWCACHE_ALIGN, NULL);
        memblk = kmem_cache_alloc(myslabobj, GFP_KERNEL);
        memblk->aa = 0xabcd;
        memblk->bb = 0x1234;
        memblk->cc = 0x6789;
}

static void mm_destroy(void)
{
        //kfree(memblk);
        //kmem_cache_destroy(myslabobj);
}

static int __init slub_debug_init(void)
{
        mm_create();
        return 0;
}

static void __exit slub_debug_exit(void)
{
        mm_destroy();
}

module_init(slub_debug_init);
module_exit(slub_debug_exit);
MODULE_LICENSE("GPL");
