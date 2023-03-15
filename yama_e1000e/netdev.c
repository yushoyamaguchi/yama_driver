

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

static struct sockaddr default_mac_addr = {
	.sa_family = 0,
	.sa_data = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}
};

unsigned int yama_er32(struct yama_e1000e_adapter *adapter, u_int16_t reg){
	return readl((volatile uint32_t *)(adapter->mmio_base + reg));
}

void yama_ew32(struct yama_e1000e_adapter *adapter, u_int16_t reg, uint32_t val){
	writel(val,(volatile uint32_t *)(adapter->mmio_base + reg));
}

void rx_init(struct net_device *ndev){

}

void tx_init(struct net_device *ndev){
	
}

int yama_e1000e_netdev_open(struct net_device *ndev){
	rx_init(ndev);
	tx_init(ndev);

	netif_start_queue(ndev);
	netif_carrier_on(ndev);
	return 0;
}

int yama_e1000e_netdev_close(struct net_device *ndev){
	return 0;
}



static netdev_tx_t yama_e1000e_start_xmit(struct sk_buff *skb, struct net_device *dev){
	return NETDEV_TX_OK;
}

static int yama_e1000e_set_mac_addr(struct net_device *ndev, void *p){
	struct sockaddr *addr=p;
	if(!is_valid_ether_addr(addr->sa_data)){
		return -EADDRNOTAVAIL;
	}
	eth_hw_addr_set(ndev,addr->sa_data);
	return 0;
}

static const struct net_device_ops yama_e1000e_netdev_ops = {
	.ndo_open = yama_e1000e_netdev_open,
	.ndo_stop = yama_e1000e_netdev_close,
	.ndo_start_xmit = yama_e1000e_start_xmit,
	.ndo_set_mac_address=yama_e1000e_set_mac_addr
};

void dump_about_bar(uint32_t base,struct pci_dev *pdev){
	if(base&PCI_BASE_ADDRESS_SPACE){
		printk("IO BASE : 0x%x\n",base&PCI_BASE_ADDRESS_IO_MASK);
	}
	else{
		uint32_t bar_32;
		uint64_t bar_64;
		uint32_t bar_upper;
		switch (base&PCI_BASE_ADDRESS_MEM_TYPE_MASK)
		{
		case PCI_BASE_ADDRESS_MEM_TYPE_32:
			bar_32=base&PCI_BASE_ADDRESS_MEM_MASK;
			printk("MEM BASE 32BIT : 0x%x\n",bar_32);
			break;
		case PCI_BASE_ADDRESS_MEM_TYPE_1M:
			bar_32=base&PCI_BASE_ADDRESS_MEM_MASK;
			printk("MEM BASE 1M : 0x%x\n",bar_32);
			break;	
		case PCI_BASE_ADDRESS_MEM_TYPE_64:
			pci_read_config_dword(pdev,PCI_BASE_ADDRESS_1,&bar_upper);
			bar_64=(bar_upper<<32)+(base&PCI_BASE_ADDRESS_MEM_MASK);
			printk("MEM BASE 64BIT : 0x%llx\n",bar_64);
			break;

		default:
			break;
		}
	}
}

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
	netdev = alloc_etherdev(sizeof(struct yama_e1000e_adapter));
	if(!netdev){
		return -ENOMEM;
	}
	netdev->netdev_ops=&yama_e1000e_netdev_ops;
	adapter=netdev_priv(netdev);
	adapter->netdev=netdev;
	uint32_t base_buff;
	pci_read_config_dword(pdev,PCI_BASE_ADDRESS_0,&base_buff);
	dump_about_bar(base_buff,pdev);
	adapter->mmio_base=pci_iomap(pdev,0,pci_resource_len(pdev,0));
	dump_about_bar(adapter->mmio_base,pdev);
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
	pci_disable_device(pdev);
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


