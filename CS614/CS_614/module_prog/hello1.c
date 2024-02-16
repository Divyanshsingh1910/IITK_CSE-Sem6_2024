
#include <linux/module.h>  /* Needed by all the modules */
#include <linux/kernel.h>  /* Needed for KERN_INFO */

MODULE_LICENSE("GPL");

int init_module(void){

    printk("Hello World, I am the new module.\n");

    return 0; /* A non 0 return means init_module failed
                                    ---> module failed*/
}

void cleanup_module(void){
    printk("By kernel, see ya!\n");

}