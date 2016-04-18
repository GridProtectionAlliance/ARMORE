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

#ifndef BPCTL_UTIL_H
#define BPCTL_UTIL_H

#include "bp_msg.h"


#define APP_NAME     "Bypass-SD Control utility"
#define COPYRT_MSG   "Copyright Silicom Ltd."
#define PROG_NAME    "bpctl_util"
#define UTIL_VER      VER_STR_SET


#define OK                           0
#define ERROR                        1

#define HELP_ENTRY                   "help"
#define INFO_ENTRY                   "info"
#define IF_SCAN_ENTRY                "if_scan"

#define  SET_TX_ENTRY                "set_tx"
#define  GET_TX_ENTRY                "get_tx"
#define  SET_TPL_ENTRY               "set_tpl"
#define  GET_TPL_ENTRY               "get_tpl"
#define  GET_DEV_NUM_ENTRY           "get_dev_num"

#define  IS_BYPASS_ENTRY             "is_bypass"
#define  GET_BYPASS_SLAVE_ENTRY      "get_bypass_slave"
#define  GET_BYPASS_CAPS_ENTRY       "get_bypass_caps"
#define  GET_WD_SET_CAPS_ENTRY       "get_wd_set_caps"
#define  SET_BYPASS_ENTRY            "set_bypass"
#define  GET_BYPASS_ENTRY            "get_bypass"
#define  GET_BYPASS_CHANGE_ENTRY     "get_bypass_change"
#define  SET_DIS_BYPASS_ENTRY        "set_dis_bypass"
#define  GET_DIS_BYPASS_ENTRY        "get_dis_bypass"
#define  SET_BYPASS_PWOFF_ENTRY      "set_bypass_pwoff"
#define  GET_BYPASS_PWOFF_ENTRY      "get_bypass_pwoff"
#define  SET_BYPASS_PWUP_ENTRY       "set_bypass_pwup"
#define  GET_BYPASS_PWUP_ENTRY       "get_bypass_pwup"
#define  SET_STD_NIC_ENTRY           "set_std_nic"
#define  GET_STD_NIC_ENTRY           "get_std_nic"
#define  SET_BYPASS_WD_ENTRY         "set_bypass_wd"
#define  GET_BYPASS_WD_ENTRY         "get_bypass_wd"
#define  GET_WD_EXPIRE_TIME_ENTRY    "get_wd_time_expire"
#define  RESET_BYPASS_WD_TIMER_ENTRY "reset_bypass_wd"
#define  SET_TX_ENTRY                "set_tx"
#define  GET_TX_ENTRY                "get_tx"
#define  BYPASS_ENABLE               "on"
#define  BYPASS_DISABLE              "off"
#define  TAP_MODE                    "tap"
#define  BYPASS_MODE                 "bypass"
#define  SET_TAP_ENTRY               "set_tap"
#define  GET_TAP_ENTRY               "get_tap"
#define  GET_TAP_CHANGE_ENTRY        "get_tap_change"
#define  SET_DIS_TAP_ENTRY           "set_dis_tap"
#define  GET_DIS_TAP_ENTRY           "get_dis_tap"
#define  SET_TAP_PWUP_ENTRY          "set_tap_pwup"
#define  GET_TAP_PWUP_ENTRY          "get_tap_pwup"
#define  SET_WD_EXP_MODE_ENTRY       "set_wd_exp_mode"
#define  GET_WD_EXP_MODE_ENTRY       "get_wd_exp_mode"
#define  SET_FORCE_LINK_ENTRY        "set_force_link_on"
#define  GET_FORCE_LINK_ENTRY        "get_force_link_on"

#define  SET_BP_WAIT_AT_PWUP_ENTRY   "set_wait_at_pwup"
#define  GET_BP_WAIT_AT_PWUP_ENTRY   "get_wait_at_pwup"
#define  SET_BP_HW_RESET_ENTRY       "set_hw_reset"
#define  GET_BP_HW_RESET_ENTRY       "get_hw_reset"

#define  SET_DISC_PORT_ENTRY         "set_disc_port"
#define  GET_DISC_PORT_ENTRY         "get_disc_port"
#define  SET_DISC_PORT_PWUP_ENTRY    "set_disc_port_pwup"
#define  GET_DISC_PORT_PWUP_ENTRY    "get_disc_port_pwup"





#define  BYPASS_ENABLE                "on"
#define  BYPASS_DISABLE               "off"
#define  IF_NAME                      "eth"
#define  ALL_NAME                     "all "


#define IF_NAME "eth"

struct bp_cap {
    int   flag;
    char *desc;
} bp_cap, *pbp_cap;



struct bp_cap bp_cap_array[]={ 
    {BP_CAP                , BP_CAP_MSG},
    {BP_STATUS_CAP         , BP_STATUS_CAP_MSG},
    {BP_STATUS_CHANGE_CAP  , BP_STATUS_CHANGE_CAP_MSG},
    {SW_CTL_CAP            , SW_CTL_CAP_MSG},
    {BP_DIS_CAP            , BP_DIS_CAP_MSG},
    {BP_DIS_STATUS_CAP     , BP_DIS_STATUS_CAP_MSG},
    {STD_NIC_CAP           , STD_NIC_CAP_MSG},
    {BP_PWOFF_ON_CAP       , BP_PWOFF_ON_CAP_MSG},
    {BP_PWOFF_OFF_CAP      , BP_PWOFF_OFF_CAP_MSG},
    {BP_PWOFF_CTL_CAP      , BP_PWOFF_CTL_CAP_MSG},
    {BP_PWUP_ON_CAP        , BP_PWUP_ON_CAP_MSG},
    {BP_PWUP_OFF_CAP       , BP_PWUP_OFF_CAP_MSG},
    {BP_PWUP_CTL_CAP       , BP_PWUP_CTL_CAP_MSG},
    {WD_CTL_CAP            , WD_CTL_CAP_MSG},
    {WD_STATUS_CAP         , WD_STATUS_CAP_MSG},
    {WD_TIMEOUT_CAP        , WD_TIMEOUT_CAP_MSG},
    {TX_CTL_CAP            , TX_CTL_CAP_MSG},
    {TX_STATUS_CAP         , TX_STATUS_CAP_MSG},
    {TAP_CAP               , TAP_CAP_MSG},
    {TAP_STATUS_CAP        , TAP_STATUS_CAP_MSG},
    {TAP_STATUS_CHANGE_CAP , TAP_STATUS_CHANGE_CAP_MSG},
    {TAP_DIS_CAP           , TAP_DIS_CAP_MSG},
    {TAP_DIS_STATUS_CAP    , TAP_DIS_STATUS_CAP_MSG},
    {TAP_PWUP_ON_CAP       , TAP_PWUP_ON_CAP_MSG},
    {TAP_PWUP_OFF_CAP      , TAP_PWUP_OFF_CAP_MSG},
    {TAP_PWUP_CTL_CAP      , TAP_PWUP_CTL_CAP_MSG}, 
    {NIC_CAP_NEG           , NIC_CAP_NEG_MSG},
    {TPL_CAP               , TPL_CAP_MSG},
    {DISC_CAP               , DISC_CAP_MSG},
    {DISC_DIS_CAP           , DISC_DIS_CAP_MSG},
    {DISC_PWUP_CTL_CAP      , DISC_PWUP_CTL_CAP_MSG}, 

    {0,NULL}
} ;            

 




#endif                          




























