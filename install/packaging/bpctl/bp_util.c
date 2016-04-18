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

#include <fcntl.h> /* open */
#include <unistd.h> /* exit */
#include <sys/ioctl.h> /* ioctl */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <net/if.h>
#include "bp_ioctl.h" 
#include "bp_util.h" 

static void bp_usage(void){
    printf("Usage: "PROG_NAME" <if_index|bus:slot.function> <command> [parameters]\n");
    printf("       "PROG_NAME" <info|help>\n");
    printf("<if_index>   - interface name, for example, eth0, or all for all Bypass-SD/TAP-SD Control devices\n");
    printf("<command>    - bypass control command (see Commands List).\n");
    printf("[parameters] - set_bypass_wd command:\n"); 
    printf("                   WDT timeout interval, msec (0 for disabling WDT).\n");
    printf("               set_bypass/set_bypass_pwoff/set_bypass_pwup/set_dis_bypass commands:\n");
    printf("                   on/off for enable/disable Bypass\n");
    printf("               set_std_nic command:\n");
    printf("                   on/off for enable/disable Standard NIC mode\n");
    printf("               set_tx command:\n");
    printf("                   on/off for enable/disable transmit\n");
    printf("               set_tpl command:\n");
    printf("                   on/off for enable/disable TPL\n");
#ifdef PMC_FIX_FLAG
    printf("               set_wait_at_pwup command:\n");
    printf("                   on/off for enable/disable wait_at_pwup\n");
#endif
#ifdef BP_DBI_FLAG
    printf("               set_force_link_on command:\n");
    printf("                   on/off for enable/disable force_link_on\n");
#endif
    printf("               set_hw_reset command:\n");
    printf("                   on/off for enable/disable hw_reset\n");
    printf("               set_tap/set_tap_pwup/set_dis_tap commands:\n");
    printf("                   on/off for enable/disable TAP\n");
    printf("               set_disc/set_disc_pwup/set_dis_disc commands:\n");
    printf("                   on/off for enable/disable Disc\n");
    printf("               set_wd_exp_mode command:\n");
    printf("                   bypass/tap/disc for bypass/tap/disc mode\n");
    printf("               set_wd_autoreset command:\n");
    printf("                   WDT autoreset interval, msec (0 for disabling WDT autoreset).\n");
    printf("if_scan      - refresh the list of network interfaces..\n");

    printf("info         - print Program Information.\n");
    printf("help         - print this message.\n");
    printf("   Commands List:\n");
    printf("is_bypass        - check if device is a Bypass/TAP controlling device\n");
    printf("get_bypass_slave - get the second port participate in the Bypass/TAP pair\n");
    printf("get_bypass_caps  - obtain Bypass/TAP capabilities information\n");
    printf("get_wd_set_caps  - obtain watchdog timer settings capabilities\n");
    printf("get_bypass_info  - get bypass/TAP info\n");
    printf("set_bypass       - set Bypass mode state\n");
    printf("get_bypass       - get Bypass mode state\n");
    printf("get_bypass_change - get change of Bypass mode state from last status check\n");
    printf("set_dis_bypass   - set Disable Bypass mode\n");
    printf("get_dis_bypass   - get Disable Bypass mode state\n");
    printf("set_bypass_pwoff - set Bypass mode at power-off state\n");
    printf("get_bypass_pwoff - get Bypass mode at power-off state\n");
    printf("set_bypass_pwup  - set Bypass mode at power-up state\n");
    printf("get_bypass_pwup  - get Bypass mode at power-up state\n");
    printf("set_std_nic      - set Standard NIC mode of operation\n"); 
    printf("get_std_nic      - get Standard NIC mode settings\n");
    printf("set_bypass_wd    - set watchdog state\n");
    printf("get_bypass_wd    - get watchdog state\n");
    printf("get_wd_time_expire - get watchdog expired time\n");
    printf("reset_bypass_wd - reset watchdog timer\n");
    printf("set_tx      - set transmit enable / disable\n"); 
    printf("get_tx      - get transmitter state (enabled / disabled)\n");
    printf("set_tpl      - set TPL enable / disable\n"); 
    printf("get_tpl      - get TPL state (enabled / disabled)\n");
#ifdef PMC_FIX_FLAG
    printf("set_wait_at_pwup      - set wait_at_pwup enable / disable\n"); 
    printf("get_wait_at_pwup      - get wait_at_pwup (enabled / disabled)\n");
#endif
#ifdef BP_DBI_FLAG
    printf("set_force_link_on     - set force_link_on enable / disable\n"); 
    printf("get_force_link_on     - get force_link_on (enabled / disabled)\n");    
#endif
    printf("set_hw_reset          - set hw_reset enable / disable\n"); 
    printf("get_hw_reset          - get hw_reset (enabled / disabled)\n");
    printf("set_tap       - set TAP mode state\n");
    printf("get_tap       - get TAP mode state\n");
    printf("get_tap_change - get change of TAP mode state from last status check\n");
    printf("set_dis_tap   - set Disable TAP mode\n");
    printf("get_dis_tap   - get Disable TAP mode state\n");
    printf("set_tap_pwup  - set TAP mode at power-up state\n");
    printf("get_tap_pwup  - get TAP mode at power-up state\n");
    printf("set_disc       - set Disc mode state\n");
    printf("get_disc       - get Disc mode state\n");
    printf("get_disc_change - get change of Disc mode state from last status check\n");
    printf("set_dis_disc   - set Disable Disc mode\n");
    printf("get_dis_disc   - get Disable Disc mode state\n");
    printf("set_disc_pwup  - set Disc mode at power-up state\n");
    printf("get_disc_pwup  - get Disc mode at power-up state\n");
#if 0
    printf("get_disc_port  - get Disc Port mode state\n");
    printf("set_disc_port_pwup  - set Disc Port mode at power-up state\n");
    printf("get_disc_port_pwup  - get Disc Port mode at power-up state\n"); 
#endif

    printf("set_wd_exp_mode - set adapter state when WDT expired\n");
    printf("get_wd_exp_mode - get adapter state when WDT expired\n");
    printf("set_wd_autoreset - set WDT autoreset mode\n");
    printf("get_wd_autoreset - get WDT autoreset mode\n");
#ifdef BP_SELF_TEST
    printf("set_bst - set Bypass Self Test mode\n");
    printf("get_bst - get Bypass Self Test mode\n");

#endif


    printf("\nExample: "PROG_NAME" eth0 set_bypass_wd 5000\n");
    printf("         "PROG_NAME" all set_bypass on\n");
    printf("         "PROG_NAME" eth0 set_wd_exp_mode tap\n");
    printf("         "PROG_NAME" 0b:00.0 get_bypass_info\n");


}


