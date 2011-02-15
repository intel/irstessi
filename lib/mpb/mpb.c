/*
 * IMPORTANT - READ BEFORE COPYING, INSTALLING OR USING.
 * BY LOADING OR USING THIS SOFTWARE, YOU AGREE TO THE TERMS OF THIS INTEL
 * SOFTWARE LICENSE AGREEMENT.  IF YOU DO NOT WISH TO SO AGREE, DO NOT COPY,
 * INSTALL OR USE THIS SOFTWARE.
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2010 Intel Corporation. All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its
 * suppliers or licensors.
 *
 * Title to the Material remains with Intel Corporation or its suppliers and
 * licensors. The Material contains trade secrets and proprietary and
 * confidential information of Intel or its suppliers and licensors.
 * The Material is protected by worldwide copyright and trade secret laws and
 * treaty provisions. No part of the Material may be used, copied, reproduced,
 * modified, published, uploaded, posted, transmitted, distributed,
 * or disclosed in any way without Intel's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure
 * or delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
 */

#define _FILE_OFFSET_BITS_64
#define _LARGEFILE64_SOURCE

#include <features.h>

#if (HAVE_CONFIG_H == 1)
#include <config.h>
#endif /* HANVE_CONFIG_H */

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <asm/types.h>
#include <endian.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>

#undef __USE_BSD
#include <stdlib.h>

#include "mpb.h"
#include "machine_bytes.h"

/* The length of the signature. The signature comprises of the MPB_SIGNATURE
   string (24 chars) followed by the volume's creation data string (8 chars). */
#define MAX_SIGNATURE_LENGTH        32

/* The signature identifying IMSM metadata. */
#define MPB_SIGNATURE               "Intel Raid ISM Cfg Sig. "
#define MPB_SIGNATURE_LENGTH        24

/* The version of IMSM metadata. */
#define MPB_VERSION_RAID0           "1.0.00"
#define MPB_VERSION_RAID1           "1.1.00"
#define MPB_VERSION_MUL_VOLS        "1.2.00"
#define MPB_VERSION_3OR4_DISK_ARRAY "1.2.01"
#define MPB_VERSION_RAID5           "1.2.02"
#define MPB_VERSION_5OR6_DISK_ARRAY "1.2.04"
#define MPB_VERSION_CNG             "1.2.06"
#define MPB_VERSION_ATTRIBS         "1.3.00"

/* The length of serial number of IMSM metadata fields. */
#define MAX_RAID_SERIAL_LENGTH      16

/* The maximum number of Bad Block Management entries provided for the log. */
#define MAX_BBM_ENTRIES             254

/* */
struct mpb_disk {
    __u8 serial[MAX_RAID_SERIAL_LENGTH];
    __u32 total_blocks;
    __u32 scsi_id;
    __u32 status;
    __u32 owner_cfg_num;
    __u32 reserved[4];
} __attribute__((packed));

/* */
struct mpb_map {
    __u32 pba_of_lba0;
    __u32 blocks_per_member;
    __u32 num_data_stripes;
    __u16 blocks_per_strip;
    __u8 map_state;
    __u8 raid_level;
    __u8 num_members;
    __u8 num_domains;
    __u8 failed_disk_num;
    __u8 ddf;
    __u32 reserved[7];
    __u32 disk_ord_tbl[1];
} __attribute__((packed));

/* */
struct mpb_volume {
    __u32 curr_migr_unit;
    __u32 checkpoint_id;
    __u8 migr_state;
    __u8 migr_type;
    __u8 dirty;
    __u8 fs_state;
    __u16 verify_errors;
    __u16 bad_blocks;
    __u32 reserved[4];
    struct mpb_map map[1];
} __attribute__((packed));

/* */
struct mpb_device {
    __u8 name[MAX_RAID_SERIAL_LENGTH];
    __u32 size_low;
    __u32 size_high;
    __u32 status;
    __u32 reserved_blocks;
    __u8 migr_priority;
    __u8 num_sub_vols;
    __u8 tid;
    __u8 cng_master_disk;
    __u16 cache_policy;
    __u8 cng_state;
    __u8 cng_sub_state;
    __u32 reserved[10];
    struct mpb_volume raid_vol;
} __attribute__((packed));

/* */
struct mpb_header {
    __u8 signature[MAX_SIGNATURE_LENGTH];
    __u32 check_sum;
    __u32 mpb_size;
    __u32 family_number;
    __u32 generation;
    __u32 error_log_size;
    __u32 attributes;
    __u8 num_disks;
    __u8 num_raid_devices;
    __u8 error_log_pos;
    __u8 pool_id; /* reserved1 */
    __u32 cache_size;
    __u32 orig_family_number;
    __u32 power_cycle_count;
    __u32 bbm_log_size;
    __u32 reserved2[35];
    struct mpb_disk disk[1];
} __attribute__((packed));

/* */
struct bbm_entry {
    __u64 defective_block_start;
    __u32 spare_block_offset;
    __u16 remapped_mareked_count;
    __u16 disk_ordinal;
} __attribute__((packed));

