/**************************************************************************

Copyright (c) 2006-2013, Silicom
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 3. Neither the name of the Silicom nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#ifndef BP_MOD_H
#define BP_MOD_H
#include "bits.h"

#ifndef KERNEL_VERSION
    #define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif


#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8) )
    #define msleep(x)	do { if(in_interrupt()) { \
				/* Don't mdelay in interrupt context! */ \
	                	BUG(); \
			} else { \
				set_current_state(TASK_UNINTERRUPTIBLE); \
				schedule_timeout((x * HZ)/1000 + 2); \
			} } while(0)
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10))
    #define EXPORT_SYMBOL_NOVERS EXPORT_SYMBOL
#endif

#define BP_PROC_DIR "bypass"


#define GPIO6_SET_ENTRY_SD           "gpio6_set"
#define GPIO6_CLEAR_ENTRY_SD         "gpio6_clear"

#define GPIO7_SET_ENTRY_SD           "gpio7_set"
#define GPIO7_CLEAR_ENTRY_SD         "gpio7_clear"

#define PULSE_SET_ENTRY_SD            "pulse_set"
#define ZERO_SET_ENTRY_SD            "zero_set"
#define PULSE_GET1_ENTRY_SD            "pulse_get1"
#define PULSE_GET2_ENTRY_SD            "pulse_get2"

#define CMND_ON_ENTRY_SD              "cmnd_on"
#define CMND_OFF_ENTRY_SD             "cmnd_off"
#define RESET_CONT_ENTRY_SD           "reset_cont"

/*COMMANDS*/
#define BYPASS_INFO_ENTRY_SD     "bypass_info"

#define BYPASS_SLAVE_ENTRY_SD    "bypass_slave"
#define BYPASS_CAPS_ENTRY_SD     "bypass_caps"
#define WD_SET_CAPS_ENTRY_SD     "wd_set_caps"
#define BYPASS_ENTRY_SD          "bypass"
#define BYPASS_CHANGE_ENTRY_SD   "bypass_change"
#define BYPASS_WD_ENTRY_SD       "bypass_wd"
#define WD_EXPIRE_TIME_ENTRY_SD  "wd_expire_time"
#define RESET_BYPASS_WD_ENTRY_SD "reset_bypass_wd"
#define DIS_BYPASS_ENTRY_SD      "dis_bypass"
#define BYPASS_PWUP_ENTRY_SD     "bypass_pwup"
#define BYPASS_PWOFF_ENTRY_SD     "bypass_pwoff"
#define STD_NIC_ENTRY_SD         "std_nic"
#define STD_NIC_ENTRY_SD         "std_nic"
#define TAP_ENTRY_SD             "tap"
#define TAP_CHANGE_ENTRY_SD      "tap_change"
#define DIS_TAP_ENTRY_SD         "dis_tap"
#define TAP_PWUP_ENTRY_SD        "tap_pwup"
#define TWO_PORT_LINK_ENTRY_SD   "two_port_link"
#define WD_EXP_MODE_ENTRY_SD     "wd_exp_mode"
#define WD_AUTORESET_ENTRY_SD    "wd_autoreset"
#define TPL_ENTRY_SD             "tpl"
#define WAIT_AT_PWUP_ENTRY_SD    "wait_at_pwup"
#define HW_RESET_ENTRY_SD        "hw_reset"
#define DISC_ENTRY_SD             "disc"
#define DISC_CHANGE_ENTRY_SD      "disc_change"
#define DIS_DISC_ENTRY_SD         "dis_disc"
#define DISC_PWUP_ENTRY_SD        "disc_pwup"





#ifndef jiffies_to_msecs
    #define jiffies_to_msecs(x) _kc_jiffies_to_msecs(x)
static inline unsigned int jiffies_to_msecs(const unsigned long j)
{
#if HZ <= 1000 && !(1000 % HZ)
    return(1000 / HZ) * j;
#elif HZ > 1000 && !(HZ % 1000)
    return(j + (HZ / 1000) - 1)/(HZ / 1000);
#else
    return(j * 1000) / HZ;
#endif
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10))

    #define pci_get_class pci_find_class
    #define pci_get_subsys pci_find_subsys
    #define pci_get_device pci_find_device

#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,4))
    #define MODULE_VERSION(_version)
#endif


#define SILICOM_VID              0x1374
#define SILICOM_SVID             0x1374

#define SILICOM_PXG2BPFI_SSID    0x0026
#define SILICOM_PXG2BPFILX_SSID  0x0027
#define SILICOM_PXGBPI_SSID      0x0028
#define SILICOM_PXGBPIG_SSID     0x0029
#define SILICOM_PXG2TBFI_SSID    0x002a
#define SILICOM_PXG4BPI_SSID     0x002c
#define SILICOM_PXG4BPFI_SSID    0x002d
#define SILICOM_PXG4BPFILX_SSID  0x002e
#define SILICOM_PXG2BPFIL_SSID   0x002F
#define SILICOM_PXG2BPFILLX_SSID 0x0030
#define SILICOM_PEG4BPI_SSID     0x0031
#define SILICOM_PEG2BPI_SSID     0x0037
#define SILICOM_PEG4BPIN_SSID    0x0038
#define SILICOM_PEG2BPFI_SSID    0x0039
#define SILICOM_PEG2BPFILX_SSID  0x003A
#define SILICOM_PMCXG2BPFI_SSID  0x003B
#define NOKIA_PMCXG2BPFIN_SSID   0x0510
#define NOKIA_PMCXG2BPIN_SSID    0x0513
#define NOKIA_PMCXG4BPIN_SSID    0x0514
#define NOKIA_PMCXG2BPFIN_SVID   0x13B8 
#define NOKIA_PMCXG2BPIN2_SSID    0x0515
#define NOKIA_PMCXG4BPIN2_SSID    0x0516 
#define SILICOM_PMCX2BPI_SSID       0x041
#define SILICOM_PMCX4BPI_SSID       0x042 
#define SILICOM_PXG2BISC1_SSID   0x003d
#define SILICOM_PEG2TBFI_SSID    0x003E
#define SILICOM_PXG2TBI_SSID     0x003f 
#define SILICOM_PXG4BPFID_SSID   0x0043
#define SILICOM_PEG4BPFI_SSID    0x0040
#define SILICOM_PEG4BPIPT_SSID   0x0044
#define SILICOM_PXG6BPI_SSID     0x0045
#define SILICOM_PEG4BPIL_SSID    0x0046
#define SILICOM_PEG2BPI5_SSID    0x0052
#define SILICOM_PEG6BPI_SSID    0x0053
#define SILICOM_PEG4BPFI5_SSID   0x0050
#define SILICOM_PEG4BPFI5LX_SSID   0x0051
#define SILICOM_PEG2BISC6_SSID 0x54