#define BPCTL_GET_CMD(fn,file_desc,if_index,dev_num, bus, slot, func) \
   ({if((bus)||(slot)||(func)||(if_index)) \
   get_##fn##_cmd(file_desc,if_index, bus, slot, func); \
   else \
           get_##fn##_all_cmd(file_desc,dev_num);})

#define BPCTL_SET_CMD(fn,file_desc,if_index,dev_num,param, bus, slot, func) \
   ({if((bus)||(slot)||(func)||(if_index)) \
           set_##fn##_cmd(file_desc,if_index,param, bus, slot, func); \
    else \
        set_##fn##_all_cmd(file_desc,dev_num,param);})

#define IS_BYPASS_CMD(file_desc,if_index,dev_num, bus, slot, func) \
   ({if((bus)||(slot)||(func)||(if_index)) \
           is_bypass_cmd(file_desc,if_index, bus, slot, func); \
    else \
        is_bypass_all_cmd(file_desc,dev_num);})

#define RESET_WD_CMD(file_desc,if_index,dev_num, bus, slot, func) \
   ({if((bus)||(slot)||(func)||(if_index)) \
           reset_bypass_wd_cmd(file_desc,if_index, bus, slot, func); \
    else \
        reset_bypass_wd_all_cmd(file_desc,dev_num);})




int if_info_msg (int file_desc, struct bpctl_cmd *bpctl_cmd){
    char if_name[IFNAMSIZ];
	int ret_val = 0;
    struct bpctl_cmd bpctl_cmd1;

    memset(&bpctl_cmd1, 0, sizeof(bpctl_cmd1));
    bpctl_cmd1.in_param[1] = bpctl_cmd->out_param[3];
    bpctl_cmd1.in_param[5] = bpctl_cmd->out_param[0];
    bpctl_cmd1.in_param[6] = bpctl_cmd->out_param[1];
    bpctl_cmd1.in_param[7] = bpctl_cmd->out_param[2];



    printf("%02x:%02x.%x ",bpctl_cmd->out_param[0],bpctl_cmd->out_param[1],bpctl_cmd->out_param[2]);
    if ((bpctl_cmd->out_param[3]>0)&&(if_indextoname(bpctl_cmd->out_param[3], (char *)&if_name)!=NULL))
        printf("%s ",if_name);
    else printf("     ");

	if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(IS_BYPASS), &bpctl_cmd1))<0) {
        return 0;
    }

    if (bpctl_cmd1.status < 0) {
        return 0;
    }
   /* if ((bpctl_cmd->out_param[2]==1)||(bpctl_cmd->out_param[2]==3)) { */
    if (bpctl_cmd1.status == 0) {
        printf("slave\n");
        return 0;
    } else return 1;
}

int if_infox_msg (struct bpctl_cmd *bpctl_cmd){
    char if_name[IFNAMSIZ];

    printf("%02x:%02x.%x ",bpctl_cmd->out_param[0],bpctl_cmd->out_param[1],bpctl_cmd->out_param[2]);
    if ((bpctl_cmd->out_param[3]>0)&&(if_indextoname(bpctl_cmd->out_param[3], (char *)&if_name)!=NULL))
        printf("%s ",if_name);
    else printf("     ");
    if ((bpctl_cmd->out_param[2]==1)||(bpctl_cmd->out_param[2]==3)) {
        return 0;
    } else return 1;
}


void set_dis_bypass_cmd(int file_desc,int if_index,int bp_mode, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=bp_mode;

    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_DIS_BYPASS), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if ((ret_val == 0)&&(bpctl_cmd.status==0)) {
        printf(SET_DIS_BYPASS_ENTRY" "SUCCESS_MSG);
    } else {
        printf(NOT_SUPP_SLAVE_MSG);
    }
}

void set_dis_bypass_all_cmd(int file_desc, int dev_num, int bp_mode){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    
    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=bp_mode;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;

        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_DIS_BYPASS), &bpctl_cmd);
        if (ret_val==0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;
            if (bpctl_cmd.status==0)
                printf("ok\n");
            else printf("fail\n");

        }
    }
}


static void if_scan(int file_desc){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    
    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(IF_SCAN), &bpctl_cmd))<0) {
        printf("fail\n");
        return;
    }
    if ((ret_val == 0)&&(bpctl_cmd.status==0)) {
        printf(IF_SCAN_ENTRY" "SUCCESS_MSG);
    } else {
        printf("fail\n");
    }
}


void set_bypass_cmd(int file_desc,int if_index,int tx_state, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=tx_state;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;



    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BYPASS), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if ((ret_val == 0)&&(bpctl_cmd.status==0)) {
        printf(SET_BYPASS_ENTRY" "SUCCESS_MSG);
    } else {
        printf(NOT_SUPP_BP_SLAVE_MSG);
    }
}

void set_bypass_all_cmd(int file_desc,int dev_num,int tx_state){
     
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=tx_state;

    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BYPASS), &bpctl_cmd);
        if (ret_val==0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;
            if (bpctl_cmd.status==0)
                printf("ok\n");
            else
                printf("fail\n");
        }
    }
}

void set_tx_cmd(int file_desc,int if_index,int tx_state, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=tx_state;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_TX), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(SET_TX_ENTRY" "SUCCESS_MSG);
    else
        printf(NOT_SUPP_MSG);
}

void set_tx_all_cmd(int file_desc,int dev_num, int tx_state){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=tx_state;

    for (i=0;i<dev_num;i++) {
        int ret_info=0;
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_TX), &bpctl_cmd);
        if (ret_val==0) {
            ret_info=if_infox_msg(&bpctl_cmd);

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else {
                if (ret_info==0)
                    printf("slave\n");
                else printf("fail\n");    
            }
        }
    }
}

void get_tx_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
     

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_TX), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }

    if ((ret_val !=0)||(bpctl_cmd.status<0)) {
        printf(NOT_SUPP_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(TX_DIS_MSG);
    else
        printf(TX_EN_MSG);
}

void get_tx_all_cmd(int file_desc,int dev_num){
    int ret_val, i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_TX), &bpctl_cmd);
        if (ret_val == 0) {
            int ret_info=if_infox_msg(&bpctl_cmd);

            if (bpctl_cmd.status<0) {
                if (ret_info==0)
                    printf("slave\n");
                else printf("fail\n") ;
            } else {
                if (bpctl_cmd.status==0)
                    printf("off\n");
                else if (bpctl_cmd.status==1)
                    printf("on\n");
            }
        }
    }

}
#ifdef BP_DBI_FLAG

void set_bp_force_link_cmd(int file_desc,int if_index,int tx_state, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=tx_state;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BP_FORCE_LINK), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(SET_FORCE_LINK_ENTRY" "SUCCESS_MSG);
    else
        printf(NOT_SUPP_MSG);
}

void set_bp_force_link_all_cmd(int file_desc,int dev_num, int tx_state){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=tx_state;

    for (i=0;i<dev_num;i++) {
        int ret_info=0;
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BP_FORCE_LINK), &bpctl_cmd);
        if (ret_val==0) {
            ret_info=if_infox_msg(&bpctl_cmd);

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else {
                if (ret_info==0)
                    printf("slave\n");
                else printf("fail\n");    
            }
        }
    }
}

void get_bp_force_link_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
     

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BP_FORCE_LINK), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }

    if ((ret_val !=0)||(bpctl_cmd.status<0)) {
        printf(NOT_SUPP_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(FORCE_LINK_DIS_MSG);
    else
        printf(FORCE_LINK_EN_MSG);
}

void get_bp_force_link_all_cmd(int file_desc,int dev_num){
    int ret_val, i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BP_FORCE_LINK), &bpctl_cmd);
        if (ret_val == 0) {
            int ret_info=if_infox_msg(&bpctl_cmd);

            if (bpctl_cmd.status<0) {
                if (ret_info==0)
                    printf("slave\n");
                else printf("fail\n") ;
            } else {
                if (bpctl_cmd.status==0)
                    printf("off\n");
                else if (bpctl_cmd.status==1)
                    printf("on\n");
            }
        }
    }

}
#endif //BP_DBI_FLAG



void set_tpl_cmd(int file_desc,int if_index,int tpl_state, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=tpl_state;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_TPL), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(SET_TPL_ENTRY" "SUCCESS_MSG);
    else
        printf(NOT_SUPP_SLAVE_MSG);
}

void set_tpl_all_cmd(int file_desc,int dev_num, int tpl_state){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=tpl_state;

    for (i=0;i<dev_num;i++) {
        int ret_info=0;
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_TPL), &bpctl_cmd);
        if (ret_val==0) {
            ret_info=if_infox_msg(&bpctl_cmd);

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else {
                if (ret_info==0)
                    printf("slave\n");
                else printf("fail\n");    
            }
        }
    }
}

