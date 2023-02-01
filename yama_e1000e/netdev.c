#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/pci.h>

MODULE_LICENSE("GPL v2");

char e1000e_driver_name[] = "yama_e1000e";

static int e1000_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    return 0;
}

/* PCI Device API Driver */
static struct pci_driver e1000_driver = {
	.name     = e1000e_driver_name,
	.probe    = e1000_probe
};


/**
 * e1000_init_module - Driver Registration Routine
 *
 * e1000_init_module is the first routine called when the driver is
 * loaded. All it does is register with the PCI subsystem.
 **/
static int __init e1000_init_module(void)
{

	return pci_register_driver(&e1000_driver);
}
module_init(e1000_init_module);

/**
 * e1000_exit_module - Driver Exit Cleanup Routine
 *
 * e1000_exit_module is called just before the driver is removed
 * from memory.
 **/
static void __exit e1000_exit_module(void)
{
	pci_unregister_driver(&e1000_driver);
}
module_exit(e1000_exit_module);