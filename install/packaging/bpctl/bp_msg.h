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

#ifndef _BP_MSG_H
#define _BP_MSG_H

#define HELP_ENTRY                   "help"
#define INFO_ENTRY                   "info"

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
#define  DISC_MODE                 "disc"
#define  SET_TAP_ENTRY               "set_tap"
#define  GET_TAP_ENTRY               "get_tap"
#define  SET_FORCE_LINK_ENTRY        "set_force_link_on"
#define  GET_FORCE_LINK_ENTRY        "get_force_link_on"


#define  SET_HW_RESET_ENTRY          "set_hw_reset"
#define  GET_HW_RESET_ENTRY          "get_hw_reset"

#define  SET_WAIT_AT_PWUP_ENTRY      "set_wait_at_pwup"
#define  GET_WAIT_AT_PWUP_ENTRY      "get_wait_at_pwup"


#define  GET_TAP_CHANGE_ENTRY        "get_tap_change"
#define  SET_DIS_TAP_ENTRY           "set_dis_tap"
#define  GET_DIS_TAP_ENTRY           "get_dis_tap"
#define  SET_TAP_PWUP_ENTRY          "set_tap_pwup"
#define  GET_TAP_PWUP_ENTRY          "get_tap_pwup"
#define  SET_DISC_ENTRY               "set_disc"
#define  GET_DISC_ENTRY               "get_disc"
#define  GET_DISC_CHANGE_ENTRY        "get_disc_change"
#define  SET_DIS_DISC_ENTRY           "set_dis_disc"
#define  GET_DIS_DISC_ENTRY           "get_dis_disc"
#define  SET_DISC_PWUP_ENTRY          "set_disc_pwup"
#define  GET_DISC_PWUP_ENTRY          "get_disc_pwup"
#define  SET_WD_EXP_MODE_ENTRY       "set_wd_exp_mode"
#define  GET_WD_EXP_MODE_ENTRY       "get_wd_exp_mode"
#define  SET_WD_AUTORESET_ENTRY      "set_wd_autoreset"
#define  GET_WD_AUTORESET_ENTRY      "get_wd_autoreset"
#define  GET_BYPASS_INFO_ENTRY       "get_bypass_info"

#ifdef BP_SELF_TEST
#define  SET_BP_SELF_TEST_ENTRY      "set_bst"
#define  GET_BP_SELF_TEST_ENTRY      "get_bst"
#endif

#define IF_NAME "eth"

/********MESSAGES*************/
#define NO_BPT_DEV_MSG            "No Bypass/TAP control devices were found\n" 
#define NOT_SUPP_MSG              "The interface is not capable of the operation.\n"
#define NOT_SUPP_BP_MSG           "The interface doesn't support Bypass.\n"
#define NOT_SUPP_BPT_MSG          "The interface doesn't support Bypass/TAP.\n"
#define SLAVE_IF_MSG              "The interface is a slave interface.\n"
#define MASTER_IF_MSG             "The interface is a control interface.\n"
#define NOT_SUPP_BPT_SLAVE_MSG    "The interface is a slave interface or doesn't support Bypass/TAP.\n"
#define NOT_SUPP_BP_SLAVE_MSG     "The interface is a slave interface or doesn't support Bypass.\n"
#define NOT_SUPP_BP_SLAVE_UN_MSG  "The interface is a slave interface or doesn't support Bypass or Bypass state is unknown.\n"
#define NOT_SUPP_TAP_SLAVE_MSG    "The interface is a slave interface or doesn't support TAP.\n"
#define NOT_SUPP_SLAVE_MSG        "The interface is a slave interface or doesn't support this feature.\n"
#define BP_PAIR_MSG               "%s is a slave port.\n"
#define SUCCESS_MSG               "completed successfully.\n"
#define BP_MODE_MSG               "The interface is in the Bypass mode.\n"
#define NON_BP_MODE_MSG           "The interface is in the non-Bypass mode.\n"
#define TAP_MODE_MSG              "The interface is in the TAP mode.\n"
#define DISC_MODE_MSG             "The interface is in the Disconnect mode.\n"
#define DISC_PORT_MODE_MSG        "The interface is in the Disconnect Port mode.\n"