#define SILICOM_PEG6BPIFC_SSID 0x55



#define SILICOM_PEG2BPFI5_SSID   0x0056
#define SILICOM_PEG2BPFI5LX_SSID   0x0057


#define SILICOM_PXEG4BPFI_SSID    0x0058





#define SILICOM_PEG2BPFID_SSID   0x0047
#define SILICOM_PEG2BPFIDLX_SSID  0x004C
#define SILICOM_MEG2BPFILN_SSID  0x0048
#define SILICOM_MEG2BPFINX_SSID  0x0049
#define SILICOM_PEG4BPFILX_SSID  0x004A
#define SILICOM_MHIO8AD_SSID    0x004F

#define SILICOM_MEG2BPFILXLN_SSID 0x004b
#define SILICOM_PEG2BPIX1_SSID    0x004d
#define SILICOM_MEG2BPFILXNX_SSID 0x004e


#define SILICOM_PE10G2BPISR_SSID  0x0102
#define SILICOM_PE10G2BPILR_SSID  0x0103
#define SILICOM_PE10G2BPICX4_SSID  0x0101

#define SILICOM_XE10G2BPILR_SSID 0x0163
#define SILICOM_XE10G2BPISR_SSID 0x0162
#define SILICOM_XE10G2BPICX4_SSID 0x0161
#define SILICOM_XE10G2BPIT_SSID   0x0160

#define SILICOM_PE10GDBISR_SSID   0x0181
#define SILICOM_PE10GDBILR_SSID   0x0182




#define SILICOM_PE210G2DBi9SR_SSID	    0x0188	
#define SILICOM_PE210G2DBi9SRRB_SSID	0x0188	
#define SILICOM_PE210G2DBi9LR_SSID	    0x0189	
#define SILICOM_PE210G2DBi9LRRB_SSID	0x0189	
#define SILICOM_PE310G4DBi940SR_SSID	0x018C
#define SILICOM_PE310G4DBi940LR_SSID	0x018D
#define SILICOM_PE310G4DBi940T_SSID	0x018e

#define SILICOM_PE310G4DBi9T_SSID       0x18e



	

#define SILICOM_PE310G4BPi9T_SSID       0x130
#define SILICOM_PE310G4BPi9SR_SSID        0x132
#define SILICOM_PE310G4BPi9LR_SSID        0x133

#define SILICOM_PE310G4BPi9SRD_SSID        0x134
#define SILICOM_PE310G4BPi9LRD_SSID        0x135





#define SILICOM_M6E310G4BPi9SR_SSID    	0x0492
#define SILICOM_M6E310G4BPi9LR_SSID    	0x0493




#define NOKIA_XE10G2BPIXR_SVID   0x13B8
#define NOKIA_XE10G2BPIXR_SSID   0x051C


#define INTEL_PEG4BPII_PID   0x10A0
#define INTEL_PEG4BPFII_PID   0x10A1
#define INTEL_PEG4BPII_SSID   0x11A0  
#define INTEL_PEG4BPFII_SSID   0x11A1   

#define INTEL_PEG4BPIIO_SSID   0x10A0   
#define INTEL_PEG4BPIIO_PID   0x105e 


#define BROADCOM_VID         0x14e4
#define BROADCOM_PE10G2_PID  0x164e

#define SILICOM_PE10G2BPTCX4_SSID 0x0141
#define SILICOM_PE10G2BPTSR_SSID  0x0142
#define SILICOM_PE10G2BPTLR_SSID  0x0143
#define SILICOM_PE10G2BPTT_SSID   0x0140


#define SILICOM_PEG4BPI6_SSID     0x0320
#define SILICOM_PEG4BPFI6_SSID    0x0321
#define SILICOM_PEG4BPFI6LX_SSID    0x0322
#define SILICOM_PEG4BPFI6ZX_SSID    0x0323


#define SILICOM_PEG2BPI6_SSID    0x0300
#define SILICOM_PEG2BPFI6_SSID    0x0301
#define SILICOM_PEG2BPFI6LX_SSID    0x0302
#define SILICOM_PEG2BPFI6ZX_SSID    0x0303
#define SILICOM_PEG2BPFI6FLXM_SSID  0x0304

#define SILICOM_PEG2DBI6_SSID    0x0308
#define SILICOM_PEG2DBFI6_SSID    0x0309
#define SILICOM_PEG2DBFI6LX_SSID    0x030A
#define SILICOM_PEG2DBFI6ZX_SSID    0x030B


#define SILICOM_MEG2BPI6_SSID     0x0310
#define SILICOM_XEG2BPI6_SSID    0x0318
#define SILICOM_PEG4BPI6FC_SSID     0x0328
#define SILICOM_PEG4BPFI6FC_SSID    0x0329
#define SILICOM_PEG4BPFI6FCLX_SSID    0x032A
#define SILICOM_PEG4BPFI6FCZX_SSID    0x032B

#define SILICOM_PEG6BPI6_SSID     0x0340
#define SILICOM_PE2G6BPI6_SSID     0x0341

#define SILICOM_PEG2BPI6SC6_SSID     0x0360

#define SILICOM_MEG2BPI6_SSID     0x0310
#define SILICOM_XEG2BPI6_SSID     0x0318
#define SILICOM_MEG4BPI6_SSID     0x0330


#define SILICOM_PE2G4BPi80L_SSID    0x0380

#define SILICOM_M6E2G8BPi80A_SSID    0x0474

#define SILICOM_PE2G4BPi35_SSID    0x03d8

#define SILICOM_PE2G4BPFi80_SSID    0x0381
#define SILICOM_PE2G4BPFi80LX_SSID    0x0382
#define SILICOM_PE2G4BPFi80ZX_SSID    0x0383

#define SILICOM_PE2G4BPi80_SSID    0x0388

#define SILICOM_PE2G2BPi80_SSID    0x0390
#define SILICOM_PE2G2BPFi80_SSID    0x0391
#define SILICOM_PE2G2BPFi80LX_SSID    0x0392
#define SILICOM_PE2G2BPFi80ZX_SSID    0x0393

