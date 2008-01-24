#ifndef _GIO_H
#define _GIO_H

struct gio_device;

#define GIO_MAX_SLOTNAME_SIZE 32

struct gio_device {
	unsigned int slot;
	unsigned int id;
	char slot_name[GIO_MAX_SLOTNAME_SIZE];

	struct device dev;

	struct gio_device *next;
};

#define GIO_SLOT_GFX    0
#define GIO_SLOT_GIO1   1
#define GIO_SLOT_GIO2   2
#define GIO_NUM_SLOTS   3

#define GIO_ANY_ID      0xff

#define GIO_VALID_ID_ONLY   0x01
#define GIO_IFACE_64        0x02
#define GIO_HAS_ROM         0x04


#define GIO_IDCODE(x)       ((x) & 0x7f)
#define GIO_ALL_BITS_VALID  0x80
#define GIO_REV(x)          (((x) >> 8) & 0xff)
#define GIO_GIO_SIZE_64     0x10000
#define GIO_ROM_PRESENT     0x20000
#define GIO_VENDOR_CODE(x)  (((x) >> 18) & 0x3fff)

struct gio_slot {
	unsigned char slot_number;
	unsigned long base_addr;
	unsigned int map_size;
	char slot_name[5];
};

#define GIO_PIO_MAP_BASE    0x1f000000L
#define GIO_PIO_MAP_SIZE    (16 * 1024*1024)

#define GIO_ADDR_GFX        0x1f000000L
#define GIO_ADDR_GIO1       0x1f400000L
#define GIO_ADDR_GIO2       0x1f600000L

#define GIO_GFX_MAP_SIZE    (4 * 1024*1024)
#define GIO_GIO1_MAP_SIZE   (2 * 1024*1024)
#define GIO_GIO2_MAP_SIZE   (4 * 1024*1024)

#define GIO_NO_DEVICE       0x80

#define to_gio_device(n) container_of(n, struct gio_device, dev)

#endif /* _GIO_H */
