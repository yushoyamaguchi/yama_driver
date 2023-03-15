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

struct yama_e1000e_adapter {
    struct net_device *netdev;
    struct pci_dev *pdev;
    struct mutex lock;
	struct work_struct irq_work;
	struct work_struct tx_work;
    unsigned int mmio_base;
};