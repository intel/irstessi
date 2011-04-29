
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


// -*- mode: c; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*-
// ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab:

/* sgio.c - by Mark Lord (C) 2007 -- freely distributable */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <scsi/scsi.h>
#include <scsi/sg.h>
#include <linux/hdreg.h>

#include "sgio.h"

/*
 * Taskfile layout for SG_ATA_16 cdb:
 *
 * LBA48:
 * cdb[ 3] = hob_feature
 * cdb[ 5] = hob_nsect
 * cdb[ 7] = hob_lbal
 * cdb[ 9] = hob_lbam
 * cdb[11] = hob_lbah
 *
 * LBA28/LBA48:
 * cdb[ 4] = feature
 * cdb[ 6] = nsect
 * cdb[ 8] = lbal
 * cdb[10] = lbam
 * cdb[12] = lbah
 * cdb[13] = device
 * cdb[14] = command
 *
 * dxfer_direction choices:
 *  SG_DXFER_TO_DEV, SG_DXFER_FROM_DEV, SG_DXFER_NONE
 */

void tf_init (struct ata_tf *tf, __u8 ata_op, __u64 lba, unsigned int nsect)
{
    const __u64 lba28_mask = 0x0fffffff;

    memset(tf, 0, sizeof(*tf));
    tf->command = ata_op;
    tf->dev     = ATA_USING_LBA;
    if (lba) {
        tf->lob.lbal = lba;
        tf->lob.lbam = lba >>  8;
        tf->lob.lbah = lba >> 16;
        if ((lba & ~lba28_mask) == 0) {
            tf->dev |= (lba >> 24) & 0x0f;
        } else {
            tf->hob.lbal = lba >> 24;
            tf->hob.lbam = lba >> 32;
            tf->hob.lbah = lba >> 40;
            tf->is_lba48 = 1;
        }
    }
    if (nsect) {
        tf->lob.nsect = nsect;
        if ((lba & ~lba28_mask) != 0)
            tf->hob.nsect = nsect >> 8;
    }
}

#ifdef SG_IO

__u64 tf_to_lba (struct ata_tf *tf)
{
    __u32 lba24, lbah;
    __u64 lba64;

    lba24 = (tf->lob.lbah << 16) | (tf->lob.lbam << 8) | (tf->lob.lbal);
    if (tf->is_lba48)
        lbah = (tf->hob.lbah << 16) | (tf->hob.lbam << 8) | (tf->hob.lbal);
    else
        lbah = (tf->dev & 0x0f);
    lba64 = (((__u64)lbah) << 24) | (__u64)lba24;
    return lba64;
}

enum {
    SG_CDB2_TLEN_NODATA = 0 << 0,
    SG_CDB2_TLEN_FEAT   = 1 << 0,
    SG_CDB2_TLEN_NSECT  = 2 << 0,

    SG_CDB2_TLEN_BYTES  = 0 << 2,
    SG_CDB2_TLEN_SECTORS    = 1 << 2,

    SG_CDB2_TDIR_TO_DEV = 0 << 3,
    SG_CDB2_TDIR_FROM_DEV   = 1 << 3,

    SG_CDB2_CHECK_COND  = 1 << 5
};

static int bad_sense (unsigned char *sb, int len)
{
    (void)sb;
    (void)len;

    return EIO;
}

