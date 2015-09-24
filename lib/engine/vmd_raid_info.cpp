
/*
Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/




#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <features.h>
#include <asm/types.h>
#include <cstddef>
#include <typeinfo>
#include <string.h>

#include <ssi.h>
#include <orom/orom.h>

#include "exception.h"
#include "container.h"
#include "string.h"
#include "filesystem.h"
#include "object.h"
#include "controller.h"
#include "nvme.h"
#include "raid_info.h"
#include "nvme_raid_info.h"
#include "utils.h"
#include "vmd_raid_info.h"
#include "vmd.h"


/* */
VMD_RaidInfo::VMD_RaidInfo(VMD *pVMD)
    : RaidInfo(&orom_vmd)
{
    attachController(pVMD);

	memset(&orom_vmd, 0, sizeof(orom_info));

	m_OromDevId = -1;

	//Supported Raid Levels
	orom_vmd.rlc0 = 1;
	orom_vmd.rlc1 = 1;
	orom_vmd.rlc10 = 1;
	orom_vmd.rlc5 = 1;

	//Supported Strip Size
	orom_vmd.chk4k = 1;
	orom_vmd.chk8k = 1;
	orom_vmd.chk16k = 1;
	orom_vmd.chk32k = 1;
	orom_vmd.chk64k = 1;
	orom_vmd.chk128k = 1;

	//Supported Amount of disks/volumens
	orom_vmd.tds = 12;
	orom_vmd.dpa = 12;
	orom_vmd.vphba = 4;
	orom_vmd.vpa = 2;

	//supported Attr
	orom_vmd.a_2tb_disk = 1;
	orom_vmd.a_2tb_vol = 1;

	//supported features
}

bool VMD_RaidInfo::operator ==(const Object &object) const {
    return typeid(*this) == typeid(object) &&
            static_cast<const RaidInfo *>(&object)->getControllerType() == SSI_ControllerTypeVMD;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 textwidth=80 expandtab: */
