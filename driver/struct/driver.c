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

struct ppl_header_entry {
        __le64 data_sector;     /* raid sector of the new data */
        __le32 pp_size;         /* length of partial parity */
        __le32 data_size;       /* length of data */
        __le32 parity_disk;     /* member disk containing parity */
        __le32 checksum;        /* checksum of partial parity data for this
                                 * entry (~crc32c) */
} __attribute__ ((__packed__));

#define PPL_HEADER_SIZE 4096
#define PPL_HDR_RESERVED 512
#define PPL_HDR_ENTRY_SPACE \
        (PPL_HEADER_SIZE - PPL_HDR_RESERVED - 4 * sizeof(__le32) - sizeof(__le64))
#define PPL_HDR_MAX_ENTRIES \
        (PPL_HDR_ENTRY_SPACE / sizeof(struct ppl_header_entry))

struct ppl_header {
        __u8 reserved[PPL_HDR_RESERVED];/* reserved space, fill with 0xff */
        __le32 signature;               /* signature (family number of volume) */
        __le32 padding;                 /* zero pad */
        __le64 generation;              /* generation number of the header */
        __le32 entries_count;           /* number of entries in entry array */
        __le32 checksum;                /* checksum of the header (~crc32c) */
        struct ppl_header_entry entries[PPL_HDR_MAX_ENTRIES];
} __attribute__ ((__packed__));

struct test_struct {
        __u8 reserved[PPL_HDR_RESERVED];/* reserved space, fill with 0xff */
        char test[8];
} __attribute__ ((__packed__));

int driver_test_init(void)
{
        struct page *page;
        struct ppl_header *pplhdr;
        int i = 0;

        page = alloc_page(GFP_NOIO | __GFP_ZERO);
        if (!page)
                return -ENOMEM;

        pplhdr = page_address(page);
        pr_info("sizeof pplhdr:%ld\n", sizeof(*pplhdr));
        pr_info("sizeof test:%ld\n", sizeof(struct test_struct));
        pr_info("sizeof struct ppl_header:%ld\n", sizeof(struct ppl_header));
        pr_info("PPL_HDR_MAX_ENTRIES:%ld\n", PPL_HDR_MAX_ENTRIES);
        for (; i < 512; i++) {
                if (i && !(i%32))
                        pr_info("\n");
                printk("0x%02x ", *((char *)pplhdr++));
        }
        pr_info("\n");
        pr_info("--------------------------------------\n");
        pr_info("--------------------------------------\n");
        i = 0;
        pplhdr = page_address(page);
        memset(pplhdr->reserved, 0xff, PPL_HDR_RESERVED);
        for (; i < 512; i++) {
                if (i && !(i%32))
                        pr_info("\n");
                printk("0x%02x ", pplhdr->reserved[i]);
        }
        pr_info("\n");
        pr_info("--------------------------------------\n");
        pr_info("--------------------------------------\n");
        i = 0;
        pplhdr = page_address(page);

        pplhdr->signature = 0xAA;
        pplhdr->checksum = 0xBB;

        for (; i < 4096; i++) {
                if (i && !(i%32))
                        pr_info("\n");
                printk("0x%02x ", *((char *)pplhdr++));
        }
        pr_info("\n");
        return 0;
}
void driver_test_exit(void)
{
        printk("exit\n");
}

MODULE_LICENSE("Dual BSD/GPL");   
module_init(driver_test_init);
module_exit(driver_test_exit);