#define NON_TAP_MODE_MSG          "The interface is in the non-TAP mode.\n"
#define NON_DISC_MODE_MSG         "The interface is in the non-Disconnect mode.\n"
#define NON_DISC_PORT_MODE_MSG    "The interface is in the non-Disconnect Port mode.\n"
#define NORMAL_MODE_MSG           "The interface is in the Normal mode.\n"
#define TAP_BP_MODE_EN_MSG        "Bypass/TAP mode is enabled.\n"
#define NORMAL_MODE_LAST_MSG      "There was no change to bypass/tap/disc from last read of the status.\n"
#define NON_BYPASS_MODE_LAST_MSG      "There was no change to bypass from last read of the status.\n"
#define NON_TAP_MODE_LAST_MSG         "There was no change to tap from last read of the status.\n"
#define NON_DISC_MODE_LAST_MSG        "There was no change to disconnect from last read of the status.\n"

#define TAP_MODE_LAST_MSG         "There was a change to TAP state or it's now in TAP state.\n"
//#define NON_TAP_MODE_LAST_MSG     "There was a change to non-TAP state or it's now in non-TAP state.\n"

#define DISC_MODE_LAST_MSG         "There was a change to Disconnect state or it's now in Disconnect state.\n"
//#define NON_DISC_MODE_LAST_MSG     "There was a change to non-Disconnect state or it's now in non-Disconnect state.\n"
#define BP_MODE_LAST_MSG          "There was a change to Bypass state or it's now in Bypass state.\n"
#define BP_MODE_EN_MSG            "Bypass mode is enabled.\n"
#define BP_MODE_DIS_MSG           "Bypass mode is disabled.\n"
#define TAP_MODE_EN_MSG           "TAP mode is enabled.\n"
#define TAP_MODE_DIS_MSG          "TAP mode is disabled.\n"
#define DISC_MODE_EN_MSG           "Disconnect mode is enabled.\n"
#define DISC_MODE_DIS_MSG          "Disconnect mode is disabled.\n"
#define BP_DIS_PWOFF_MSG          "The interface is in the non-Bypass mode at power off state.\n"
#define BP_EN_PWOFF_MSG           "The interface is in the Bypass mode at power off state.\n"
#define BP_DIS_PWUP_MSG           "The interface is in the non-Bypass mode at power up state.\n"
#define BP_EN_PWUP_MSG            "The interface is in the Bypass mode at power up state.\n"
#define TAP_EN_PWUP_MSG           "The interface is in the TAP mode at power up state.\n"
#define TAP_DIS_PWUP_MSG          "The interface is in the non-TAP mode at power up state.\n"
#define DISC_EN_PWUP_MSG           "The interface is in the Disconnect mode at power up state.\n"
#define DISC_DIS_PWUP_MSG          "The interface is in the non-Disconnect mode at power up state.\n"
#define DISC_PORT_EN_PWUP_MSG      "The interface is in the Disconnect Port mode at power up state.\n"
#define DISC_PORT_DIS_PWUP_MSG    "The interface is in the non-Disconnect Port mode at power up state.\n"

#define WDT_STATE_MSG             "WDT is enabled with %d ms timeout value.\n"
#define WD_DIS_MSG                "WDT is disabled.\n" 
#define NOT_WD_DIS_MSG            "WDT is disabled or WDT state is unknown.\n" 
#define WD_STATE_EXT_MSG          "WDT is enabled with %d ms timeout value.\n"
#define WD_TIME_LEFT_MSG          "WDT is enabled; %d ms time left till WDT expired.\n"
#define WD_STATE_UNKNOWN_MSG      "WDT state is unknown.\n" 
#define NOT_NET_DEV_MSG           "%s does not appear to be a Silicom Bypass device. Must be "IF_NAME"<if_num>.\n"
#define NOT_WDT_SLAVE_MSG         "The interface is a slave interface or WDT state is unknown.\n"
#define WDT_EXP_MSG               "WDT has expired.\n"
#define STD_NIC_MODE_MSG          "The interface is in Standard NIC mode.\n"
#define NON_STD_NIC_MODE_MSG      "The interface is not in Standard NIC mode.\n"
#define TX_EN_MSG                 "Transmit is enable.\n"
#define TX_DIS_MSG                "Transmit is disable.\n"
#define TPL_EN_MSG                 "TPL is enable.\n"
#define TPL_DIS_MSG                "TPL is disable.\n"
#define TAP_WD_EXP_MODE_MSG       "When WDT timeout occurs, the interface will be in the TAP mode.\n"
#define BYPASS_WD_EXP_MODE_MSG    "When WDT timeout occurs, the interface will be in the Bypass mode.\n"
#define BP_WAIT_AT_PWUP_EN_MSG    "wait_at_pwup is enable.\n"
#define BP_WAIT_AT_PWUP_DIS_MSG    "wait_at_pwup is disable.\n"
#define BP_HW_RESET_EN_MSG         "hw_reset is enable.\n"
#define BP_HW_RESET_DIS_MSG        "hw_reset is disable.\n"
#define FORCE_LINK_EN_MSG         "Force link is enable.\n"
#define FORCE_LINK_DIS_MSG        "Force link is disable.\n"