void get_tpl_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
     

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_TPL), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }

    if ((ret_val !=0)||(bpctl_cmd.status<0)) {
        printf(NOT_SUPP_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(TPL_DIS_MSG);
    else
        printf(TPL_EN_MSG);
}

void get_tpl_all_cmd(int file_desc,int dev_num){
    int ret_val, i;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=0;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_TPL), &bpctl_cmd);
        if (ret_val == 0) {
            int ret_info=if_infox_msg(&bpctl_cmd);

            if (bpctl_cmd.status<0) {
                if (ret_info==0)
                    printf("slave\n");
                else printf("fail\n") ;
            } else {
                if (bpctl_cmd.status==0)
                    printf("off\n");
                else if (bpctl_cmd.status==1)
                    printf("on\n");
            }
        }
    }

}

//#ifdef PMC_FIX_FLAG
void set_bp_wait_at_pwup_cmd(int file_desc,int if_index,int state, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=state;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BP_WAIT_AT_PWUP), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(SET_BP_WAIT_AT_PWUP_ENTRY" "SUCCESS_MSG);
    else
        printf(NOT_SUPP_MSG);
}

void set_bp_wait_at_pwup_all_cmd(int file_desc,int dev_num, int state){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=state;

    for (i=0;i<dev_num;i++) {
        int ret_info=0;
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BP_WAIT_AT_PWUP), &bpctl_cmd);
        if (ret_val==0) {
            ret_info=if_infox_msg(&bpctl_cmd);

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else {
                if (ret_info==0)
                    printf("slave\n");
                else printf("fail\n");    
            }
        }
    }
}

void get_bp_wait_at_pwup_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
     

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BP_WAIT_AT_PWUP), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }

    if ((ret_val !=0)||(bpctl_cmd.status<0)) {
        printf(NOT_SUPP_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(BP_WAIT_AT_PWUP_DIS_MSG);
    else
        printf(BP_WAIT_AT_PWUP_EN_MSG);
}

void get_bp_wait_at_pwup_all_cmd(int file_desc,int dev_num){
    int ret_val, i;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=0;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BP_WAIT_AT_PWUP), &bpctl_cmd);
        if (ret_val == 0) {
            int ret_info=if_infox_msg(&bpctl_cmd);

            if (bpctl_cmd.status<0) {
                if (ret_info==0)
                    printf("slave\n");
                else printf("fail\n") ;
            } else {
                if (bpctl_cmd.status==0)
                    printf("off\n");
                else if (bpctl_cmd.status==1)
                    printf("on\n");
            }
        }
    }

}

void set_bp_hw_reset_cmd(int file_desc,int if_index,int state, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=state;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BP_HW_RESET), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(SET_BP_HW_RESET_ENTRY" "SUCCESS_MSG);
    else
        printf(NOT_SUPP_MSG);
}

void set_bp_hw_reset_all_cmd(int file_desc,int dev_num, int state){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=state;

    for (i=0;i<dev_num;i++) {
        int ret_info=0;
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BP_HW_RESET), &bpctl_cmd);
        if (ret_val==0) {
            ret_info=if_infox_msg(&bpctl_cmd);

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else {
                if (ret_info==0)
                    printf("slave\n");
                else printf("fail\n");    
            }
        }
    }
}

void get_bp_hw_reset_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
     
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BP_HW_RESET), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }

    if ((ret_val !=0)||(bpctl_cmd.status<0)) {
        printf(NOT_SUPP_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(BP_HW_RESET_DIS_MSG);
    else
        printf(BP_HW_RESET_EN_MSG);
}

void get_bp_hw_reset_all_cmd(int file_desc,int dev_num){
    int ret_val, i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BP_HW_RESET), &bpctl_cmd);
        if (ret_val == 0) {
            int ret_info=if_infox_msg(&bpctl_cmd);

            if (bpctl_cmd.status<0) {
                if (ret_info==0)
                    printf("slave\n");
                else printf("fail\n") ;
            } else {
                if (bpctl_cmd.status==0)
                    printf("off\n");
                else if (bpctl_cmd.status==1)
                    printf("on\n");
            }
        }
    }

}
//#endif


void set_wd_autoreset_cmd(int file_desc,int if_index,int param, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=param;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_WD_AUTORESET), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(SET_WD_AUTORESET_ENTRY" "SUCCESS_MSG);
    else
        printf(NOT_SUPP_BPT_SLAVE_MSG);
}

void set_wd_autoreset_all_cmd(int file_desc,int dev_num, int param){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=param;

    for (i=0;i<dev_num;i++) {
        int ret_info=0;
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_WD_AUTORESET), &bpctl_cmd);
        if (ret_val==0) {
            ret_info=if_infox_msg(&bpctl_cmd);

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else {
                if (ret_info==0)
                    printf("slave\n");
                else printf("fail\n");    
            }
        }
    }
}

void get_wd_autoreset_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
     

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_WD_AUTORESET), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if ((ret_val !=0)||(bpctl_cmd.status<0)) {
        printf(NOT_SUPP_BPT_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(WD_AUTORESET_DIS_MSG);
    else
        printf(WD_AUTORESET_STATE_MSG,bpctl_cmd.status);
}

void get_wd_autoreset_all_cmd(int file_desc,int dev_num){
    int ret_val, i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_WD_AUTORESET), &bpctl_cmd);
        if (ret_val == 0) {
            int ret_info=0;

            if_infox_msg(&bpctl_cmd);

            if (bpctl_cmd.status<0) {
                if (ret_info==0)
                    printf("slave\n");
                else printf("fail\n") ;
            } else {
                if (bpctl_cmd.status==0)
                    printf("disable\n");
                else
                    printf("%d ms\n",bpctl_cmd.status);
            }
        }
    }
}

#ifdef BP_SELF_TEST
void set_bp_self_test_cmd(int file_desc,int if_index,int param, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=param;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BP_SELF_TEST), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(SET_BP_SELF_TEST_ENTRY" "SUCCESS_MSG);
    else
        printf(NOT_SUPP_BPT_SLAVE_MSG);
}

void set_bp_self_test_all_cmd(int file_desc,int dev_num, int param){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=param;

    for (i=0;i<dev_num;i++) {
        int ret_info=0;
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BP_SELF_TEST), &bpctl_cmd);
        if (ret_val==0) {
            ret_info=if_infox_msg(&bpctl_cmd);

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else {
                if (ret_info==0)
                    printf("slave\n");
                else printf("fail\n");    
            }
        }
    }
}

void get_bp_self_test_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
     

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BP_SELF_TEST), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if ((ret_val !=0)||(bpctl_cmd.status<0)) {
        printf(NOT_SUPP_BPT_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(BP_SELF_TEST_DIS_MSG);
    else
        printf(BP_SELF_TEST_MSG,bpctl_cmd.status);
}

void get_bp_self_test_all_cmd(int file_desc,int dev_num){
    int ret_val, i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BP_SELF_TEST), &bpctl_cmd);
        if (ret_val == 0) {
            int ret_info=0;

            if_infox_msg(&bpctl_cmd);

            if (bpctl_cmd.status<0) {
                if (ret_info==0)
                    printf("slave\n");
                else printf("fail\n") ;
            } else {
                if (bpctl_cmd.status==0)
                    printf("disable\n");
                else
                    printf("%d ms\n",bpctl_cmd.status);
            }
        }
    }
}

#endif

void get_dis_bypass_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
     

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;



    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DIS_BYPASS), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(BP_MODE_EN_MSG);
    else if (bpctl_cmd.status==1)
        printf(BP_MODE_DIS_MSG);
    else printf(GET_DIS_BP_FAIL_MSG);
}

void get_dis_bypass_all_cmd(int file_desc,int dev_num){
    struct bpctl_cmd bpctl_cmd;
    int ret_val,i;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DIS_BYPASS), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0) {
                printf("off\n");
            } else if (bpctl_cmd.status==1) {
                printf("on\n");
            } else printf("fail\n");
        }

    }
}


