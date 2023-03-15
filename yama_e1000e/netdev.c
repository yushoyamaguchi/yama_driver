

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
	struct yama_e1000e_adapter *adapter=netdev_priv(ndev);
	//NICには物理アドレスを渡す
	yama_ew32(adapter,E1000_RDBAL, (uint32_t)(adapter->rx_dma & 0xffffffff));
	yama_ew32(adapter,E1000_RDBAH, (uint32_t)(adapter->tx_dma >> 32));
	// rx descriptor length
	yama_ew32(adapter,E1000_RDLEN, (uint32_t)(TX_RING_SIZE * sizeof(struct rx_desc)));
	//head&tail
	yama_ew32(adapter,E1000_RDH, 0);
	yama_ew32(adapter,E1000_RDT, RX_RING_SIZE-1);
	uint32_t rctl=(E1000_RCTL_BAM |
                    0); 
	yama_ew32(adapter,E1000_RCTL, 0);
}

void tx_init(struct net_device *ndev){
	struct yama_e1000e_adapter *adapter=netdev_priv(ndev);
	//NICには物理アドレスを渡す
	yama_ew32(adapter,E1000_TDBAL, (uint32_t)(adapter->tx_dma & 0xffffffff));
	yama_ew32(adapter,E1000_TDBAH, (uint32_t)(adapter->tx_dma >> 32));
	// tx descriptor length
	yama_ew32(adapter,E1000_TDLEN, (uint32_t)(TX_RING_SIZE * sizeof(struct tx_desc)));
	//head&tail
	yama_ew32(adapter,E1000_TDH, 0);
	yama_ew32(adapter,E1000_TDT, 0);
	uint32_t tctl=(E1000_TCTL_PSP |
                    E1000_TCTL_CT_HERE |
                    E1000_TCTL_COLD_HERE|
                    0); 
	yama_ew32(adapter,E1000_TCTL, 0);				
}

int yama_e1000e_netdev_open(struct net_device *ndev){
	printk("yama_e1000_open start\n");
	struct yama_e1000e_adapter *adapter=netdev_priv(ndev);
	/*rx_init(ndev);
	tx_init(ndev);
	//general setting
    uint32_t ctl_val=E1000_CTL_FD | E1000_CTL_ASDE | E1000_CTL_SLU | E1000_CTL_FRCDPLX | E1000_CTL_SPEED | E1000_CTL_FRCSPD;
    yama_ew32(adapter,E1000_CTL,ctl_val);
    //割り込み
    uint32_t ims_val=E1000_IMS_LSC | E1000_IMS_RXDMT0 | E1000_IMS_RXSEQ | E1000_IMS_RXO | E1000_IMS_RXT0;
    yama_ew32(adapter, E1000_IMS, ims_val);
    //rx_enabale
    uint32_t current_rctl=yama_er32(adapter,E1000_RCTL);
    uint32_t rctl=(E1000_RCTL_EN |
                    current_rctl);
    yama_ew32(adapter,E1000_RCTL,rctl);
    //tx_enable
    uint32_t current_tctl=yama_er32(adapter,E1000_TCTL);
    uint32_t tctl=(E1000_TCTL_EN |
                    current_tctl);
    yama_ew32(adapter,E1000_TCTL,tctl);
	if(yama_er32(adapter,E1000_TCTL)&E1000_TCTL_EN){
		printk("yama_e1000e_opened\n");
	}

	netif_start_queue(ndev);
	netif_carrier_on(ndev);*/
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

static int alloc_tx_ring(struct net_device *ndev){
	struct yama_e1000e_adapter *adapter=netdev_priv(ndev);
	struct tx_desc *tx_ring=adapter->tx_ring;
	tx_ring=dma_alloc_coherent(&(adapter->pdev->dev),RX_RING_SIZE*sizeof(struct tx_desc),&(adapter->tx_dma),GFP_KERNEL);
	if(!tx_ring){
		return -ENOMEM;
	}
	memset(tx_ring,0,TX_RING_SIZE*sizeof(struct tx_desc));
	return 0;
}

static int alloc_rx_ring(struct net_device *ndev){
	struct yama_e1000e_adapter *adapter=netdev_priv(ndev);
	struct rx_desc *rx_ring=adapter->rx_ring;
	rx_ring=dma_alloc_coherent(&(adapter->pdev->dev),RX_RING_SIZE*sizeof(struct rx_desc),&(adapter->rx_dma),GFP_KERNEL);
	if(!rx_ring){
		return -ENOMEM;
	}
	memset(rx_ring,0,RX_RING_SIZE*sizeof(struct rx_desc));
	for (int i = 0; i < RX_RING_SIZE; i++) {
        dummy[i] = dma_alloc_coherent(&(adapter->pdev->dev), sizeof(uint64_t), &(rx_ring[i].addr), GFP_KERNEL);
        if (!dummy[i]) {
            // Free previously allocated memory in case of failure
            while (i > 0) {
                i--;
                dma_free_coherent(&(adapter->pdev->dev), sizeof(uint64_t), dummy[i], rx_ring[i].addr);
            }
            dma_free_coherent(&(adapter->pdev->dev), RX_RING_SIZE * sizeof(struct rx_desc), rx_ring, adapter->rx_dma);//この処理は1回だけ
            return -ENOMEM;
        }
    }
	return 0;
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
	adapter->pdev=pdev;
	uint32_t base_buff;
	pci_read_config_dword(pdev,PCI_BASE_ADDRESS_0,&base_buff);
	dump_about_bar(base_buff,pdev);
	adapter->mmio_base=pci_iomap(pdev,0,pci_resource_len(pdev,0));
	dump_about_bar(adapter->mmio_base,pdev);
	err=alloc_tx_ring(netdev);
	if(err){
		printk("err tx_ring\n");
		goto err_free_netdev;
	}
	err=alloc_rx_ring(netdev);
	if(err){
		printk("err rx_ring\n");
		goto err_free_netdev;
	}
	ret=register_netdev(netdev);
	if(ret){
		printk("err register_netdev\n");
		goto err_free_netdev;
	}
	printk("probe end\n");
    return 0;
err_irq:
	//free_irq(adapter->spi->irq, adapter);
err_free_buf:
	//dma_free_coherent(&spi->dev, DMA_BUFFER_SIZE, adapter->dma_buf, adapter->dma_handle);
err_free_netdev:
	free_netdev(netdev);
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


