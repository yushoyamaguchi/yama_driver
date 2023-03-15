#include <linux/bitops.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <linux/netdevice.h>
#include <linux/pci.h>
#include <linux/crc32.h>
#include <linux/if_vlan.h>
#include <linux/timecounter.h>
#include <linux/net_tstamp.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/ptp_classify.h>
#include <linux/mii.h>
#include <linux/mdio.h>
#include <linux/mutex.h>
#include <linux/pm_qos.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>

#include "hw.h"

#define RX_RING_SIZE 16
#define TX_RING_SIZE 16

void *dummy[RX_RING_SIZE];

struct yama_e1000e_adapter {
    struct net_device *netdev;
    struct pci_dev *pdev;
    struct mutex lock;
	struct work_struct irq_work;
	struct work_struct tx_work;
    unsigned int mmio_base;
    dma_addr_t rx_dma;
    struct rx_desc *rx_ring;
    dma_addr_t tx_dma;
    struct tx_desc *tx_ring;
};



/* Transmit Descriptor status definitions [E1000 3.3.3.2] */
//#define E1000_TXD_STAT_DD    0x00000001 /* Descriptor Done */

struct tx_desc
{
    uint64_t buffer_address;
    uint16_t length;
    uint8_t  checksum_offset;
    uint8_t  command;
    uint8_t  status : 4;
    uint8_t  reserved : 4;
    uint8_t  checksum_start_field;
    uint16_t special;
}__attribute__((packed));



#define E1000_RXD_STAT_DD       0x01    /* Descriptor Done */
#define E1000_RXD_STAT_EOP      0x02    /* End of Packet */
struct rx_desc
{
    uint64_t addr;
    uint16_t length;
    uint16_t csum;
    uint8_t status;
    uint8_t errors; 
    uint16_t special;
}__attribute__((packed));


