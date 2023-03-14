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

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/pci.h>

#include "include/pci_table.h"
#include "include/e1000.h"
#include "include/yama_e1000e.h"


MODULE_LICENSE("GPL v2");

char yama_e1000e_driver_name[] = "yama_e1000e";


int yama_e1000e_netdev_open(struct net_device *dev){
	return 0;
}

int yama_e1000e_netdev_close(struct net_device *dev){
	return 0;
}


static netdev_tx_t yama_e1000e_start_xmit(struct sk_buff *skb, struct net_device *dev){
	return NETDEV_TX_OK;
}

static const struct net_device_ops yama_e1000e_netdev_ops = {
	.ndo_open = yama_e1000e_netdev_open,
	.ndo_stop = yama_e1000e_netdev_close,
	.ndo_start_xmit = yama_e1000e_start_xmit,
};

static int yama_e1000_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	printk("yama_e1000_probe start\n");
	int err,ret;
	err=pci_enable_device(pdev);
	if(err){
		return err;
	}
	//irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
	struct net_device *netdev;
	struct yama_e1000e_adapter *adapter;
	netdev = alloc_etherdev(sizeof(struct net_device));
	if(!netdev){
		return netdev
	}
	netdev->netdev_ops=&yama_e1000e_netdev_ops;
	adapter=netdev_priv(netdev);
	adapter->netdev=netdev;
	adapter->io_base=pci_resource_start(pdev);
	ret=register_netdev(netdev);
    return 0;
err_irq:
	//free_irq(adapter->spi->irq, adapter);
err_free_buf:
	//dma_free_coherent(&spi->dev, DMA_BUFFER_SIZE, adapter->dma_buf, adapter->dma_handle);
err_free_netdev:
	//free_netdev(netdev);
	return ret;
}

static void yama_e1000_remove(struct pci_dev *pdev)
{
	struct net_device *netdev = pci_get_drvdata(pdev);
	unregister_netdev(netdev);
	free_netdev(netdev);
}

/* PCI Device API Driver */
static struct pci_driver yama_e1000_driver = {
	.name     = yama_e1000e_driver_name,
	.id_table	  = yama_e1000_pci_tbl, //扱えるハードウェアのリスト
	.probe    = yama_e1000_probe,
	.remove = yama_e1000_remove
};


/**
 * e1000_init_module - Driver Registration Routine
 *
 * e1000_init_module is the first routine called when the driver is
 * loaded. All it does is register with the PCI subsystem.
 **/
static int __init yama_e1000_init_module(void)
{

	return pci_register_driver(&yama_e1000_driver);
}
module_init(yama_e1000_init_module);

/**
 * e1000_exit_module - Driver Exit Cleanup Routine
 *
 * e1000_exit_module is called just before the driver is removed
 * from memory.
 **/
static void __exit yama_e1000_exit_module(void)
{
	pci_unregister_driver(&yama_e1000_driver);
}
module_exit(yama_e1000_exit_module);