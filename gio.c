#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/proc_fs.h>
#include <asm/addrspace.h>
#include <asm/paccess.h>

#include "gio.h"

/*
 * "debug" parameter for module
 */
static unsigned int debug = 0;
module_param(debug, uint, 0444);
MODULE_PARM_DESC(debug, "set debug output level");

/*
 * device list
 */
static struct gio_device *gio_first_dev = NULL;

/*
 * bus information for sysfs
 */
static struct bus_type gio_bus = {
	.name = "gio",

	/*.hotplug = NULL,*/
	.suspend = NULL,
	.resume = NULL
};

static struct gio_slot gio_slots[GIO_NUM_SLOTS] = {{
	.base_addr   = GIO_ADDR_GFX,
	.map_size    = GIO_GFX_MAP_SIZE,
	.slot_name   = "GFX",
}, {
	.base_addr   = GIO_ADDR_GIO1,
	.map_size    = GIO_GIO1_MAP_SIZE,
	.slot_name   = "EXP0",
}, {
	.base_addr   = GIO_ADDR_GIO2,
	.map_size    = GIO_GIO2_MAP_SIZE,
	.slot_name   = "EXP1"
}};

static ssize_t gio_show_id(struct device *dev, char *buf)
{
	struct gio_device *gdev = to_gio_device (dev);
	return sprintf (buf, "%02X\n", gdev->id);
}

static DEVICE_ATTR(id, S_IRUGO, gio_show_id, NULL);

static ssize_t gio_show_slot_name(struct device *dev, char *buf)
{
	struct gio_device *gdev = to_gio_device (dev);
	return sprintf (buf, "%s\n", gdev->slot_name);
}

static DEVICE_ATTR(slot_name, S_IRUGO, gio_show_slot_name, NULL);

static struct gio_device *gio_attach_dev(struct gio_device *list,
	struct gio_device *dev)
{
	struct gio_device *tmpdev;

	if(list == NULL) return dev;

	tmpdev = list;
	while(tmpdev->next)
		tmpdev = tmpdev->next;
	tmpdev->next = dev;

	return list;
}

static void gio_device_release(struct device *dev)
{
}

static int __init gio_init_device(struct gio_device *gdev, unsigned int slot)
{
	unsigned int id, addr;

	addr = KSEG1ADDR(gio_slots[slot].base_addr);
	if(get_dbe(id, (u32 *) addr)) {
		return -ENODEV;
	}

	memset(gdev, 0, sizeof(*gdev));

	gdev->slot = slot;
	gdev->id = GIO_IDCODE(id);
	sprintf(gdev->slot_name, "%s", gio_slots[slot].slot_name);
	gdev->dev.bus = &gio_bus;
	gdev->dev.release = gio_device_release;
	sprintf (gdev->dev.bus_id, "00:%02X", slot);

	printk ("GIO: Card 0x%02x @ 0x%08lx (%s slot)\n", gdev->id,
		gio_slots[slot].base_addr, gio_slots[slot].slot_name);

	if (device_register (&gdev->dev)) {
		kfree(gdev);
		return -1;
	}

	gio_first_dev = gio_attach_dev(gio_first_dev, gdev);

	device_create_file (&gdev->dev, &dev_attr_id);
	device_create_file (&gdev->dev, &dev_attr_slot_name);

	return 0;
}

static int __init gio_init(void)
{
	unsigned int i, found = 0;
#if 0
	id, found = 0, serial, j, k, tmp;
#endif
	struct gio_device *gdev;

	/* register bus in sysfs */
	bus_register(&gio_bus);

	printk("GIO: Scanning for GIO cards...\n");
	for(i = 0; i < GIO_NUM_SLOTS; i ++) {
		if(!(gdev = kmalloc (sizeof (*gdev), GFP_KERNEL))) {
			printk(KERN_WARNING "GIO: cannot allocate memory\n");
			return -ENOMEM;
		}

		if (gio_init_device (gdev, i)) {
			kfree(gdev);
			continue;
		}

		found = 1;
	}

	if(!found) {
		printk("GIO: No GIO cards present.\n");
		bus_unregister(&gio_bus);
		return -EINVAL;
	}

#if 0
		found = 1;
		gio_slot[i].device = GIO_IDCODE(id);
		if (id & GIO_ALL_BITS_VALID) {
			gio_slot[i].revision = GIO_REV(id);
			gio_slot[i].vendor = GIO_VENDOR_CODE(id);
			gio_slot[i].flags =
				(id & GIO_GIO_SIZE_64) ? GIO_IFACE_64 : 0 |
				(id & GIO_ROM_PRESENT) ? GIO_HAS_ROM : 0;
		} else
			gio_slot[i].flags = GIO_VALID_ID_ONLY;


		if(id & GIO_ALL_BITS_VALID)
			printk("GIO: \trevision %d, vendor 0x%d, flags 0x%x\n",
				gio_slot[i].revision, gio_slot[i].vendor, gio_slot[i].flags);

		serial = *(volatile unsigned int *)
			(KSEG1ADDR(gio_slot[i].base_addr) + 0x4);

		if(debug && serial)
			printk("GIO: \tserial number: 0x%08x\n", serial);

		if(debug > 100) {
			for(j = 0; j < gio_slot[i].map_size; j += 16) {
				printk("GIO: 0x%08lx: ", gio_slot[i].base_addr + j);
				for(k = 0; k < 16; k += 4) {
					if(0 == get_dbe(tmp,
						(u32 *) KSEG1ADDR(gio_slot[i].base_addr + k + j))) {

						printk("0x%08x ", tmp);
					} else {
						printk("0xXXXXXXXX ");
					}
				}
				printk("\n");
			}
		} /* debug > 100 */
	}

#endif

	return 0;
}

static void __exit gio_exit(void)
{
	struct gio_device *gdev, *gnext;

	printk("GIO: unloading module\n");

	/* unregister device files */
	gdev = gio_first_dev;
	while(gdev) {
		if(debug)
			printk("GIO: \tremoving card %02x\n", gdev->id);

		gnext = gdev->next;

		device_remove_file(&gdev->dev, &dev_attr_id);
		device_remove_file(&gdev->dev, &dev_attr_slot_name);
		device_unregister(&gdev->dev);
		kfree(gdev);

		gdev = gnext;
	}

	/* unregister bus */
	bus_unregister(&gio_bus);
}

module_init(gio_init);
module_exit(gio_exit);

MODULE_DESCRIPTION("GIO bus driver for SGI IP22 (and others?)");
MODULE_LICENSE("GPL");