int sg16 (int fd, int rw, struct ata_tf *tf,
    void *data, unsigned int data_bytes, unsigned int timeout_secs)
{
    unsigned char cdb[SG_ATA_16_LEN];
    unsigned char sb[32], *desc;
    struct scsi_sg_io_hdr io_hdr;

    memset(&cdb, 0, sizeof(cdb));
    cdb[ 0] = SG_ATA_16;
    cdb[ 1] = data ? (rw ? SG_ATA_PROTO_PIO_OUT : SG_ATA_PROTO_PIO_IN) : SG_ATA_PROTO_NON_DATA;
#if 0
    if (data_bytes && (data_bytes % 512) == 0) {
        cdb[1] = SG_ATA_PROTO_DMA;
    }
#endif
    cdb[ 2] = SG_CDB2_CHECK_COND;
    if (data) {
        cdb[2] |= SG_CDB2_TLEN_NSECT | SG_CDB2_TLEN_SECTORS;
        cdb[2] |= rw ? SG_CDB2_TDIR_TO_DEV : SG_CDB2_TDIR_FROM_DEV;
    }
    cdb[ 4] = tf->lob.feat;
    cdb[ 6] = tf->lob.nsect;
    cdb[ 8] = tf->lob.lbal;
    cdb[10] = tf->lob.lbam;
    cdb[12] = tf->lob.lbah;
    cdb[13] = tf->dev;
    cdb[14] = tf->command;
    if (tf->is_lba48) {
        cdb[ 1] |= SG_ATA_LBA48;
        cdb[ 3]  = tf->hob.feat;
        cdb[ 5]  = tf->hob.nsect;
        cdb[ 7]  = tf->hob.lbal;
        cdb[ 9]  = tf->hob.lbam;
        cdb[11]  = tf->hob.lbah;
    }

    memset(&sb,     0, sizeof(sb));
    memset(&io_hdr, 0, sizeof(struct scsi_sg_io_hdr));
    io_hdr.interface_id = 'S';
    io_hdr.cmd_len      = SG_ATA_16_LEN;
    io_hdr.mx_sb_len    = sizeof(sb);
    io_hdr.dxfer_direction  = data ? (rw ? SG_DXFER_TO_DEV : SG_DXFER_FROM_DEV) : SG_DXFER_NONE;
    io_hdr.dxfer_len    = data ? data_bytes : 0;
    io_hdr.dxferp       = data;
    io_hdr.cmdp     = cdb;
    io_hdr.sbp      = sb;
    io_hdr.pack_id      = tf_to_lba(tf);
    io_hdr.timeout      = (timeout_secs ? timeout_secs : 5) * 1000; /* msecs */

    if (ioctl(fd, SG_IO, &io_hdr) == -1) {
        return -1;  /* SG_IO not supported */
    }

    if (io_hdr.host_status || io_hdr.driver_status != SG_DRIVER_SENSE
     || (io_hdr.status && io_hdr.status != SG_CHECK_CONDITION))
    {
        errno = EIO;
        return -1;
    }

    if (sb[0] != 0x72 || sb[7] < 14)
        return bad_sense(sb, sizeof(sb));

    desc = sb + 8;

    if (desc[0] != 9 || desc[1] < 12)
        return bad_sense(sb, sizeof(sb));

    tf->is_lba48  = desc[ 2] & 1;
    tf->error     = desc[ 3];
    tf->lob.nsect = desc[ 5];
    tf->lob.lbal  = desc[ 7];
    tf->lob.lbam  = desc[ 9];
    tf->lob.lbah  = desc[11];
    tf->dev       = desc[12];
    tf->status    = desc[13];
    if (tf->is_lba48) {
        tf->hob.nsect = desc[ 4];
        tf->hob.lbal  = desc[ 6];
        tf->hob.lbam  = desc[ 8];
        tf->hob.lbah  = desc[10];
    }
#if 0 /*FIXME: we have to check this somewhere !! */
    if (tf->status & (ATA_STAT_ERR | ATA_STAT_DRQ)) {
        errno = EIO;
        return -1;
    }
#endif
    return 0;
}

#endif /* SG_IO */

int do_drive_cmd (int fd, unsigned char *args)
{
#ifdef SG_IO

    struct ata_tf tf;
    void *data = NULL;
    unsigned int data_bytes = 0;
    int rc;

    if (args == NULL)
        goto use_legacy_ioctl;
    /*
     * Reformat and try to issue via SG_IO:
     */
    if (args[3]) {
        data_bytes = args[3] * 512;
        data       = args + 4;
    }
    tf_init(&tf, args[0], 0, args[1]);
    tf.lob.feat = args[2];
    if (tf.command == ATA_OP_SMART) {
        tf.lob.nsect = args[3];
        tf.lob.lbal  = args[1];
        tf.lob.lbam  = 0x4f;
        tf.lob.lbah  = 0xc2;
    }

    rc = sg16(fd, SG_READ, &tf, data, data_bytes, 0);
    if (rc) {
        if (rc == -1)
            goto use_legacy_ioctl;
        errno = rc;
        rc = -1;
    } else if (tf.status & (ATA_STAT_ERR | ATA_STAT_DRQ)) {
        errno = EIO;
        rc = -1;
    }

    args[0] = tf.status;
    args[1] = tf.error;
    args[2] = tf.lob.nsect;
    return rc;

use_legacy_ioctl:
#endif /* SG_IO */
    return ioctl(fd, HDIO_DRIVE_CMD, args);
}