void get_bypass_change_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
     

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS_CHANGE), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }

    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(NON_BYPASS_MODE_LAST_MSG);
    else if (bpctl_cmd.status==1)
        printf(BP_MODE_LAST_MSG);
    else printf(GET_BP_CHANGE_FAIL_MSG);
}

void get_bypass_change_all_cmd(int file_desc,int dev_num){
    int ret_val=0,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS_CHANGE), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("off\n");
            else if (bpctl_cmd.status==1)
                printf("on\n");
            else printf("fail\n");
        }
    }
}


void get_bypass_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
     
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));



    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;       
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;



    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }

    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_BP_SLAVE_UN_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(NON_BP_MODE_MSG);
    else if (bpctl_cmd.status==1)
        printf(BP_MODE_MSG);
    else printf(GET_BP_FAIL_MSG);
}

void get_bypass_all_cmd(int file_desc,int dev_num){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));


    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("off\n");
            else if (bpctl_cmd.status==1)
                printf("on\n");
            else printf("fail\n");
        }
    }
}

void get_bypass_caps_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
     

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS_CAPS), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }

    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_BPT_MSG);
        return;
    }
    printf("bypass_caps: 0x%x\n", bpctl_cmd.status);

    for (i=0;bp_cap_array[i].flag;i++) {
        if (bpctl_cmd.status & bp_cap_array[i].flag)
            printf(bp_cap_array[i].desc);
    }
}

void get_bypass_caps_all_cmd(int file_desc,int dev_num){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;

    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS_CAPS), &bpctl_cmd);
        if (ret_val==0) {
            if_infox_msg(&bpctl_cmd);
            printf("0x%x\n",bpctl_cmd.status);
        }
    }
}

void get_bypass_slave_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    char if_name[IFNAMSIZ];
     
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS_SLAVE), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }

    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_BPT_MSG);
        return;
    }
    if (bpctl_cmd.status==0) {
        printf(SLAVE_IF_MSG);
        return;
    }
    if (bpctl_cmd.status==1) {

        printf("slave: %02x:%02x.%x ",bpctl_cmd.out_param[4],bpctl_cmd.out_param[5],bpctl_cmd.out_param[6]);
        if ((bpctl_cmd.out_param[7]>0)&&(if_indextoname(bpctl_cmd.out_param[7], (char *)&if_name)!=NULL))
            printf("%s",if_name);
        printf("\n");
    }

    else {
        printf(GET_BPSLAVE_FAIL_MSG);
        return;
    }
}

void get_bypass_slave_all_cmd(int file_desc,int dev_num){
    int ret_val, i;
    struct bpctl_cmd bpctl_cmd;
    char if_name[IFNAMSIZ];
     
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS_SLAVE), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

			if (bpctl_cmd.status < 0)
                printf("fail\n");
            else {
                printf("slave: %02x:%02x.%x ",bpctl_cmd.out_param[4],bpctl_cmd.out_param[5],bpctl_cmd.out_param[6]);
                if ((bpctl_cmd.out_param[7]>0)&&(if_indextoname(bpctl_cmd.out_param[7], (char *)&if_name)!=NULL))
                    printf("%s",if_name);
                printf("\n"); 
            }
        }
    }
}


void is_bypass_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(IS_BYPASS), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }

    if (bpctl_cmd.status <0) {
        printf(NOT_SUPP_BPT_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(SLAVE_IF_MSG);
    else  printf(MASTER_IF_MSG);
}

void is_bypass_all_cmd(int file_desc,int dev_num){
    int ret_val, i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(IS_BYPASS), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status<0)
                printf("fail\n");
            else
                printf("%s\n",bpctl_cmd.status==0?"slave":"master");
        }
    }
}

int parse_bypass_mode(char *param){
    int bypass_mode=-1;

    if (!strcmp(param, BYPASS_ENABLE))
        bypass_mode=1;
    else if (!strcmp(param, BYPASS_DISABLE))
        bypass_mode=0;
    return bypass_mode;
}

int parse_wd_exp_mode(char *param){
    int mode=-1;

    if (!strcmp(param, TAP_MODE))
        mode=1;
    else if (!strcmp(param, BYPASS_MODE))
        mode=0;
    else if (!strcmp(param, DISC_MODE))
        mode=2;

    return mode;
}

void get_wd_set_caps_cmd(int file_desc, int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
     
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;



    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_WD_SET_CAPS), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }

    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0) {
        printf(SLAVE_IF_MSG);
        return;
    }
    printf(WD_MIN_TIME_MSG,WD_MIN_TIME_GET(bpctl_cmd.status)*100);
    printf(WD_STEP_TIME_MSG,((bpctl_cmd.status & WDT_STEP_TIME)?1:0));
    printf(WD_STEP_COUNT_MSG,WD_STEP_COUNT_GET(bpctl_cmd.status));
}

void get_wd_set_caps_all_cmd(int file_desc, int dev_num){
    int ret_val, i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_WD_SET_CAPS), &bpctl_cmd);
        if (ret_val==0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;
            if (bpctl_cmd.status<0)
                printf("fail\n");
            else printf("0x%x\n",bpctl_cmd.status);
        }
    }
}

void set_bypass_pwoff_cmd(int file_desc, int if_index,int bypass_mode, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=bypass_mode;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BYPASS_PWOFF), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }

    if ((ret_val == 0)&&(bpctl_cmd.status==0)) {
        printf(SET_BYPASS_PWOFF_ENTRY" "SUCCESS_MSG);
    } else {
        printf(NOT_SUPP_SLAVE_MSG);
    }
}

void set_bypass_pwoff_all_cmd(int file_desc, int dev_num, int bypass_mode){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=bypass_mode;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BYPASS_PWOFF), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else printf("fail\n");
        }
    }
}


void get_bypass_pwoff_cmd(int file_desc, int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
     

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS_PWOFF), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }

    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(BP_DIS_PWOFF_MSG);
    else if (bpctl_cmd.status==1)
        printf(BP_EN_PWOFF_MSG);
    else printf(GET_BP_PWOFF_FAIL_MSG);
}

void get_bypass_pwoff_all_cmd(int file_desc, int dev_num){

     
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS_PWOFF), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("off\n");
            else if (bpctl_cmd.status==1)
                printf("on\n");
            else printf("fail\n");
        }
    }
}


void set_bypass_pwup_cmd(int file_desc, int if_index,int bypass_mode, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=bypass_mode;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BYPASS_PWUP), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }

    if ((ret_val == 0)&&(bpctl_cmd.status==0)) {
        printf(SET_BYPASS_PWUP_ENTRY" "SUCCESS_MSG);
    } else {
        printf(NOT_SUPP_SLAVE_MSG);
        exit(-1);
    }
}

void set_bypass_pwup_all_cmd(int file_desc, int dev_num,int bypass_mode){

    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=bypass_mode;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BYPASS_PWUP), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else printf("fail\n");


        }
    }
}


void get_bypass_pwup_cmd(int file_desc, int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
     
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS_PWUP), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(BP_DIS_PWUP_MSG);
    else if (bpctl_cmd.status==1)
        printf(BP_EN_PWUP_MSG);
    else printf(GET_BP_PWUP_FAIL_MSG);
}

void get_bypass_pwup_all_cmd(int file_desc, int dev_num){

     
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS_PWUP), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("off\n");
            else if (bpctl_cmd.status==1)
                printf("on\n");
            else printf("fail\n");
        }
    }
}


void set_bypass_wd_cmd(int file_desc,int if_index,int timeout, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=timeout;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BYPASS_WD), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if ((ret_val == 0)&&(bpctl_cmd.status>=0)) {
        if (!bpctl_cmd.status)
            printf(WD_DIS_MSG);
        else if (bpctl_cmd.status>0)
            printf(WDT_STATE_MSG,bpctl_cmd.status);
    } else {
        printf(NOT_SUPP_SLAVE_MSG); 
    }
}