#define DISC_WD_EXP_MODE_MSG       "When WDT timeout occurs, the interface will be in the Disconnect mode.\n"
#define NOT_DEV_MSG               "No such device.\n"
#define GET_BPSLAVE_FAIL_MSG      "Get Bypass slave failed.\n" 
#define SET_BP_FAIL_MSG           "Set Bypass failed.\n" 
#define GET_BP_FAIL_MSG           "Get Bypass failed.\n" 
#define GET_BP_CHANGE_FAIL_MSG    "Get Bypass change failed.\n" 
#define SET_DIS_BP_FAIL_MSG       "Set disable Bypass failed.\n" 
#define GET_DIS_BP_FAIL_MSG       "Get disable Bypass failed.\n"
#define SET_BP_PWOFF_FAIL_MSG     "Set Bypass/Normal mode on Power Off failed.\n" 
#define GET_BP_PWOFF_FAIL_MSG     "Get Bypass/Normal mode on Power Off failed.\n"
#define SET_BP_PWUP_FAIL_MSG      "Set Bypass/Normal mode on Power Up failed.\n" 
#define GET_BP_PWUP_FAIL_MSG      "Get Bypass/Normal mode on Power Up failed.\n"
#define SET_BP_WD_FAIL_MSG        "Set Bypass WD failed.\n"
#define GET_BP_WD_FAIL_MSG        "Get Bypass WD failed.\n"
#define RESET_BP_WD_FAIL_MSG      "Reset Bypass WD failed\n"
#define SET_STD_NIC_FAIL_MSG      "Set Standard NIC mode failed\n"
#define GET_TAP_PWUP_FAIL_MSG     "Get TAP mode on power up failed\n"
#define GET_TAP_FAIL_MSG          "Get TAP mode failed\n"
#define GET_TAP_CHANGE_FAIL_MSG   "Get TAP change failed.\n" 
#define GET_DIS_TAP_FAIL_MSG      "Get disable TAP failed.\n"

#define GET_DISC_PWUP_FAIL_MSG     "Get Disconnect mode on power up failed\n"
#define GET_DISC_FAIL_MSG          "Get Disconnect mode failed\n"

#define GET_DISC_PORT_PWUP_FAIL_MSG     "Get Disconnect Port mode on power up failed\n"
#define GET_DISC_PORT_FAIL_MSG          "Get Disconnect Port mode failed\n"

#define GET_DISC_CHANGE_FAIL_MSG   "Get Disconnect change failed.\n" 
#define GET_DIS_DISC_FAIL_MSG      "Get disable Disconnect failed.\n"

#define WD_AUTORESET_STATE_MSG    "WDT autoreset is enabled with %d ms period.\n"
#define WD_AUTORESET_DIS_MSG      "WDT autoreset is disabled.\n"

#ifdef BP_SELF_TEST
#define BP_SELF_TEST_MSG          "Bypass Self Test is enabled with %d ms period.\n"
#define BP_SELF_TEST_DIS_MSG      "Bypass Self Test is disabled.\n"
#endif


#define NOT_SUPP_TXCTL_MSG       "The interface doesn't support TX Control feature.\n"
#define NOT_SUPP_TXCTL_SLAVE_MSG "The interface is a slave interface.\n"
#define NO_TXCTL_DEV_MSG         "No TX control devices were found\n" 
#define TXCTL_FAIL_MSG           "Set Transmit mode failed.\n" 
#define TXCTL_OK_MSG             "Set Transmit mode completed successfully.\n"
#define GET_DEV_NUM_FAIL_MSG     "Get TX Control devices number failed\n" 
#define GET_TXCTL_FAIL_MSG       "Get Transmit mode failed.\n" 

