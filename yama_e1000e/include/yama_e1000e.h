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
#include "hw.h"

struct yama_e1000e_adapter {
    struct net_device *netdev;
};