void set_bypass_wd_all_cmd(int file_desc, int dev_num, int timeout){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=timeout;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_BYPASS_WD), &bpctl_cmd);
        if (ret_val==0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status<0)
                printf("fail\n");
            else if (bpctl_cmd.status==0)
                printf("disable\n");
            else
                printf("%d\n",bpctl_cmd.status);
        }



    }
}


void get_bypass_wd_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS_WD), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_SLAVE_MSG);
        return;
    }
    if (!bpctl_cmd.data[0])
        printf(WD_DIS_MSG);
    else if (bpctl_cmd.data[0]>0)
        printf(WD_STATE_EXT_MSG,bpctl_cmd.data[0] );
    else printf(WD_STATE_UNKNOWN_MSG);
}

void get_bypass_wd_all_cmd(int file_desc, int dev_num){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS_WD), &bpctl_cmd);
        if (ret_val==0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status<0)
                printf("fail\n");
            else if (!bpctl_cmd.data[0])
                printf("disable\n");
            else if (bpctl_cmd.data[0]>0)
                printf("%d\n", bpctl_cmd.data[0]);
            else printf("unknown\n");
        }



    }
}

void get_wd_expire_time_cmd(int file_desc, int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
     

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index; 
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_WD_EXPIRE_TIME), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0)
        printf(NOT_WDT_SLAVE_MSG);
    else if (!bpctl_cmd.data[0])
        printf(WD_DIS_MSG);
    else if (bpctl_cmd.data[0]>0)
        printf(WD_TIME_LEFT_MSG,bpctl_cmd.data[0] );
    else
        printf(WDT_EXP_MSG);    
}

void get_wd_expire_time_all_cmd(int file_desc, int dev_num){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=0;

    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_WD_EXPIRE_TIME), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status<0)
                printf("fail\n");
            else if (!bpctl_cmd.data[0])
                printf("disable\n");
            else if (bpctl_cmd.data[0]>0)
                printf("%d\n", bpctl_cmd.data[0]);
            else printf("expired\n");
        }



    }
}

void reset_bypass_wd_cmd(int file_desc, int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index; 
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(RESET_BYPASS_WD_TIMER), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if ((ret_val == 0)&&(bpctl_cmd.status>=0)) {
        if (bpctl_cmd.status<0)
            printf(NOT_WDT_SLAVE_MSG);
        else if (!bpctl_cmd.status)
            printf(WD_DIS_MSG);
        else if (bpctl_cmd.status>0)
            printf(RESET_BYPASS_WD_TIMER_ENTRY" "SUCCESS_MSG);
    } else {
        printf(NOT_SUPP_SLAVE_MSG); 
    }
}

void reset_bypass_wd_all_cmd(int file_desc, int dev_num){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;

    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(RESET_BYPASS_WD_TIMER), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status<0)
                printf("fail\n");
            else if (bpctl_cmd.status==0)
                printf("disable\n");
            else printf("ok\n");
        }
    }
}

void set_std_nic_cmd(int file_desc, int if_index, int nic_mode, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=nic_mode;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_STD_NIC), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if ((ret_val == 0)&&(bpctl_cmd.status>=0))
        printf(SET_STD_NIC_ENTRY" "SUCCESS_MSG);
    else
        printf(NOT_SUPP_SLAVE_MSG);
}

void set_std_nic_all_cmd(int file_desc, int dev_num, int nic_mode){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd; 
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));


    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=nic_mode;

    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_STD_NIC), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status<0)
                printf("fail\n");
            else printf("ok\n");



        }
    }
}


void get_std_nic_cmd(int file_desc, int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
     
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_STD_NIC), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0)
        printf(NOT_SUPP_SLAVE_MSG);
    else if (!bpctl_cmd.status)
        printf(NON_STD_NIC_MODE_MSG);
    else
        printf(STD_NIC_MODE_MSG);
}

void get_std_nic_all_cmd(int file_desc, int dev_num){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;

    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_STD_NIC), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status<0)
                printf("fail\n");
            else if (bpctl_cmd.status==0)
                printf("non-standard\n");
            else printf("standard\n");
        }
    }
}

void set_dis_tap_cmd(int file_desc,int if_index,int tap_mode, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=tap_mode;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_DIS_TAP), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if ((ret_val == 0)&&(bpctl_cmd.status==0)) {
        printf(SET_DIS_TAP_ENTRY" "SUCCESS_MSG);
    } else {
        printf(NOT_SUPP_SLAVE_MSG);
    }
}

void set_dis_tap_all_cmd(int file_desc,int dev_num, int tap_mode){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=tap_mode;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;

        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_DIS_TAP), &bpctl_cmd);
        if (ret_val==0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else printf("fail\n");
        }
    }
}


void set_tap_cmd(int file_desc,int if_index,int tap_mode, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=tap_mode;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_TAP), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if ((ret_val == 0)&&(bpctl_cmd.status==0)) {
        printf(SET_TAP_ENTRY" "SUCCESS_MSG);
    } else {
        printf(NOT_SUPP_BP_SLAVE_MSG);
    }
}

void set_tap_all_cmd(int file_desc,int dev_num,int tap_mode){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=tap_mode;

    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_TAP), &bpctl_cmd);
        if (ret_val==0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else
                printf("fail\n");
        }
    }
}

void get_dis_tap_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DIS_TAP), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(TAP_MODE_EN_MSG);
    else if (bpctl_cmd.status==1)
        printf(TAP_MODE_DIS_MSG);
    else printf(GET_DIS_TAP_FAIL_MSG);
}

void get_dis_tap_all_cmd(int file_desc,int dev_num){
    struct bpctl_cmd bpctl_cmd;
    int ret_val=0, i;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DIS_TAP), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0) {
                printf("off\n");
            } else if (bpctl_cmd.status==1) {
                printf("on\n");
            } else printf("fail\n");
        }
    }
}


void get_tap_change_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_TAP_CHANGE), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(NON_TAP_MODE_LAST_MSG);
    else if (bpctl_cmd.status==1)
        printf(TAP_MODE_LAST_MSG);
    else printf(GET_TAP_CHANGE_FAIL_MSG);
}

void get_tap_change_all_cmd(int file_desc,int dev_num){
    struct bpctl_cmd bpctl_cmd;
    int ret_val=0, i;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_TAP_CHANGE), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("off\n");
            else if (bpctl_cmd.status==1)
                printf("on\n");
            else printf("fail\n");
        }
    }
}

void get_tap_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_TAP), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_BPT_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(NON_TAP_MODE_MSG);
    else if (bpctl_cmd.status==1)
        printf(TAP_MODE_MSG);
    else printf(GET_TAP_FAIL_MSG);
}

void get_tap_all_cmd(int file_desc,int dev_num){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;

    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_TAP), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("off\n");
            else if (bpctl_cmd.status==1)
                printf("on\n");
            else printf("fail\n");
        }
    }
}

void set_tap_pwup_cmd(int file_desc, int if_index,int tap_mode, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=tap_mode;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_TAP_PWUP), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if ((ret_val == 0)&&(bpctl_cmd.status==0)) {
        printf(SET_TAP_PWUP_ENTRY" "SUCCESS_MSG);
    } else {
        printf(NOT_SUPP_SLAVE_MSG);
        exit(-1);
    }
}

void set_tap_pwup_all_cmd(int file_desc, int dev_num, int tap_mode){

    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=tap_mode;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_TAP_PWUP), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else printf("fail\n");



        }
    }
}


void get_tap_pwup_cmd(int file_desc, int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_TAP_PWUP), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(TAP_DIS_PWUP_MSG);
    else if (bpctl_cmd.status==1)
        printf(TAP_EN_PWUP_MSG);
    else printf(GET_TAP_PWUP_FAIL_MSG);
}