#define SILICOM_PE2G4BPi35L_SSID    0x03D0
#define SILICOM_PE2G4BPFi35_SSID    0x03D1
#define SILICOM_PE2G4BPFi35CS_SSID    0x0b80
#define SILICOM_PE2G4BPFi35LX_SSID    0x03D2
#define SILICOM_PE2G4BPFi35ZX_SSID    0x03D3

#define SILICOM_M1E2G4BPi35_SSID    0x04D0
#define SILICOM_M1E2G4BPFi35_SSID    0x04D1
#define SILICOM_M1E2G4BPFi35LX_SSID    0x04D2
#define SILICOM_M1E2G4BPFi35ZX_SSID    0x04D3

#define SILICOM_M1E2G4BPi35JP_SSID    0x1800
#define SILICOM_M1E2G4BPi35JP1_SSID    0x1801



#define SILICOM_PE2G2BPi35_SSID    0x03c0
#define SILICOM_PAC1200BPi35_SSID    0x03cc
#define SILICOM_PE2G2BPFi35_SSID    0x03C1
#define SILICOM_PE2G2BPFi35LX_SSID    0x03C2
#define SILICOM_PE2G2BPFi35ZX_SSID    0x03C3



#define SILICOM_PE2G6BPi35_SSID    0x03E0
#define SILICOM_PE2G6BPi35CX_SSID  0x0AA0








#define INTEL_PE210G2SPI9_SSID     0x00C

#define SILICOM_M1EG2BPI6_SSID     0x400

#define SILICOM_M1EG2BPFI6_SSID     0x0401
#define SILICOM_M1EG2BPFI6LX_SSID     0x0402
#define SILICOM_M1EG2BPFI6ZX_SSID     0x0403

#define SILICOM_M1EG4BPI6_SSID     0x0420

#define SILICOM_M1EG4BPFI6_SSID       0x0421
#define SILICOM_M1EG4BPFI6LX_SSID     0x0422
#define SILICOM_M1EG4BPFI6ZX_SSID     0x0423

#define SILICOM_M1EG6BPI6_SSID     0x0440

#define SILICOM_M1E2G4BPi80_SSID     0x0460
#define SILICOM_M1E2G4BPFi80_SSID       0x0461
#define SILICOM_M1E2G4BPFi80LX_SSID     0x0462
#define SILICOM_M1E2G4BPFi80ZX_SSID     0x0463

#define SILICOM_M6E2G8BPi80_SSID        0x0470
#define SILICOM_PE210G2BPi40_SSID       0x01a0
#define SILICOM_M1E210G2BPI40T_SSID       0x0480
#define SILICOM_PE310G4BPi40_SSID       0x01a4

#define PEG540_IF_SERIES(pid) \
          ((pid==SILICOM_PE210G2BPi40_SSID) || \
          (pid==SILICOM_PE310G4BPi40_SSID) || \
          (pid==SILICOM_M1E210G2BPI40T_SSID)) 
          




#define OLD_IF_SERIES(pid) \
        ((pid==SILICOM_PXG2BPFI_SSID)||   \
          (pid==SILICOM_PXG2BPFILX_SSID)) 

#define P2BPFI_IF_SERIES(pid) \
        ((pid==SILICOM_PXG2BPFI_SSID)||   \
          (pid==SILICOM_PXG2BPFILX_SSID)|| \
          (pid==SILICOM_PEG2BPFI_SSID)|| \
          (pid==SILICOM_PEG2BPFID_SSID)|| \
          (pid==SILICOM_PEG2BPFIDLX_SSID)|| \
         (pid==SILICOM_MEG2BPFILN_SSID)|| \
         (pid==SILICOM_MEG2BPFINX_SSID)|| \
         (pid==SILICOM_PEG4BPFILX_SSID)|| \
         (pid==SILICOM_PEG4BPFI_SSID)|| \
         (pid==SILICOM_PXEG4BPFI_SSID)|| \
         (pid==SILICOM_PXG4BPFID_SSID)|| \
         (pid==SILICOM_PEG2TBFI_SSID)|| \
         (pid==SILICOM_PE10G2BPISR_SSID)|| \
         (pid==SILICOM_PE10G2BPILR_SSID)|| \
          (pid==SILICOM_PEG2BPFILX_SSID)|| \
          (pid==SILICOM_PMCXG2BPFI_SSID) || \
	  (pid==SILICOM_MHIO8AD_SSID) || \
      (pid==SILICOM_PEG4BPFI5LX_SSID) || \
      (pid==SILICOM_PEG4BPFI5_SSID) || \
      (pid==SILICOM_PEG4BPFI6FC_SSID) || \
      (pid==SILICOM_PEG4BPFI6FCLX_SSID) || \
      (pid==SILICOM_PEG4BPFI6FCZX_SSID) || \
          (pid==NOKIA_PMCXG2BPFIN_SSID)|| \
          (pid==SILICOM_MEG2BPFILXLN_SSID)|| \
          (pid==SILICOM_MEG2BPFILXNX_SSID)|| \
          (pid==SILICOM_XE10G2BPIT_SSID)|| \
           (pid==SILICOM_XE10G2BPICX4_SSID)|| \
           (pid==SILICOM_XE10G2BPISR_SSID)|| \
           (pid==NOKIA_XE10G2BPIXR_SSID)|| \
           (pid==SILICOM_PE10GDBISR_SSID)|| \
           (pid==SILICOM_PE10GDBILR_SSID)|| \
           (pid==SILICOM_XE10G2BPILR_SSID))


#define INTEL_IF_SERIES(pid) \
        ((pid==INTEL_PEG4BPII_SSID)||   \
        (pid==INTEL_PEG4BPIIO_SSID)|| \
        (pid==INTEL_PEG4BPFII_SSID)) 

#define NOKIA_SERIES(pid) \
        ((pid==NOKIA_PMCXG2BPIN_SSID)||   \
          (pid==NOKIA_PMCXG4BPIN_SSID)|| \
          (pid==SILICOM_PMCX4BPI_SSID)|| \
          (pid==NOKIA_PMCXG2BPFIN_SSID)|| \
          (pid==SILICOM_PMCXG2BPFI_SSID)|| \
           (pid==NOKIA_PMCXG2BPIN2_SSID)|| \
           (pid==NOKIA_PMCXG4BPIN2_SSID)|| \
           (pid==SILICOM_PMCX2BPI_SSID)) 

#define DISCF_IF_SERIES(pid) \
        (pid==SILICOM_PEG2TBFI_SSID) 

