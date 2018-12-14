#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/list.h>

unsigned char *pagemem;
unsigned char *kmallocmem;
unsigned char *vmallocmem;

struct test {
        struct list_head list;
};

struct node {
        struct list_head list_node;
};

int __init mem_module_init(void)
{
        struct test test;
        struct node *node, *tmp;

        node = (struct node *)kmalloc(sizeof(*node), GFP_KERNEL);

        INIT_LIST_HEAD(&test.list);
        printk("init: test.list.next:%p, prev:%p\n\n", test.list.next,
                        test.list.prev);
        list_add_tail(&node->list_node, &test.list);
        printk("add tail: test.list.next:%p, prev:%p\n", test.list.next,
                        test.list.prev);
        printk("add tail: node.list.next:%p, prev:%p\n\n", node->list_node.next,
                        node->list_node.prev);
        tmp = list_entry(test.list.next, struct node, list_node);
        printk("list entry: tmp->list.next:%p, prev:%p\n\n", tmp->list_node.next,
                        tmp->list_node.prev);
        list_del(&tmp->list_node);
        printk("list del: tmp->list.next:%p, prev:%p\n", tmp->list_node.next,
                        tmp->list_node.prev);
        printk("list del: test.list.next:%p, prev:%p\n\n", test.list.next,
                        test.list.prev);

        
        kfree(tmp);
	return 0;
}

void __exit mem_module_exit(void)
{
}

module_init(mem_module_init);
module_exit(mem_module_exit);
MODULE_LICENSE("GPL");