void get_tap_pwup_all_cmd(int file_desc, int dev_num){

     
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_TAP_PWUP), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("off\n");
            else if (bpctl_cmd.status==1)
                printf("on\n");
            else printf("fail\n");
        }
    }
}
void set_dis_disc_cmd(int file_desc,int if_index,int disc_mode, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=disc_mode;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_DIS_DISC), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if ((ret_val == 0)&&(bpctl_cmd.status==0)) {
        printf(SET_DIS_DISC_ENTRY" "SUCCESS_MSG);
    } else {
        printf(NOT_SUPP_SLAVE_MSG);
    }
}

void set_dis_disc_all_cmd(int file_desc,int dev_num, int disc_mode){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=disc_mode;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;

        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_DIS_DISC), &bpctl_cmd);
        if (ret_val==0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else printf("fail\n");
        }
    }
}


void set_disc_cmd(int file_desc,int if_index,int disc_mode, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=disc_mode;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_DISC), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if ((ret_val == 0)&&(bpctl_cmd.status==0)) {
        printf(SET_DISC_ENTRY" "SUCCESS_MSG);
    } else {
        printf(NOT_SUPP_BP_SLAVE_MSG);
    }
}

void set_disc_all_cmd(int file_desc,int dev_num,int disc_mode){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=disc_mode;

    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_DISC), &bpctl_cmd);
        if (ret_val==0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else
                printf("fail\n");
        }
    }
}

void get_dis_disc_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DIS_DISC), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(DISC_MODE_EN_MSG);
    else if (bpctl_cmd.status==1)
        printf(DISC_MODE_DIS_MSG);
    else printf(GET_DIS_DISC_FAIL_MSG);
}

void get_dis_disc_all_cmd(int file_desc,int dev_num){
    struct bpctl_cmd bpctl_cmd;
    int ret_val=0, i;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DIS_DISC), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0) {
                printf("off\n");
            } else if (bpctl_cmd.status==1) {
                printf("on\n");
            } else printf("fail\n");
        }
    }
}


void get_disc_change_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DISC_CHANGE), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(NON_DISC_MODE_LAST_MSG);
    else if (bpctl_cmd.status==1)
        printf(DISC_MODE_LAST_MSG);
    else printf(GET_DISC_CHANGE_FAIL_MSG);
}

void get_disc_change_all_cmd(int file_desc,int dev_num){
    struct bpctl_cmd bpctl_cmd;
    int ret_val=0, i;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DISC_CHANGE), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("off\n");
            else if (bpctl_cmd.status==1)
                printf("on\n");
            else printf("fail\n");
        }
    }
}

void get_disc_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DISC), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_BPT_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(NON_DISC_MODE_MSG);
    else if (bpctl_cmd.status==1)
        printf(DISC_MODE_MSG);
    else printf(GET_DISC_FAIL_MSG);
}

void get_disc_all_cmd(int file_desc,int dev_num){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DISC), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("off\n");
            else if (bpctl_cmd.status==1)
                printf("on\n");
            else printf("fail\n");
        }
    }
}

void set_disc_pwup_cmd(int file_desc, int if_index,int disc_mode, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=disc_mode;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_DISC_PWUP), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if ((ret_val == 0)&&(bpctl_cmd.status==0)) {
        printf(SET_DISC_PWUP_ENTRY" "SUCCESS_MSG);
    } else {
        printf(NOT_SUPP_SLAVE_MSG);
        exit(-1);
    }
}

void set_disc_pwup_all_cmd(int file_desc, int dev_num, int disc_mode){

    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=disc_mode;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_DISC_PWUP), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else printf("fail\n");



        }
    }
}


void get_disc_pwup_cmd(int file_desc, int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DISC_PWUP), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(DISC_DIS_PWUP_MSG);
    else if (bpctl_cmd.status==1)
        printf(DISC_EN_PWUP_MSG);
    else printf(GET_DISC_PWUP_FAIL_MSG);
}

void get_disc_pwup_all_cmd(int file_desc, int dev_num){

     
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DISC_PWUP), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("off\n");
            else if (bpctl_cmd.status==1)
                printf("on\n");
            else printf("fail\n");
        }
    }
}

void set_disc_port_cmd(int file_desc,int if_index,int disc_mode, int bus, int slot, int func){
    int ret_val ;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=disc_mode;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_DISC_PORT), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if ((ret_val == 0)&&(bpctl_cmd.status==0)) {
        printf(SET_DISC_PORT_ENTRY" "SUCCESS_MSG);
    } else {
        printf(NOT_SUPP_BP_SLAVE_MSG);
    }
}

void set_disc_port_all_cmd(int file_desc,int dev_num,int disc_mode){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=disc_mode;

    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_DISC_PORT), &bpctl_cmd);
        if (ret_val==0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else
                printf("fail\n");
        }
    }
}


void get_disc_port_cmd(int file_desc,int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DISC_PORT), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_BPT_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(NON_DISC_PORT_MODE_MSG);
    else if (bpctl_cmd.status==1)
        printf(DISC_PORT_MODE_MSG);
    else printf(GET_DISC_FAIL_MSG);
}

void get_disc_port_all_cmd(int file_desc,int dev_num){
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DISC_PORT), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("off\n");
            else if (bpctl_cmd.status==1)
                printf("on\n");
            else printf("fail\n");
        }
    }
}

void set_disc_port_pwup_cmd(int file_desc, int if_index,int disc_mode, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));
    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=disc_mode;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_DISC_PORT_PWUP), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if ((ret_val == 0)&&(bpctl_cmd.status==0)) {
        printf(SET_DISC_PORT_PWUP_ENTRY" "SUCCESS_MSG);
    } else {
        printf(NOT_SUPP_SLAVE_MSG);
        exit(-1);
    }
}

void set_disc_port_pwup_all_cmd(int file_desc, int dev_num, int disc_mode){

    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=disc_mode;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_DISC_PORT_PWUP), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else printf("fail\n");



        }
    }
}


void get_disc_port_pwup_cmd(int file_desc, int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DISC_PORT_PWUP), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_SLAVE_MSG);
        return;
    }
    if (bpctl_cmd.status==0)
        printf(DISC_PORT_DIS_PWUP_MSG);
    else if (bpctl_cmd.status==1)
        printf(DISC_PORT_EN_PWUP_MSG);
    else printf(GET_DISC_PORT_PWUP_FAIL_MSG);
}

void get_disc_port_pwup_all_cmd(int file_desc, int dev_num){

     
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_DISC_PORT_PWUP), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("off\n");
            else if (bpctl_cmd.status==1)
                printf("on\n");
            else printf("fail\n");
        }
    }
}



void set_wd_exp_mode_cmd(int file_desc, int if_index,int mode, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[2]=mode;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_WD_EXP_MODE), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0)
        printf(NOT_SUPP_SLAVE_MSG);
    else printf(SET_WD_EXP_MODE_ENTRY" "SUCCESS_MSG);
}

void set_wd_exp_mode_all_cmd(int file_desc, int dev_num, int mode){

    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    bpctl_cmd.in_param[2]=mode;

    for (i=0;i<dev_num;i++) {

        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(SET_WD_EXP_MODE), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;

            if (bpctl_cmd.status==0)
                printf("ok\n");
            else printf("fail\n");  
        }
    }
}

void get_wd_exp_mode_cmd(int file_desc, int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;

    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_WD_EXP_MODE), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0)
        printf(NOT_SUPP_SLAVE_MSG);
    else if (!bpctl_cmd.status)
        printf(BYPASS_WD_EXP_MODE_MSG);
    else if (bpctl_cmd.status==1)
        printf(TAP_WD_EXP_MODE_MSG);
    else if (bpctl_cmd.status==2)
        printf(DISC_WD_EXP_MODE_MSG);
    else printf("fail\n");

}