#define PEGF_IF_SERIES(pid) \
         ((pid==SILICOM_PEG2BPFI_SSID)||    \
         (pid==SILICOM_PEG2BPFID_SSID)||    \
         (pid==SILICOM_PEG2BPFIDLX_SSID)||    \
         (pid==SILICOM_PEG2BPFILX_SSID)||    \
          (pid==SILICOM_PEG4BPFI_SSID)||    \
          (pid==SILICOM_PXEG4BPFI_SSID)||    \
          (pid==SILICOM_MEG2BPFILN_SSID)|| \
         (pid==SILICOM_MEG2BPFINX_SSID)|| \
         (pid==SILICOM_PEG4BPFILX_SSID)|| \
         (pid==SILICOM_PEG2TBFI_SSID)|| \
         (pid==SILICOM_MEG2BPFILXLN_SSID)|| \
         (pid==SILICOM_MEG2BPFILXNX_SSID))


#define TPL_IF_SERIES(pid) \
        ((pid==SILICOM_PXG2BPFIL_SSID)||   \
          (pid==SILICOM_PXG2BPFILLX_SSID)|| \
          (pid==SILICOM_PXG2TBFI_SSID)|| \
	  (pid==SILICOM_PXG4BPFID_SSID)|| \
         (pid==SILICOM_PXG4BPFI_SSID))


#define BP10G_IF_SERIES(pid) \
          ((pid==SILICOM_PE10G2BPISR_SSID)|| \
          (pid==SILICOM_PE10G2BPICX4_SSID)|| \
          (pid==SILICOM_PE10G2BPILR_SSID)|| \
          (pid==SILICOM_XE10G2BPIT_SSID)|| \
           (pid==SILICOM_XE10G2BPICX4_SSID)|| \
           (pid==SILICOM_XE10G2BPISR_SSID)|| \
           (pid==NOKIA_XE10G2BPIXR_SSID)|| \
           (pid==SILICOM_PE10GDBISR_SSID)|| \
           (pid==SILICOM_PE10GDBILR_SSID)|| \
           (pid==SILICOM_XE10G2BPILR_SSID))

#define BP10GB_IF_SERIES(pid) \
          ((pid==SILICOM_PE10G2BPTCX4_SSID)|| \
          (pid==SILICOM_PE10G2BPTSR_SSID)|| \
          (pid==SILICOM_PE10G2BPTLR_SSID)|| \
          (pid==SILICOM_PE10G2BPTT_SSID))



#define BP10G_CX4_SERIES(pid) \
    (pid==SILICOM_PE10G2BPICX4_SSID)

#define BP10GB_CX4_SERIES(pid) \
    (pid==SILICOM_PE10G2BPTCX4_SSID)
    


    



#define SILICOM_M2EG2BPFI6_SSID       0x0501
#define SILICOM_M2EG2BPFI6LX_SSID     0x0502
#define SILICOM_M2EG2BPFI6ZX_SSID     0x0503
#define SILICOM_M2EG4BPI6_SSID        0x0520

#define SILICOM_M2EG4BPFI6_SSID       0x0521
#define SILICOM_M2EG4BPFI6LX_SSID     0x0522
#define SILICOM_M2EG4BPFI6ZX_SSID     0x0523

#define SILICOM_M2EG6BPI6_SSID     0x0540





#define SILICOM_M1E10G2BPI9CX4_SSID  0x481
#define SILICOM_M1E10G2BPI9SR_SSID   0x482
#define SILICOM_M1E10G2BPI9LR_SSID   0x483
//#define SILICOM_M1E10G2BPI9T_SSID    0x480

#define SILICOM_M2E10G2BPI9CX4_SSID  0x581
#define SILICOM_M2E10G2BPI9SR_SSID   0x582
#define SILICOM_M2E10G2BPI9LR_SSID   0x583
#define SILICOM_M2E10G2BPI9T_SSID    0x580


#define SILICOM_PE210G2BPI9CX4_SSID  0x121
#define SILICOM_PE210G2BPI9SR_SSID   0x122
#define SILICOM_PE210G2BPI9LR_SSID   0x123
#define SILICOM_PE210G2BPI9LRD_SSID   0x125
#define SILICOM_PE210G2BPI9SRD_SSID   0x124
#define SILICOM_PE210G2BPI9T_SSID    0x120

#define SILICOM_M1E210G2BPI9SRDJP_SSID 0x1E00
#define SILICOM_M1E210G2BPI9SRDJP1_SSID 0x1E10
#define SILICOM_M1E210G2BPI9LRDJP_SSID 0x1F00
#define SILICOM_M1E210G2BPI9LRDJP1_SSID 0x1F10
  




#define DBI_IF_SERIES(pid) \
((pid==SILICOM_PE10GDBISR_SSID)|| \
           (pid==SILICOM_PE10GDBILR_SSID)|| \
           (pid==SILICOM_XE10G2BPILR_SSID)|| \
           (pid==SILICOM_PE210G2DBi9LR_SSID))


#define PEGF5_IF_SERIES(pid) \
((pid==SILICOM_PEG2BPFI5_SSID)|| \
          (pid==SILICOM_PEG2BPFI5LX_SSID)|| \
          (pid==SILICOM_PEG4BPFI6_SSID)|| \
          (pid==SILICOM_PE2G6BPI6_SSID)|| \
          (pid==SILICOM_PEG4BPFI6LX_SSID)|| \
           (pid==SILICOM_PEG4BPFI6ZX_SSID)|| \
           (pid==SILICOM_PEG2BPFI6_SSID)|| \
           (pid==SILICOM_PEG2BPFI6LX_SSID)|| \
           (pid==SILICOM_PEG2BPFI6ZX_SSID)|| \
           (pid==SILICOM_PEG2BPFI6FLXM_SSID)|| \
           (pid==SILICOM_PEG2DBFI6_SSID)|| \
           (pid==SILICOM_PEG2DBFI6LX_SSID)|| \
           (pid==SILICOM_PEG2DBFI6ZX_SSID)|| \
           (pid==SILICOM_PEG4BPI6FC_SSID)|| \
           (pid==SILICOM_PEG4BPFI6FCLX_SSID)|| \
           (pid==SILICOM_PEG4BPI6FC_SSID)|| \
           (pid==SILICOM_M1EG2BPFI6_SSID)|| \
           (pid==SILICOM_M1EG2BPFI6LX_SSID)|| \
           (pid==SILICOM_M1EG2BPFI6ZX_SSID)|| \
           (pid==SILICOM_M1EG4BPFI6_SSID)|| \
           (pid==SILICOM_M1EG4BPFI6LX_SSID)|| \
           (pid==SILICOM_M1EG4BPFI6ZX_SSID)|| \
           (pid==SILICOM_M2EG2BPFI6_SSID)|| \
           (pid==SILICOM_M2EG2BPFI6LX_SSID)|| \
           (pid==SILICOM_M2EG2BPFI6ZX_SSID)|| \
           (pid==SILICOM_M2EG4BPFI6_SSID)|| \
           (pid==SILICOM_M2EG4BPFI6LX_SSID)|| \
           (pid==SILICOM_M2EG4BPFI6ZX_SSID)|| \
           (pid==SILICOM_PEG4BPFI6FCZX_SSID))
           