/* */
struct bbm_log {
    __u32 signature;
    __u32 count;
    __u32 reserved_spare_block_count;
    __u32 reserved;
    __u64 first_spare_lba;
    struct bbm_entry entry[MAX_BBM_ENTRIES];
} __attribute__((packed));

#if 0 /* APW */

/* */
static __u32 mpb_calc_checksum(struct mpb_header * hdr)
{
    __u32 *ptr = (__u32 *)hdr;
    __u32 end = hdr->mpb_size / sizeof(__u32);
    __u32 sum = 0;

    while (end--) {
        sum += __le32_to_cpu(*(ptr++));
    }
    return sum - __le32_to_cpu(hdr->check_sum);
}

/* */
static ssize_t mpb_get_device_size(int fd)
{
    unsigned long long result = 0;
    if (ioctl(fd, BLKGETSIZE64, &result) != 0) {
        unsigned long size;
        if (ioctl(fd, BLKGETSIZE, &size) == 0) {
            result = (unsigned long long)size << 9;
        }
    }
    return result;
}

#endif /* APW */

/* */
static int mpb_read_header_from_device(int fd, struct mpb_header *mpb_hdr)
{
#if 0 /* APW */
    struct mpb_header * temp;
    ssize_t device_size;

    if (posix_memalign((void **)&temp, 512, 512) != 0) {
        return false;
    }
    if ((device_size = mpb_get_device_size(fd)) == 0) {
        free(temp);
        return false;
    }
    if (lseek64(fd, device_size - (512 * 2), SEEK_SET) < 0) {
        free(temp);
        return false;
    }
    if (read(fd, temp, 512) != 512) {
        free(temp);
        return false;
    }
    if (strncmp((char *)temp->signature, MPB_SIGNATURE, MPB_SIGNATURE_LENGTH) != 0) {
        free(temp);
        return false;
    }
#else /* APW */
    (void)mpb_hdr;
    (void)fd;
    return MPB_E_SUCCESS;
#endif /* APW */
}

static int mpb_write_header_to_device(int fd, struct mpb_header *mpb_hdr)
{
#if 0 /* APW */
    ssize_t device_size;

    if ((device_size = mpb_get_device_size(fd)) == 0) {
        return MPB_E_DEVICE_SIZE;
    }
    if (lseek64(fd, device_size - (512 * 2), SEEK_SET) < 0) {
        return MPB_E_SEEK64;
    }
    if (write(fd, temp, 512) != 512) {
        return MPB_E_WRITE;
    }
#else /* APW */
    (void)mpb_hdr;
    (void)fd;
    return MPB_E_SUCCESS;
#endif /* APW */
}

/* */
static int mpb_read_header(const char *path, struct mpb_header *mpb_hdr)
{
    int fd, result = MPB_E_FAILED;

    if ((fd = open(path, O_RDONLY)) >= 0) {
        result = mpb_read_header_from_device(fd, mpb_hdr);
        close(fd);
    } else {
        switch (errno) {
        case EOVERFLOW:
        case EFBIG:
            result = MPB_E_OVERFLOW;
            break;
        case EACCES:
        case EPERM:
        case ETXTBSY:
            result = MPB_E_DENIED;
            break;
        default:
            result = MPB_E_FAILED;
        }
    }
    return result;
}

/* */
static int mpb_write_header(const char *path, struct mpb_header *mpb_hdr)
{
    int fd, result = MPB_E_SUCCESS;

    if ((fd = open(path, O_WRONLY)) >= 0) {
        result = mpb_write_header_to_device(fd, mpb_hdr);
        close(fd);
    } else {
        switch (errno) {
        case EOVERFLOW:
        case EFBIG:
            result = MPB_E_OVERFLOW;
            break;
        case EACCES:
        case EPERM:
        case ETXTBSY:
            result = MPB_E_DENIED;
            break;
        default:
            result = MPB_E_FAILED;
        }
    }
    return result;
}

/* */
int mpb_assign_storage_pool(const char *path, unsigned char storage_pool)
{
    struct mpb_header mpb_hdr;
    int result;

    if ((result = mpb_read_header(path, &mpb_hdr)) == MPB_E_SUCCESS) {
        mpb_hdr.pool_id = storage_pool;
        result = mpb_write_header(path, &mpb_hdr);
    }
    return result;
}

/* */
unsigned char mpb_get_storage_pool(const char * path)
{
    (void)path;

    return 0;
}

#define MSA_SIGNATURE               "Metadata Storage Area Header"
#define MSA_SIGNATURE_LENGTH        29
#define MSA_RESERVED_BYTES          481

/* */
struct msa_header {
    __u8 signature[MSA_SIGNATURE_LENGTH];
    __u8 version;
    __u8 footer_location;
    __u8 reserved[MSA_RESERVED_BYTES];
};

/* */
int msa_write_storage_area(const char * path, void * buffer, size_t buffer_size)
{
    (void)path;
    (void)buffer_size;
    (void)buffer;

    return 0;
}

/* */
int msa_read_storage_area(const char * path, void * buffer, size_t buffer_size)
{
    (void)path;
    (void)buffer_size;
    (void)buffer;

    return 0;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=98 expandtab: */