void get_wd_exp_mode_all_cmd(int file_desc, int dev_num){
     
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    bpctl_cmd.in_param[1]=0;
    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_WD_EXP_MODE), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;
            if (bpctl_cmd.status==0)
                printf("bypass\n");
            else if (bpctl_cmd.status==1)
                printf("tap\n");
            else if (bpctl_cmd.status==2)
                printf("disc\n");

            else printf("fail\n");
        }
    }
}

void get_bypass_info_cmd(int file_desc, int if_index, int bus, int slot, int func){
    int ret_val;
    struct bpctl_cmd bpctl_cmd; 
    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));


    bpctl_cmd.in_param[1]=if_index;
    bpctl_cmd.in_param[5]=bus;
    bpctl_cmd.in_param[6]= slot;
    bpctl_cmd.in_param[7]= func;


    if ((ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS_INFO), &bpctl_cmd))<0) {
        printf(NOT_SUPP_BPCTL_MSG);
        return;
    }
    if (bpctl_cmd.status<0) {
        printf(NOT_SUPP_SLAVE_MSG);
        return;
    }
    printf("Name\t\t\t%s\n", (char *)bpctl_cmd.data);
    printf("Firmware version\t0x%x\n", bpctl_cmd.out_param[4]);
}

void get_bypass_info_all_cmd(int file_desc, int dev_num){
     
    int ret_val,i;
    struct bpctl_cmd bpctl_cmd;

    memset(&bpctl_cmd, 0, sizeof(bpctl_cmd));

    for (i=0;i<dev_num;i++) {
        bpctl_cmd.in_param[0]=i;
        ret_val = ioctl(file_desc, IOCTL_TX_MSG(GET_BYPASS_INFO), &bpctl_cmd);
        if (ret_val == 0) {
            if (if_info_msg(file_desc, &bpctl_cmd)==0)
                continue;
            if (bpctl_cmd.status==0) {
                printf("\n\tName\t\t\t%s\n", (char *)bpctl_cmd.data);
                printf("\tFirmware version\t0x%x\n", bpctl_cmd.out_param[4]);
            } else printf("fail\n");
        }
    }
}

static void str_low(char *str){
    int i;

    for (i=0;i<strlen(str);i++)
        if ((str[i]>=65)&&(str[i]<=90))
            str[i]+=32;
}

static unsigned long str_to_hex(char *p) {
    unsigned long      hex    = 0;
    unsigned long     length = strlen(p), shift  = 0;
    unsigned char     dig    = 0;

    str_low(p);

    length = strlen(p);

    if (length == 0 )
        return 0;

    do {
        dig  = p[--length];
        dig  = dig<'a' ? (dig - '0') : (dig - 'a' + 0xa);
        hex |= (dig<<shift);
        shift += 4;
    }while (length);
    return hex;
}