#define PEG5_IF_SERIES(pid) \
((pid==SILICOM_PEG4BPI6_SSID)|| \
(pid==SILICOM_PEG2BPI6_SSID)|| \
(pid==SILICOM_PEG2BPI6SC6_SSID)|| \
(pid==SILICOM_PEG4BPI6FC_SSID)|| \
(pid==SILICOM_PEG6BPI6_SSID)|| \
(pid==SILICOM_MEG2BPI6_SSID)|| \
(pid==SILICOM_XEG2BPI6_SSID)|| \
(pid==SILICOM_MEG4BPI6_SSID)|| \
(pid==SILICOM_M1EG2BPI6_SSID)|| \
(pid==SILICOM_M1EG4BPI6_SSID)|| \
(pid==SILICOM_M1EG6BPI6_SSID)|| \
(pid==SILICOM_PEG6BPI_SSID)|| \
(pid==SILICOM_PEG4BPIL_SSID)|| \
(pid==SILICOM_PEG2BISC6_SSID)|| \
(pid==SILICOM_PEG2BPI5_SSID))
 

#define PEG80_IF_SERIES(pid) \
((pid==SILICOM_M1E2G4BPi80_SSID)|| \
(pid==SILICOM_M6E2G8BPi80_SSID)|| \
(pid==SILICOM_PE2G4BPi80L_SSID)|| \
(pid==SILICOM_M6E2G8BPi80A_SSID)|| \
(pid==SILICOM_PE2G2BPi35_SSID)|| \
(pid==SILICOM_PAC1200BPi35_SSID)|| \
(pid==SILICOM_PE2G4BPi35_SSID)|| \
(pid==SILICOM_PE2G4BPi35L_SSID)|| \
(pid==SILICOM_M1E2G4BPi35_SSID)|| \
(pid==SILICOM_M1E2G4BPi35JP_SSID)|| \
(pid==SILICOM_M1E2G4BPi35JP1_SSID)|| \
(pid==SILICOM_PE2G6BPi35_SSID)|| \
(pid==SILICOM_PE2G2BPi80_SSID)|| \
(pid==SILICOM_PE2G4BPi80_SSID)|| \
(pid==SILICOM_PE2G4BPFi80_SSID)|| \
(pid==SILICOM_PE2G4BPFi80LX_SSID)|| \
(pid==SILICOM_PE2G4BPFi80ZX_SSID)|| \
(pid==SILICOM_PE2G4BPFi80ZX_SSID)|| \
(pid==SILICOM_PE2G2BPFi80_SSID)|| \
(pid==SILICOM_PE2G2BPFi80LX_SSID)|| \
(pid==SILICOM_PE2G2BPFi80ZX_SSID)|| \
(pid==SILICOM_PE2G2BPFi35_SSID)|| \
(pid==SILICOM_PE2G2BPFi35LX_SSID)|| \
(pid==SILICOM_PE2G2BPFi35ZX_SSID)|| \
(pid==SILICOM_M1E2G4BPFi35_SSID)|| \
(pid==SILICOM_M1E2G4BPFi35LX_SSID)|| \
(pid==SILICOM_M1E2G4BPFi35ZX_SSID)|| \
(pid==SILICOM_PE2G4BPFi35_SSID)|| \
(pid==SILICOM_PE2G4BPFi35CS_SSID)|| \
(pid==SILICOM_PE2G4BPFi35LX_SSID)|| \
(pid==SILICOM_PE2G4BPFi35ZX_SSID))


#define PEGF80_IF_SERIES(pid) \
((pid==SILICOM_PE2G4BPFi80_SSID)|| \
(pid==SILICOM_PE2G4BPFi80LX_SSID)|| \
(pid==SILICOM_PE2G4BPFi80ZX_SSID)|| \
(pid==SILICOM_PE2G4BPFi80ZX_SSID)|| \
(pid==SILICOM_M1E2G4BPFi80_SSID)|| \
(pid==SILICOM_M1E2G4BPFi80LX_SSID)|| \
(pid==SILICOM_M1E2G4BPFi80ZX_SSID)|| \
(pid==SILICOM_PE2G2BPFi80_SSID)|| \
(pid==SILICOM_PE2G2BPFi80LX_SSID)|| \
(pid==SILICOM_PE2G2BPFi80ZX_SSID)|| \
(pid==SILICOM_PE2G2BPFi35_SSID)|| \
(pid==SILICOM_PE2G2BPFi35LX_SSID)|| \
(pid==SILICOM_PE2G2BPFi35ZX_SSID)|| \
(pid==SILICOM_M1E2G4BPFi35_SSID)|| \
(pid==SILICOM_M1E2G4BPFi35LX_SSID)|| \
(pid==SILICOM_M1E2G4BPFi35ZX_SSID)|| \
(pid==SILICOM_PE2G4BPFi35_SSID)|| \
(pid==SILICOM_PE2G4BPFi35CS_SSID)|| \
(pid==SILICOM_PE2G4BPFi35LX_SSID)|| \
(pid==SILICOM_PE2G4BPFi35ZX_SSID))