int do_taskfile_cmd (int fd, struct hdio_taskfile *r, unsigned int timeout_secs)
{
    int rc;
#ifdef SG_IO
    struct ata_tf tf;
    void *data = NULL;
    unsigned int data_bytes = 0;
    int rw = SG_READ;
    /*
     * Reformat and try to issue via SG_IO:
     */
    tf_init(&tf, 0, 0, 0);
    if (r->oflags.b.feat)   tf.lob.feat  = r->lob.feat;
    if (r->oflags.b.lbal)   tf.lob.lbal  = r->lob.lbal;
    if (r->oflags.b.nsect)  tf.lob.nsect = r->lob.nsect;
    if (r->oflags.b.lbam)   tf.lob.lbam  = r->lob.lbam;
    if (r->oflags.b.lbah)   tf.lob.lbah  = r->lob.lbah;
    if (r->oflags.b.dev)    tf.dev       = r->lob.dev;
    if (r->oflags.b.command) tf.command  = r->lob.command;
    if ((r->oflags.all >> 8) || (r->iflags.all >> 8)) {
        tf.is_lba48 = 1;
        if (r->oflags.b.hob_feat)   tf.hob.feat  = r->hob.feat;
        if (r->oflags.b.hob_lbal)   tf.hob.lbal  = r->hob.lbal;
        if (r->oflags.b.hob_nsect)  tf.hob.nsect = r->hob.nsect;
        if (r->oflags.b.hob_lbam)   tf.hob.lbam  = r->hob.lbam;
        if (r->oflags.b.hob_lbah)   tf.hob.lbah  = r->hob.lbah;
    }
    switch (r->cmd_req) {
        case TASKFILE_CMD_REQ_OUT:
        case TASKFILE_CMD_REQ_RAW_OUT:
            data_bytes = r->obytes;
            data       = r->data;
            rw         = SG_WRITE;
            break;
        case TASKFILE_CMD_REQ_IN:
            data_bytes = r->ibytes;
            data       = r->data;
            break;
    }

    rc = sg16(fd, rw, &tf, data, data_bytes, timeout_secs);
    if (rc) {
        if (rc == -1)
            goto use_legacy_ioctl;
        errno = rc;
        rc = -1;
    } else if (tf.status & (ATA_STAT_ERR | ATA_STAT_DRQ)) {
        errno = EIO;
        rc = -1;
    }

    if (r->iflags.b.feat)       r->lob.feat  = tf.error;
    if (r->iflags.b.lbal)       r->lob.lbal  = tf.lob.lbal;
    if (r->iflags.b.nsect)      r->lob.nsect = tf.lob.nsect;
    if (r->iflags.b.lbam)       r->lob.lbam  = tf.lob.lbam;
    if (r->iflags.b.lbah)       r->lob.lbah  = tf.lob.lbah;
    if (r->iflags.b.dev)        r->lob.dev   = tf.dev;
    if (r->iflags.b.command)    r->lob.command = tf.status;
    if (r->iflags.b.hob_feat)   r->hob.feat  = tf.hob.feat;
    if (r->iflags.b.hob_lbal)   r->hob.lbal  = tf.hob.lbal;
    if (r->iflags.b.hob_nsect)  r->hob.nsect = tf.hob.nsect;
    if (r->iflags.b.hob_lbam)   r->hob.lbam  = tf.hob.lbam;
    if (r->iflags.b.hob_lbah)   r->hob.lbah  = tf.hob.lbah;
    return rc;

use_legacy_ioctl:
#else
    timeout_secs = 0;   /* keep compiler happy */
#endif /* SG_IO */
    errno = 0;
    rc = ioctl(fd, HDIO_DRIVE_TASKFILE, r);
    return rc;
}

void init_hdio_taskfile (struct hdio_taskfile *r, __u8 ata_op, int rw, int force_lba48,
                __u64 lba, unsigned int nsect, int data_bytes)
{
    const __u64 lba28_mask = 0x0fffffff;

    memset(r, 0, sizeof(struct hdio_taskfile) + data_bytes);
    if (!data_bytes) {
        r->dphase  = TASKFILE_DPHASE_NONE;
        r->cmd_req = TASKFILE_CMD_REQ_NODATA;
    } else if (rw == RW_WRITE) {
        r->dphase  = TASKFILE_DPHASE_PIO_OUT;
        r->cmd_req = TASKFILE_CMD_REQ_RAW_OUT;
        r->obytes  = data_bytes;
    } else { /* rw == RW_READ */
        r->dphase  = TASKFILE_DPHASE_PIO_IN;
        r->cmd_req = TASKFILE_CMD_REQ_IN;
        r->ibytes  = data_bytes;
    }
    r->lob.command      = ata_op;
    r->oflags.b.command = 1;
    r->oflags.b.dev     = 1;
    r->oflags.b.lbal    = 1;
    r->oflags.b.lbam    = 1;
    r->oflags.b.lbah    = 1;
    r->oflags.b.nsect   = 1;

    r->iflags.b.command = 1;
    r->iflags.b.feat    = 1;

    r->lob.nsect = nsect;
    r->lob.lbal  = lba;
    r->lob.lbam  = lba >>  8;
    r->lob.lbah  = lba >> 16;
    r->lob.dev   = ATA_USING_LBA;

    if ((lba & ~lba28_mask) == 0 && nsect <= 256 && !force_lba48) {
        r->lob.dev |= (lba >> 24) & 0x0f;
    } else {
        r->hob.nsect = nsect >>  8;
        r->hob.lbal  = lba   >> 24;
        r->hob.lbam  = lba   >> 32;
        r->hob.lbah  = lba   >> 40;
        r->oflags.b.hob_nsect = 1;
        r->oflags.b.hob_lbal  = 1;
        r->oflags.b.hob_lbam  = 1;
        r->oflags.b.hob_lbah  = 1;
    }
}