int parse_cmdline(int argc, char **argp, int file_desc){
    int err=0, if_index=0, dev_num=0, i=0;
    char *if_name;
    int bus=0, slot=0, func=0;
    char *cmd_user;

    if (argc==1) {
        err=0;
    } else if (argc==2) {
        i=1;
        if (!strcmp(argp[i], INFO_ENTRY)) {
            printf(APP_NAME" Version "UTIL_VER" \n");
            printf(COPYRT_MSG"\n");
            return OK;
        } else if (!strcmp(argp[i], HELP_ENTRY)) {
            bp_usage();
            return 1;
        } else if (!strcmp(argp[i], IF_SCAN_ENTRY)) {
            if_scan(file_desc);
            return 1;
        } else goto cmd_err;
    } else if (argc>=3) {
        struct bpctl_cmd bpctl_cmd;
#if 0
        if ((strncmp(argp[1],"eth",3))&&((strncmp(argp[1],"all",3)))) {
            fprintf(stderr,NOT_NET_DEV_MSG,argp[1]);
            return ERROR;
        }
#endif	
        if (strstr(argp[1], ":")) {
            cmd_user=strdup(argp[1]);
            bus = str_to_hex(strsep (&cmd_user, ":"));
            if (!strstr(cmd_user, "."))
                goto cmd_err;
            slot = str_to_hex(strsep (&cmd_user, "."));
            func=  str_to_hex(cmd_user);

            //  return 0;

        } else if (!strcmp(argp[1], "all")) {
            if_index=0;
            if (ioctl(file_desc, IOCTL_TX_MSG(GET_DEV_NUM), &bpctl_cmd)<0) {
                printf(GET_DEV_NUM_FAIL_MSG);
                return ERROR;
            }
            if ((dev_num=bpctl_cmd.out_param[0])==0) {
                printf(NO_BPCTL_DEV_MSG);
                return ERROR;
            }

        } else {
            if_name= argp[1];
            if_index=if_nametoindex(if_name);
            if (if_index==0) {
                printf("%s is not exist!\n",if_name);
                return ERROR;
            }
            bpctl_cmd.in_param[1]=if_index;
        }    
        i=2;
        if (!strcmp(argp[i], SET_TX_ENTRY)) {
            int nic_mode=0;
            if ((argc<(i+2))||((nic_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(tx,file_desc,if_index,dev_num,nic_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_TX_ENTRY)) {
            BPCTL_GET_CMD(tx,file_desc,if_index,dev_num, bus, slot, func);
            return OK;      
        }if (!strcmp(argp[i], SET_TPL_ENTRY)) {
            int nic_mode=0;
            if ((argc<(i+2))||((nic_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(tpl,file_desc,if_index,dev_num,nic_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_TPL_ENTRY)) {
            BPCTL_GET_CMD(tpl,file_desc,if_index,dev_num, bus, slot, func);
            return OK;      
        } else if (!strcmp(argp[i], IS_BYPASS_ENTRY)) {
            IS_BYPASS_CMD(file_desc,if_index,dev_num, bus, slot, func);
            return OK;      
        } else if (!strcmp(argp[i], GET_BYPASS_SLAVE_ENTRY)) {
            BPCTL_GET_CMD(bypass_slave,file_desc,if_index,dev_num, bus, slot, func);
            return OK;      
        } else if (!strcmp(argp[i], GET_BYPASS_CAPS_ENTRY)) {
            BPCTL_GET_CMD(bypass_caps,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_WD_SET_CAPS_ENTRY)) {
            BPCTL_GET_CMD(wd_set_caps,file_desc,if_index,dev_num, bus, slot, func);
            return OK;      
        } else if (!strcmp(argp[i], SET_BYPASS_ENTRY)) {
            int nic_mode=0;

            if ((argc<(i+2))||((nic_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;

            BPCTL_SET_CMD(bypass,file_desc,if_index,dev_num,nic_mode, bus, slot, func);
            return OK;

        } else if (!strcmp(argp[i], GET_BYPASS_ENTRY)) {
            BPCTL_GET_CMD(bypass,file_desc,if_index,dev_num, bus, slot, func);
            return OK;      
        } else if (!strcmp(argp[i], GET_BYPASS_CHANGE_ENTRY)) {
            BPCTL_GET_CMD(bypass_change,file_desc,if_index,dev_num, bus, slot, func);
            return OK;      
        } else if (!strcmp(argp[i], SET_DIS_BYPASS_ENTRY)) {
            int nic_mode=0;
            if ((argc<(i+2))||((nic_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(dis_bypass,file_desc,if_index,dev_num,nic_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_DIS_BYPASS_ENTRY)) {
            BPCTL_GET_CMD(dis_bypass,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], SET_BYPASS_PWOFF_ENTRY)) {
            int bypass_mode=0;
            if ((argc<(i+2))||((bypass_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(bypass_pwoff,file_desc,if_index,dev_num,bypass_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_BYPASS_PWOFF_ENTRY)) {
            BPCTL_GET_CMD(bypass_pwoff,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], SET_BYPASS_PWUP_ENTRY)) {
            int bypass_mode=0;
            if ((argc<(i+2))||((bypass_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(bypass_pwup,file_desc,if_index,dev_num,bypass_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_BYPASS_PWUP_ENTRY)) {
            BPCTL_GET_CMD(bypass_pwup,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], SET_STD_NIC_ENTRY)) {
            int nic_mode=0;
            if ((argc<(i+2))||((nic_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(std_nic,file_desc,if_index,dev_num,nic_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_STD_NIC_ENTRY)) {
            BPCTL_GET_CMD(std_nic,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], SET_TX_ENTRY)) {
            int nic_mode=0;
            if ((argc<(i+2))||((nic_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(tx,file_desc, if_index,dev_num,nic_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_TX_ENTRY)) {
            BPCTL_GET_CMD(tx,file_desc, if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], SET_BYPASS_WD_ENTRY)) {
            int timeout=0;
            if (argc<(i+2))
                goto cmd_err;
            timeout=atoi(argp[i+1]);
            BPCTL_SET_CMD(bypass_wd,file_desc,if_index,dev_num,timeout, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_BYPASS_WD_ENTRY)) {
            BPCTL_GET_CMD(bypass_wd,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_WD_EXPIRE_TIME_ENTRY)) {
            BPCTL_GET_CMD(wd_expire_time,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], RESET_BYPASS_WD_TIMER_ENTRY)) {
            RESET_WD_CMD(file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], SET_TAP_ENTRY)) {
            int bypass_mode=0;
            if ((argc<(i+2))||((bypass_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(tap,file_desc,if_index,dev_num,bypass_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_TAP_ENTRY)) {
            BPCTL_GET_CMD(tap,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_TAP_CHANGE_ENTRY)) {
            BPCTL_GET_CMD(tap_change,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], SET_DIS_TAP_ENTRY)) {
            int bypass_mode=0; 
            if ((argc<(i+2))||((bypass_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(dis_tap,file_desc,if_index,dev_num,bypass_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_DIS_TAP_ENTRY)) {
            BPCTL_GET_CMD(dis_tap,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], SET_TAP_PWUP_ENTRY)) {
            int bypass_mode=0;
            if ((argc<(i+2))||((bypass_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(tap_pwup,file_desc,if_index,dev_num,bypass_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_TAP_PWUP_ENTRY)) {
            BPCTL_GET_CMD(tap_pwup,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        }
#if 0
        else if (!strcmp(argp[i], SET_DISC_PORT_ENTRY)) {
            int bypass_mode=0;
            if ((argc<(i+2))||((bypass_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(disc_port,file_desc,if_index,dev_num,bypass_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_DISC_PORT_ENTRY)) {
            BPCTL_GET_CMD(disc_port,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        }  else if (!strcmp(argp[i], SET_DISC_PORT_PWUP_ENTRY)) {
            int bypass_mode=0;
            if ((argc<(i+2))||((bypass_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(disc_port_pwup,file_desc,if_index,dev_num,bypass_mode, bus, slot, func);
            return OK;
        }
#endif
         else if (!strcmp(argp[i], GET_DISC_PORT_PWUP_ENTRY)) {
            BPCTL_GET_CMD(disc_port_pwup,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], SET_DISC_ENTRY)) {
            int bypass_mode=0;
            if ((argc<(i+2))||((bypass_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(disc,file_desc,if_index,dev_num,bypass_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_DISC_ENTRY)) {
            BPCTL_GET_CMD(disc,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_DISC_CHANGE_ENTRY)) {
            BPCTL_GET_CMD(disc_change,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], SET_DIS_DISC_ENTRY)) {
            int bypass_mode=0; 
            if ((argc<(i+2))||((bypass_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(dis_disc,file_desc,if_index,dev_num,bypass_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_DIS_DISC_ENTRY)) {
            BPCTL_GET_CMD(dis_disc,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], SET_DISC_PWUP_ENTRY)) {
            int bypass_mode=0;
            if ((argc<(i+2))||((bypass_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(disc_pwup,file_desc,if_index,dev_num,bypass_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_DISC_PWUP_ENTRY)) {
            BPCTL_GET_CMD(disc_pwup,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        }

        else if (!strcmp(argp[i], SET_WD_EXP_MODE_ENTRY)) {
            int mode=0;
            if ((argc<(i+2))||((mode=parse_wd_exp_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(wd_exp_mode,file_desc,if_index,dev_num,mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_WD_EXP_MODE_ENTRY)) {
            BPCTL_GET_CMD(wd_exp_mode,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], SET_WD_AUTORESET_ENTRY)) {
            int timeout=0;
            if (argc<(i+2))
                goto cmd_err;
            timeout=atoi(argp[i+1]);
            BPCTL_SET_CMD(wd_autoreset,file_desc, if_index,dev_num,timeout, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_WD_AUTORESET_ENTRY)) {
            BPCTL_GET_CMD(wd_autoreset,file_desc, if_index,dev_num, bus, slot, func);
            return OK;
#ifdef BP_DBI_FLAG
        } else if (!strcmp(argp[i], SET_FORCE_LINK_ENTRY)) {
            int bypass_mode=0;
            if ((argc<(i+2))||((bypass_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(bp_force_link,file_desc,if_index,dev_num,bypass_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_FORCE_LINK_ENTRY)) {
            BPCTL_GET_CMD(bp_force_link,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
#endif

#ifdef BP_SELF_TEST
        } else if (!strcmp(argp[i], SET_BP_SELF_TEST_ENTRY)) {
            int timeout=0;
            if (argc<(i+2))
                goto cmd_err;
            timeout=atoi(argp[i+1]);
            BPCTL_SET_CMD(bp_self_test,file_desc, if_index,dev_num,timeout, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_BP_SELF_TEST_ENTRY)) {
            BPCTL_GET_CMD(bp_self_test,file_desc, if_index,dev_num, bus, slot, func);
            return OK;
#endif

//#ifdef PMC_FIX_FLAG
        } else if (!strcmp(argp[i], SET_HW_RESET_ENTRY)) {
            int bypass_mode=0;
            if ((argc<(i+2))||((bypass_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(bp_hw_reset,file_desc,if_index,dev_num,bypass_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_HW_RESET_ENTRY)) {
            BPCTL_GET_CMD(bp_hw_reset,file_desc,if_index,dev_num, bus, slot, func);
            return OK;

        } else if (!strcmp(argp[i], SET_WAIT_AT_PWUP_ENTRY)) {
            int bypass_mode=0;
            if ((argc<(i+2))||((bypass_mode=parse_bypass_mode(argp[i+1]))<0))
                goto cmd_err;
            BPCTL_SET_CMD(bp_wait_at_pwup,file_desc,if_index,dev_num,bypass_mode, bus, slot, func);
            return OK;
        } else if (!strcmp(argp[i], GET_WAIT_AT_PWUP_ENTRY)) {
            BPCTL_GET_CMD(bp_wait_at_pwup,file_desc,if_index,dev_num, bus, slot, func);
            return OK;
//#endif
        } else if (!strcmp(argp[i], GET_BYPASS_INFO_ENTRY)) {
            BPCTL_GET_CMD(bypass_info, file_desc,if_index,dev_num, bus, slot, func);
            return OK;
        }
    }
    cmd_err:
    printf("Command line error!\n");
    bp_usage();
    return ERROR;
}

/*
* Main - Call the ioctl functions
*/
int main(int argc, char **argp, char **envp){

    int file_desc;
    file_desc = open(DEVICE_NODE, 0);

    if (file_desc < 0) {
        printf("Can't open device file: %s\n", DEVICE_NAME);
        exit(-1);
    }
    parse_cmdline(argc, argp, file_desc);
    close(file_desc);
    return OK;
}











































