#define NO_BPCTL_DEV_MSG         "No Bypass-SD/TAP-SD control devices were found\n" 
#define NOT_SUPP_BPCTL_MSG       "The interface is not Bypass-SD/TAP-SD device.\n"
#define NO_BP_DEV_MSG            "No Bypass-SD control devices were found\n"




#define BP_CAP_MSG               "\nBP_CAP               The interface is Bypass/TAP capable in general.\n"
#define BP_STATUS_CAP_MSG        "\nBP_STATUS_CAP        The interface can report of the current Bypass mode.\n"
#define BP_STATUS_CHANGE_CAP_MSG "\nBP_STATUS_CHANGE_CAP The interface can report on a change to bypass mode\n" \
                                 "                     from the last time the mode was defined\n"
#define SW_CTL_CAP_MSG           "\nSW_CTL_CAP           The interface is Software controlled capable for\n"     \
                                 "                     bypass/non bypass/TAP/non TAP modes.\n"
#define BP_DIS_CAP_MSG           "\nBP_DIS_CAP           The interface is capable of disabling the Bypass mode\n" \
                                 "                     at all times. This mode will retain its mode even\n" \
                                 "                     during power loss and also after power recovery. This\n" \
                                 "                     will overcome on any bypass operation due to watchdog\n" \
                                 "                     timeout or set bypass command.\n"
#define BP_DIS_STATUS_CAP_MSG    "\nBP_DIS_STATUS_CAP    The interface can report of the current DIS_BP_CAP.\n"
#define STD_NIC_CAP_MSG          "\nSTD_NIC_CAP          The interface is capable to be configured to operate\n" \
                                 "                     as standard, non Bypass, NIC interface (have direct\n" \
                                 "                     connection to interfaces at all power modes).\n"
#define BP_PWOFF_ON_CAP_MSG      "\nBP_PWOFF_ON_CAP      The interface can be in Bypass mode at power off state.\n"
#define BP_PWOFF_OFF_CAP_MSG     "\nBP_PWOFF_OFF_CAP     The interface can disconnect the Bypass mode at\n" \
                                 "                     power off state without effecting all the other\n" \
                                 "                     states of operation.\n"
#define BP_PWOFF_CTL_CAP_MSG     "\nBP_PWOFF_CTL_CAP     The behavior of the Bypass mode at Power-off state\n" \
                                 "                     can be controlled by software without effecting any\n" \
                                 "                     other state.\n"
#define BP_PWUP_ON_CAP_MSG       "\nBP_PWUP_ON_CAP       The interface can be in Bypass mode when power is\n" \
                                 "                     turned on (until the system take control of the\n" \
                                 "                     bypass functionality).\n"
#define BP_PWUP_OFF_CAP_MSG      "\nBP_PWUP_OFF_CAP      The interface can disconnect from Bypass mode when\n" \
                                 "                     power is turned on (until the system take control\n" \
                                 "                     of the bypass functionality).\n"
#define BP_PWUP_CTL_CAP_MSG      "\nBP_PWUP_CTL_CAP      The behavior of the Bypass mode at Power-up can be\n" \
                                 "                     controlled by software\n"
#define WD_CTL_CAP_MSG           "\nWD_CTL_CAP           The interface has watchdog capabilities to turn to\n" \
                                 "                     Bypass mode when not reset for defined period of time.\n"
#define WD_STATUS_CAP_MSG        "\nWD_STATUS_CAP        The interface can report on the watchdog status\n" \
                                 "                     (active/inactive)\n"
#define WD_TIMEOUT_CAP_MSG       "\nWD_TIMEOUT_CAP       The interface can report the time left till watchdog\n"   \
                                 "                     triggers to Bypass mode.\n"
                                 
#define WD_MIN_TIME_MSG	         "\nWD_MIN_TIME = %d        The interface WD minimal time period (ms).\n\n"
#define WD_STEP_TIME_MSG	     "\nWD_STEP_TIME = %d         The steps of the WD timer in\n" \
                                 "                          0 - for linear steps (WD_MIN_TIME * X)\n" \
                                 "                          1 - for multiply by 2 from previous step (WD_MIN_TIME * 2^X).\n" 