#define BP10G9_IF_SERIES(pid) \
((pid==INTEL_PE210G2SPI9_SSID)|| \
(pid==SILICOM_M1E10G2BPI9CX4_SSID)|| \
(pid==SILICOM_M1E10G2BPI9SR_SSID)|| \
(pid==SILICOM_M1E10G2BPI9LR_SSID)|| \
(pid==SILICOM_M2E10G2BPI9CX4_SSID)|| \
(pid==SILICOM_M2E10G2BPI9SR_SSID)|| \
(pid==SILICOM_M2E10G2BPI9LR_SSID)|| \
(pid==SILICOM_M2E10G2BPI9T_SSID)|| \
(pid==SILICOM_PE210G2BPI9CX4_SSID)|| \
(pid==SILICOM_PE210G2BPI9SR_SSID)|| \
(pid==SILICOM_PE210G2BPI9LR_SSID)|| \
(pid==SILICOM_PE210G2BPI9LRD_SSID)|| \
(pid==SILICOM_PE210G2BPI9SRD_SSID)|| \
(pid==SILICOM_PE210G2DBi9SR_SSID)|| \
(pid==SILICOM_PE210G2DBi9SRRB_SSID)|| \
(pid==SILICOM_PE210G2DBi9LR_SSID)|| \
(pid==SILICOM_PE210G2DBi9LRRB_SSID)|| \
(pid==SILICOM_PE310G4DBi940SR_SSID)|| \
(pid==SILICOM_PE310G4DBi940LR_SSID)|| \
(pid==SILICOM_PE310G4DBi940T_SSID)|| \
(pid==SILICOM_PE310G4DBi9T_SSID)|| \
(pid==SILICOM_PE310G4BPi9T_SSID)|| \
(pid==SILICOM_PE310G4BPi9SR_SSID)|| \
(pid==SILICOM_PE310G4BPi9SRD_SSID)|| \
(pid==SILICOM_M6E310G4BPi9SR_SSID)|| \
(pid==SILICOM_M6E310G4BPi9LR_SSID)|| \
(pid==SILICOM_PE310G4BPi9LR_SSID)|| \
(pid==SILICOM_PE310G4BPi9LRD_SSID)|| \
(pid==SILICOM_M1E210G2BPI9SRDJP_SSID)|| \
(pid==SILICOM_M1E210G2BPI9SRDJP1_SSID)|| \
(pid==SILICOM_M1E210G2BPI9LRDJP_SSID)|| \
(pid==SILICOM_M1E210G2BPI9LRDJP1_SSID)|| \
(pid==SILICOM_PE210G2BPI9T_SSID))

#define BP71_IF_SERIES(pid) \
((pid==SILICOM_PEG2BPI_SSID)|| \
(pid==SILICOM_PEG2BPIX1_SSID)|| \
(pid==SILICOM_PEG4BPIN_SSID)||   \
(pid==SILICOM_PEG2BPFILX_SSID)|| \
(pid==SILICOM_PEG4BPFI_SSID)||  \
(pid==SILICOM_PEG2TBFI_SSID)|| \
(pid==SILICOM_PEG4BPIPT_SSID)|| \
(pid==SILICOM_PEG2BPFID_SSID)|| \
(pid==SILICOM_PEG2BPFIDLX_SSID)|| \
(pid==SILICOM_MEG2BPFILN_SSID)|| \
(pid==SILICOM_MEG2BPFINX_SSID)|| \
(pid==SILICOM_PEG4BPFILX_SSID)|| \
(pid==SILICOM_MEG2BPFILXLN_SSID)||\
(pid==SILICOM_PXEG4BPFI_SSID))

           


/*******************************************************/
/* 1G INTERFACE ****************************************/
/*******************************************************/

/* Intel Registers */
#define BPCTLI_CTRL          0x00000  
#define BPCTLI_LEDCTL         0x00E00
#define BPCTLI_CTRL_SWDPIO0  0x00400000 
#define BPCTLI_CTRL_SWDPIN0  0x00040000


#define BPCTLI_CTRL_EXT 0x00018  /* Extended Device Control - RW */
#define BPCTLI_STATUS   0x00008  /* Device Status - RO */

#define BPCTLI_EERD     0x14 
#define BPCTLI_EEWR     0x102c 

/* HW related */
#define BPCTLI_CTRL_EXT_SDP6_DATA 0x00000040 /* Value of SW Defineable Pin 6 */
#define BPCTLI_CTRL_EXT_SDP7_DATA 0x00000080 /* Value of SW Defineable Pin 7 */
#define BPCTLI_CTRL_SDP0_DATA     0x00040000  /* SWDPIN 0 value */
#define BPCTLI_CTRL_EXT_SDP6_DIR  0x00000400 /* Direction of SDP6 0=in 1=out */
#define BPCTLI_CTRL_EXT_SDP7_DIR  0x00000800 /* Direction of SDP7 0=in 1=out */
#define BPCTLI_CTRL_SDP0_DIR      0x00400000  /* SDP0 Input or output */
#define BPCTLI_CTRL_SWDPIN0       0x00040000
#define BPCTLI_CTRL_SWDPIN1       0x00080000
#define BPCTLI_CTRL_SDP1_DIR      0x00800000

#define BPCTLI_STATUS_LU          0x00000002      /* Link up.0=no,1=link */


#define BPCTLI_CTRL_SDP0_SHIFT     18
#define BPCTLI_CTRL_EXT_SDP6_SHIFT 6

#define BPCTLI_STATUS_TBIMODE     0x00000020
#define BPCTLI_CTRL_EXT_LINK_MODE_PCIE_SERDES  0x00C00000
#define BPCTLI_CTRL_EXT_LINK_MODE_MASK         0x00C00000

#define BPCTLI_LEDCTL_MODE_LED_ON        0xE
#define BPCTLI_LEDCTL_MODE_LED_OFF       0xF

#define BPCTLI_LEDCTL_LED0_MODE_MASK       0x0000000F
#define BPCTLI_LEDCTL_LED0_MODE_SHIFT      0
#define BPCTLI_LEDCTL_LED0_BLINK_RATE      0x0000020
#define BPCTLI_LEDCTL_LED0_IVRT            0x00000040
#define BPCTLI_LEDCTL_LED0_BLINK           0x00000080


#define BPCTLI_CTRL_EXT_MCLK_DIR  BPCTLI_CTRL_EXT_SDP7_DIR
#define BPCTLI_CTRL_EXT_MCLK_DATA BPCTLI_CTRL_EXT_SDP7_DATA
#define BPCTLI_CTRL_EXT_MDIO_DIR  BPCTLI_CTRL_EXT_SDP6_DIR
#define BPCTLI_CTRL_EXT_MDIO_DATA BPCTLI_CTRL_EXT_SDP6_DATA