#define WD_STEP_COUNT_MSG        "\nWD_STEP_COUNT_MSG = %d  Number of bits available for defining the number of WDT steps.\n" \
                                 "                          From that the number of steps the WDT will have is 2^Y\n" \
                                 "                          (Y number of bits available for defining the value).\n\n"
                                 
#define TX_CTL_CAP_MSG           "\nTX_CTL_CAP           The interface is capable to control PHY transmitter on and off.\n"
#define TX_STATUS_CAP_MSG        "\nTX_STATUS_CAP        The interface can report the status of the PHY transmitter.\n"              
#define TAP_CAP_MSG              "\nTAP_CAP              The interface is TAP capable in general.\n"
#define TAP_STATUS_CAP_MSG       "\nTAP_STATUS_CAP       The interface can report of the current TAP mode.\n"
#define TAP_STATUS_CHANGE_CAP_MSG "\nTAP_STATUS_CHANGE_CAP The interface can report on a change to TAP mode\n" \
                                  "                      from the last time the mode was defined\n"
#define TAP_DIS_CAP_MSG          "\nTAP_DIS_CAP          The interface is capable of disabling the TAP mode\n" \
                                 "                     at all times. This mode will retain its mode even\n" \
                                 "                     during power loss and also after power recovery. This\n" \
                                 "                     will overcome on any TAP operation due to watchdog\n" \
                                 "                     timeout or set TAP command.\n"
#define TAP_DIS_STATUS_CAP_MSG   "\nTAP_DIS_STATUS_CAP   The interface can report of the current DIS_TAP_CAP.\n"
#define TAP_PWUP_ON_CAP_MSG      "\nTAP_PWUP_ON_CAP      The interface can be in TAP mode when power is\n" \
                                 "                     turned on (until the system take control of the\n" \
                                 "                     TAP functionality).\n"
#define TAP_PWUP_OFF_CAP_MSG     "\nTAP_PWUP_OFF_CAP     The interface can disconnect from TAP mode when\n" \
                                 "                     power is turned on (until the system take control\n" \
                                 "                     of the TAP functionality).\n"
#define TAP_PWUP_CTL_CAP_MSG     "\nTAP_PWUP_CTL_CAP     The behavior of the TAP mode at Power-up can be\n" \
                                 "                     controlled by software\n" 
#define NIC_CAP_NEG_MSG          "\nNIC_CAP_NEG          The interface is NIC capable in general - \n" \
                                 "                     negative polarity (0 - NIC capable)\n"   
                                                                 
#define TPL_CAP_MSG              "\nTPL_CAP              The interface is TPL capable in general.\n"
#define DISC_CAP_MSG             "\nDISC_CAP             The interface is Disconnect capable in general.\n"
#define DISC_STATUS_CAP_MSG       "\nDISC_STATUS_CAP       The interface can report of the current Disconnect mode.\n"
#if 0
#define DISC_STATUS_CHANGE_CAP_MSG "\nDISC_STATUS_CHANGE_CAP The interface can report on a change to Disconnect \n" \
                                  "                      mode from the last time the mode was defined\n"
#endif                                  
#define DISC_DIS_CAP_MSG          "\nDISC_DIS_CAP         The interface is capable of disabling the Disconnect mode\n" \
                                 "                     at all times. This mode will retain its mode even\n" \
                                 "                     during power loss and also after power recovery. This\n" \
                                 "                     will overcome on any Disconnect operation due to watchdog\n" \
                                 "                     timeout or set Disconnect command.\n"
#if 0
#define DISC_DIS_STATUS_CAP_MSG   "\nDISC_DIS_STATUS_CAP   The interface can report of the current DIS_DISC_CAP.\n"
#define DISC_PWUP_ON_CAP_MSG      "\nDISC_PWUP_ON_CAP      The interface can be in Disconnect mode when power is\n" \
                                 "                     turned on (until the system take control of the\n" \
                                 "                     Disconnect functionality).\n"
#define DISC_PWUP_OFF_CAP_MSG     "\nDISC_PWUP_OFF_CAP     The interface can disconnect from Disconnect mode when\n" \
                                 "                     power is turned on (until the system take control\n" \
                                 "                     of the Disconnect functionality).\n"
#endif                                 
#define DISC_PWUP_CTL_CAP_MSG     "\nDISC_PWUP_CTL_CAP    The behavior of the Disconnect mode at Power-up can be\n" \
                                 "                     controlled by software\n" 


#endif