#define BPCTLI_CTRL_EXT_MCLK_DIR5  BPCTLI_CTRL_SDP1_DIR
#define BPCTLI_CTRL_EXT_MCLK_DATA5 BPCTLI_CTRL_SWDPIN1
#define BPCTLI_CTRL_EXT_MCLK_DIR80  BPCTLI_CTRL_EXT_SDP6_DIR
#define BPCTLI_CTRL_EXT_MCLK_DATA80 BPCTLI_CTRL_EXT_SDP6_DATA
#define BPCTLI_CTRL_EXT_MDIO_DIR5  BPCTLI_CTRL_SWDPIO0
#define BPCTLI_CTRL_EXT_MDIO_DATA5 BPCTLI_CTRL_SWDPIN0
#define BPCTLI_CTRL_EXT_MDIO_DIR80  BPCTLI_CTRL_SWDPIO0
#define BPCTLI_CTRL_EXT_MDIO_DATA80 BPCTLI_CTRL_SWDPIN0



#define BPCTL_WRITE_REG(a, reg, value) \
	(writel((value), (void *)(((a)->mem_map) + BPCTLI_##reg)))

#define BPCTL_READ_REG(a, reg) ( \
        readl((void *)((a)->mem_map) + BPCTLI_##reg))


#define BPCTL_WRITE_FLUSH(a) BPCTL_READ_REG(a, STATUS)

#define BPCTL_BP_WRITE_REG(a, reg, value) ({ \
        BPCTL_WRITE_REG(a, reg, value); \
        BPCTL_WRITE_FLUSH(a);})
        
/**************************************************************/
/************** 82575 Interface********************************/
/**************************************************************/

#define BPCTLI_MII_CR_POWER_DOWN       0x0800
#define BPCTLI_PHY_CONTROL      0x00 /* Control Register */ 
#define BPCTLI_MDIC     0x00020  /* MDI Control - RW */ 
#define BPCTLI_IGP01E1000_PHY_PAGE_SELECT        0x1F /* Page Select */
#define BPCTLI_MAX_PHY_REG_ADDRESS    0x1F  /* 5 bit address bus (0-0x1F) */  

#define BPCTLI_MDIC_DATA_MASK 0x0000FFFF
#define BPCTLI_MDIC_REG_MASK  0x001F0000
#define BPCTLI_MDIC_REG_SHIFT 16
#define BPCTLI_MDIC_PHY_MASK  0x03E00000
#define BPCTLI_MDIC_PHY_SHIFT 21
#define BPCTLI_MDIC_OP_WRITE  0x04000000
#define BPCTLI_MDIC_OP_READ   0x08000000
#define BPCTLI_MDIC_READY     0x10000000
#define BPCTLI_MDIC_INT_EN    0x20000000
#define BPCTLI_MDIC_ERROR     0x40000000


#define BPCTLI_SWFW_PHY0_SM  0x02
#define BPCTLI_SWFW_PHY1_SM  0x04 

#define BPCTLI_SW_FW_SYNC  0x05B5C /* Software-Firmware Synchronization - RW */

#define BPCTLI_SWSM      0x05B50 /* SW Semaphore */
#define BPCTLI_FWSM      0x05B54 /* FW Semaphore */
 
#define BPCTLI_SWSM_SMBI         0x00000001 /* Driver Semaphore bit */ 
#define BPCTLI_SWSM_SWESMBI      0x00000002 /* FW Semaphore bit */ 
#define BPCTLI_MAX_PHY_MULTI_PAGE_REG 0xF 
#define BPCTLI_GEN_POLL_TIMEOUT          640 

/********************************************************/




/********************************************************/
/* 10G INTERFACE ****************************************/
/********************************************************/

#define BP10G_I2CCTL              0x28

/* I2CCTL Bit Masks */
#define BP10G_I2C_CLK_IN    0x00000001
#define BP10G_I2C_CLK_OUT   0x00000002
#define BP10G_I2C_DATA_IN   0x00000004
#define BP10G_I2C_DATA_OUT  0x00000008


#define BP10G_ESDP                0x20
#define BP10G_DCA_ID              0x11070

#define BP10G_SDP0_DIR            0x100
#define BP10G_SDP1_DIR            0x200 
#define BP10G_SDP3_DIR            0x800
#define BP10G_SDP4_DIR            BIT_12  
#define BP10G_SDP5_DIR            0x2000 
#define BP10G_SDP0_DATA           0x001
#define BP10G_SDP1_DATA           0x002
#define BP10G_SDP3_DATA           0x008
#define BP10G_SDP4_DATA           0x010
#define BP10G_SDP5_DATA           0x020

#define BP10G_SDP2_DIR            0x400
#define BP10G_SDP2_DATA            0x4


#define BP10G_EODSDP              0x28

#define BP10G_SDP6_DATA_IN        0x001
#define BP10G_SDP6_DATA_OUT       0x002 

#define BP10G_SDP7_DATA_IN        0x004
#define BP10G_SDP7_DATA_OUT       0x008 

#define BP10G_MCLK_DATA_OUT       BP10G_SDP7_DATA_OUT
#define BP10G_MDIO_DATA_OUT       BP10G_SDP6_DATA_OUT
#define BP10G_MDIO_DATA_IN        BP10G_SDP6_DATA_IN

#define BP10G_MDIO_DATA           /*BP10G_SDP5_DATA*/ BP10G_SDP3_DATA
#define BP10G_MDIO_DIR            /*BP10G_SDP5_DIR*/  BP10G_SDP3_DATA

/*#define BP10G_MCLK_DATA_OUT9       BP10G_I2C_CLK_OUT
#define BP10G_MDIO_DATA_OUT9       BP10G_I2C_DATA_OUT*/


/*#define BP10G_MCLK_DATA_OUT9*/       /*BP10G_I2C_DATA_OUT*/
#define BP10G_MDIO_DATA_OUT9           BP10G_I2C_DATA_OUT /*BP10G_I2C_CLK_OUT*/

/* VIA EOSDP ! */
#define BP10G_MCLK_DATA_OUT9           BP10G_SDP4_DATA
#define BP10G_MCLK_DIR_OUT9            BP10G_SDP4_DIR


/*#define BP10G_MDIO_DATA_IN9        BP10G_I2C_DATA_IN*/

#define BP10G_MDIO_DATA_IN9           BP10G_I2C_DATA_IN /*BP10G_I2C_CLK_IN*/

#define BP10G_LEDCTL                  0x00200

/* LEDCTL Bit Masks */
#define BP10G_LED_IVRT_BASE      0x00000040
#define BP10G_LED_BLINK_BASE     0x00000080
#define BP10G_LED_MODE_MASK_BASE 0x0000000F
#define BP10G_LED_OFFSET(_base, _i) (_base << (8 * (_i)))
#define BP10G_LED_MODE_SHIFT(_i) (8*(_i))
#define BP10G_LED_IVRT(_i)       BP10G_LED_OFFSET(BP10G_LED_IVRT_BASE, _i)
#define BP10G_LED_BLINK(_i)      BP10G_LED_OFFSET(BP10G_LED_BLINK_BASE, _i)
#define BP10G_LED_MODE_MASK(_i)  BP10G_LED_OFFSET(BP10G_LED_MODE_MASK_BASE, _i)

/* LED modes */
#define BP10G_LED_LINK_UP       0x0
#define BP10G_LED_LINK_10G      0x1
#define BP10G_LED_MAC           0x2
#define BP10G_LED_FILTER        0x3
#define BP10G_LED_LINK_ACTIVE   0x4
#define BP10G_LED_LINK_1G       0x5
#define BP10G_LED_ON            0xE
#define BP10G_LED_OFF           0xF
 
#define BP10G_STATUS    0x00008
#define BP540_MDIO_DATA           /*BP10G_SDP5_DATA*/ BP10G_SDP0_DATA
#define BP540_MDIO_DIR            /*BP10G_SDP5_DIR*/  BP10G_SDP0_DIR
#define BP540_MCLK_DATA       BP10G_SDP2_DATA
#define BP540_MCLK_DIR       BP10G_SDP2_DIR



#define BP10G_WRITE_REG(a, reg, value) \
	(writel((value), (void *)(((a)->mem_map) + BP10G_##reg)))

#define BP10G_READ_REG(a, reg) ( \
        readl((void *)((a)->mem_map) + BP10G_##reg)) 

/*****BROADCOM*******************************************/

#define BP10GB_MISC_REG_GPIO						 0xa490
#define BP10GB_GPIO3_P0                              BIT_3
#define BP10GB_GPIO3_P1                              BIT_7

#define BP10GB_GPIO3_SET_P0                          BIT_11
#define BP10GB_GPIO3_CLR_P0                          BIT_19
#define BP10GB_GPIO3_OE_P0                           BIT_27

#define BP10GB_GPIO3_SET_P1                          BIT_15
#define BP10GB_GPIO3_CLR_P1                          BIT_23
#define BP10GB_GPIO3_OE_P1                           BIT_31

#define BP10GB_GPIO0_P1                              0x10
#define BP10GB_GPIO0_P0                              0x1
#define BP10GB_GPIO0_CLR_P0                          0x10000
#define BP10GB_GPIO0_CLR_P1                          0x100000
#define BP10GB_GPIO0_SET_P0                          0x100
#define BP10GB_GPIO0_SET_P1                          0x1000



#define BP10GB_GPIO0_OE_P1                           0x10000000
#define BP10GB_GPIO0_OE_P0                           0x1000000

#define BP10GB_MISC_REG_SPIO						 0xa4fc
#define BP10GB_GPIO4_OE                              BIT_28
#define BP10GB_GPIO5_OE                              BIT_29
#define BP10GB_GPIO4_CLR                             BIT_20
#define BP10GB_GPIO5_CLR                             BIT_21
#define BP10GB_GPIO4_SET                             BIT_12
#define BP10GB_GPIO5_SET                             BIT_13
#define BP10GB_GPIO4                                 BIT_4
#define BP10GB_GPIO5                                 BIT_5



#define BP10GB_MCLK_DIR  BP10GB_GPIO5_OE
#define BP10GB_MDIO_DIR  BP10GB_GPIO4_OE

#define BP10GB_MCLK_DATA BP10GB_GPIO5
#define BP10GB_MDIO_DATA BP10GB_GPIO4 

#define BP10GB_MCLK_SET BP10GB_GPIO5_SET
#define BP10GB_MDIO_SET BP10GB_GPIO4_SET

#define BP10GB_MCLK_CLR BP10GB_GPIO5_CLR
#define BP10GB_MDIO_CLR BP10GB_GPIO4_CLR 


/* toggle LED 4 times per second = 2 "blinks" per second */
#define BPVM_ID_INTERVAL    (HZ/4)

/* bit defines for adapter->led_status */
#define BPVM_LED_ON		0

/*#define BPCTLI_LEDCTL_MODE_LED_ON        0xE
#define BPCTLI_LEDCTL_MODE_LED_OFF       0xF*/

#define BPCTLI_EEPROM_ID_LED_SETTINGS    0x0004 
#define ID_LED_RESERVED_0000 0x0000
#define ID_LED_RESERVED_FFFF 0xFFFF
#define ID_LED_DEFAULT       ((ID_LED_OFF1_ON2 << 12) | \
                              (ID_LED_OFF1_OFF2 << 8) | \
                              (ID_LED_DEF1_DEF2 << 4) | \
                              (ID_LED_DEF1_DEF2))
#define ID_LED_DEFAULT_ICH8LAN  ((ID_LED_DEF1_DEF2 << 12) | \
                                 (ID_LED_DEF1_OFF2 <<  8) | \
                                 (ID_LED_DEF1_ON2  <<  4) | \
                                 (ID_LED_DEF1_DEF2))
#define ID_LED_DEF1_DEF2     0x1
#define ID_LED_DEF1_ON2      0x2
#define ID_LED_DEF1_OFF2     0x3
#define ID_LED_ON1_DEF2      0x4
#define ID_LED_ON1_ON2       0x5
#define ID_LED_ON1_OFF2      0x6
#define ID_LED_OFF1_DEF2     0x7
#define ID_LED_OFF1_ON2      0x8
#define ID_LED_OFF1_OFF2     0x9

#define BPCTL_SUCCESS           0
#define BPCTL_NVM_POLL_READ     0
#define BPCTL_NVM_RW_ADDR_SHIFT 2
#define BPCTL_ERR_NVM           1
#define BPCTL_NVM_RW_REG_DATA   16
#define BPCTL_NVM_RW_REG_DONE   2

#define BPCTL_NVM_RW_REG_START  1 




#define BP10GB_WRITE_REG(a, reg, value) \
	(writel((value), (void *)(((a)->mem_map) + BP10GB_##reg)))

#define BP10GB_READ_REG(a, reg) ( \
        readl((void *)((a)->mem_map) + BP10GB_##reg))

#define BP10G_WRITE_FLUSH(a) BP10G_READ_REG(a, STATUS)



#endif
