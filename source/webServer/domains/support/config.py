# # # #
# config.py
#
# University of Illinois/NCSA Open Source License
# Copyright (c) 2015 Information Trust Institute
# All rights reserved.
#
# Developed by:
#
# Information Trust Institute
# University of Illinois
# http://www.iti.illinois.edu
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal with
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to do
# so, subject to the following conditions:
#
# Redistributions of source code must retain the above copyright notice, this list
# of conditions and the following disclaimers. Redistributions in binary form must
# reproduce the above copyright notice, this list of conditions and the following
# disclaimers in the documentation and/or other materials provided with the
# distribution.
#
# Neither the names of Information Trust Institute, University of Illinois, nor
# the names of its contributors may be used to endorse or promote products derived
# from this Software without specific prior written permission.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS
# OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
#
# # # # #

import domains.support.lib.common as comLib
import domains.support.network as netLib
import re
import os.path
import threading
from string import Template

# These get called a lot, get them once when read in to prevent lots of file reads
ARMORE_CFG_FILE = comLib.getSupportFilePath("ArmoreConfig")
PROXY_CFG_FILE_BACKEND = comLib.getSupportFilePath("ProxyConfigBackend")
PROXY_CFG_FILE = comLib.getSupportFilePath("ProxyConfig")
TRANSPARENT_CFG_FILE = comLib.getSupportFilePath("TransparentConfig")
PASSIVE_CFG_FILE = comLib.getSupportFilePath("PassiveConfig")

PROXY_NETWORK_FILE = comLib.getSupportFilePath("ProxyNetworkTemplate")
PASSIVE_NETWORK_FILE = comLib.getSupportFilePath("PassiveNetworkTemplate")
TRANSPARENT_NETWORK_FILE = comLib.getSupportFilePath("TransparentNetworkTemplate")

PROXY_CFG_BACKEND_TEMPLATE = comLib.getSupportFilePath("ProxyConfigBackendTemplate")
PASSIVE_CFG_BACKEND_TEMPLATE = comLib.getSupportFilePath("PassiveConfigBackendTemplate")
TRANSPARENT_CFG_BACKEND_TEMPLATE = comLib.getSupportFilePath("TransparentConfigBackendTemplate")

cfgFiles = {
    'armore': ARMORE_CFG_FILE,
    'proxy': PROXY_CFG_FILE,
    'transparent': TRANSPARENT_CFG_FILE,
    'passive': PASSIVE_CFG_FILE
}

netFiles = {
    'proxy': PROXY_NETWORK_FILE,
    'transparent': TRANSPARENT_NETWORK_FILE,
    'passive': PASSIVE_NETWORK_FILE
}

def restartProxy():
    comLib.cmd('/etc/init.d/armoreconfig stop')
    comLib.cmd('/etc/init.d/armoreconfig start')
    #comLib.cmd('broctl restart')

def restartBro():
    comLib.cmd('broctl restart')

# Gets configuration from specified file as key value pair
def getConfigFromFile(theFile):

    ret = {}

    if os.path.exists(theFile):	
        configFile = open(theFile, 'r')
        for l in configFile.readlines():
            if len(l.split('=')) < 2:
                continue
            key, value = [re.sub(r'"', "", x.strip()) for x in l.rstrip().split('=')]
            ret[key] = value

    return ret

# Synchronizes the webServer config files and the backend config files
def synchConfigs():

    armoreBackConf = getArmoreConfigBackend()

    if "Operation" not in armoreBackConf:
        return

    mode = armoreBackConf.get("Operation")

    aConf = {}
    aConf["managementIp"] = armoreBackConf.get("ManagementIp")
    aConf["managementInterface"] = armoreBackConf.get("ManagementInt")
    aConf["managementMask"] = armoreBackConf.get("ManagementMask")
    aConf["operationMode"] = mode

    mConf = {}
    if mode == "Proxy":
        mConf["roleType"] = armoreBackConf.get("Roletype")
        mConf["port"] = armoreBackConf.get("Port")
        mConf["bind"] = armoreBackConf.get("Bind")
        mConf["capture"] = armoreBackConf.get("Capture")
        mConf["encryption"] = armoreBackConf.get("Encryption")
        mConf["firewall"] = armoreBackConf.get("Firewall")
        mConf["interface"] = armoreBackConf.get("Interface")
        mConf["operationMode"] = "Proxy"

    if mode == "Passive":

        mConf["operationMode"] = "Passive"
        mConf["monitored_interface"] = armoreBackConf.get("Interface")

        aConf["internalInterface"] = mConf.get("monitored_interface")
        aConf["internalIp"] = netLib.getIpAddrFromInt(mConf.get("monitored_interface"))

    if mode == "Transparent":
        mConf["Operation"] = "Transparent"
        mConf["interface1"] = armoreBackConf.get("Interface1") 
        mConf["interface2"] = armoreBackConf.get("Interface2") 
        mConf["bridgeIp"] = armoreBackConf.get("Bind") 
        mConf["broadcastIp"] = armoreBackConf.get("BroadcastIp") 
        mConf["netmask"] = armoreBackConf.get("Netmask") 
        mConf["gateway"] = armoreBackConf.get("Gateway") 
        mConf["route"] = armoreBackConf.get("Route") 

    updateConfig(mConf, "mode")
    updateConfig(aConf, "armore")

    maConf = mConf.copy()
    maConf.update(aConf)
    updateConfig(maConf, "backend")

    return aConf

# Get management interface information
def getMgmtConfig():
    configAll = getArmoreConfigBackend()
    config = {key: configAll[key] for key in configAll if re.match(".*Management.*", key)}

    return config

def getArmoreConfigBackend():
    return getConfigFromFile(PROXY_CFG_FILE_BACKEND)

def getArmoreConfig():
    return getConfigFromFile(ARMORE_CFG_FILE)
    
def getProxyConfig():
    return getConfigFromFile(PROXY_CFG_FILE)

def getTransparentConfig():
    return getConfigFromFile(TRANSPARENT_CFG_FILE)

def getPassiveConfig():
    return getConfigFromFile(PASSIVE_CFG_FILE)

# Generic function for getting a configuration
def getConfig(mode):
    if mode.lower() == "proxy":
        return getProxyConfig()
    elif mode.lower() == "passive":
        return getPassiveConfig()
    elif mode.lower() == "transparent":
        return getTransparentConfig()
    else:
        return getArmoreConfig()

# Write configuration to a template and output file
def writeTemplate(inp, tempFile, outFile):
    t = Template(open(tempFile, 'r').read())
    o = t.substitute(inp)
    with open(outFile, 'w') as f:
        f.write(o)

# Write configuration information to the configuration file
def writeConfig(config, theType):

    #aConf = Template(open(comLib.getSupportFilePath("ArmoreConfigTemplate"), 'r').read())

    confFileTemp = None
    confFileToWrite = None
    if theType == "armore":
        confFileTemp = comLib.getSupportFilePath("ArmoreConfigTemplate")
        confFileToWrite = ARMORE_CFG_FILE
    elif config.get("operationMode") == "Proxy":
        confFileTemp = comLib.getSupportFilePath("ProxyConfigTemplate")
        confFileToWrite = PROXY_CFG_FILE
    elif config.get("operationMode") == "Passive":
        confFileTemp = comLib.getSupportFilePath("PassiveConfigTemplate")
        confFileToWrite = PASSIVE_CFG_FILE
    elif config.get("operationMode") == "Transparent":
        confFileTemp = comLib.getSupportFilePath("TransparentConfigTemplate")
        confFileToWrite = TRANSPARENT_CFG_FILE
    else:
        print("# Unable to write config for '{}' mode".format(config.get("operationMode")))

    if confFileTemp:
        writeTemplate(config, confFileTemp, confFileToWrite)

def updateProxyConfig(config):
    oldConfig = getConfig("proxy")
    newConfig = {
        "roleType": config.get("networkRole") or config.get("roleType") or oldConfig.get("RoleType"),
        "port": config.get("targetPort") or config.get("port") or oldConfig.get("Port"),
        "bind": config.get("targetIp") or config.get("bind") or oldConfig.get("Bind"),
        "capture": config.get("captureMode") or config.get("capture") or oldConfig.get("Capture"),
        "encryption": config.get("encryption") or config.get("encryption") or oldConfig.get("Encryption"),
        "firewall": config.get("firewall") or config.get("firewall") or oldConfig.get("Firewall"),
        "interface": config.get("monIntProxy") or config.get("interface") or oldConfig.get("Interface"),
        "operationMode": "Proxy",
    }

    return newConfig

def updatePassiveConfig(config):
    oldConfig = getPassiveConfig()
    newConfig = {
        "monitored_interface": config.get("monIntPsv") or config.get("monitored_interface") or oldConfig.get("Monitored_Interface"),
        "operationMode": "Passive",
    }

    return newConfig
    
def updateTransparentConfig(config):
    oldConfig = getTransparentConfig()
    newConfig = {
        "netmask": config.get("netmask") or oldConfig.get("Netmask"),
        "broadcastIp": config.get("broadcastIp") or oldConfig.get("BroadcastIp"),
        "bridgeIp": config.get("bridgeIp") or oldConfig.get("BridgeIp"),
        "interface1": config.get("brdgInt1") or config.get("interface1") or oldConfig.get("Interface1"),
        "interface2": config.get("brdgInt2") or config.get("interface2") or oldConfig.get("Interface2"),
        "gateway": config.get("gateway") or oldConfig.get("Gateway"),
        "route": config.get("route") or oldConfig.get("Route"),
        "operationMode": "Transparent"
    }

    return newConfig

def updateArmoreConfig(config):
    oldConfig = getArmoreConfig()
    internalInt = config.get("intInt") or oldConfig.get("Internal_Interface")
    externalInt = config.get("extInt") or oldConfig.get("External_Interface")
    newConfig = {
        "operationMode": config.get("operationMode") or config.get("Operation") or oldConfig.get("Operation"),
        "managementIp": config.get("mgtIp") or config.get("managementIp") or oldConfig.get("Management_IP"),
        "managementMask": config.get("mgtMsk") or config.get("managementMask") or  oldConfig.get("Management_Mask"),
        "managementInterface": config.get("mgtInt") or config.get("managementInterface") or oldConfig.get("Management_Interface"),
        "internalInterface": internalInt,
        "internalIp": config.get("intIp") or oldConfig.get("Internal_IP") or netLib.getIpAddrFromInt(internalInt),
        "internalMask": config.get("intMsk") or oldConfig.get("Internal_Mask") or netLib.getNetmaskFromInt(internalInt),
        "externalInterface": externalInt,
        "externalIp": config.get("extIp") or oldConfig.get("External_IP") or netLib.getIpAddrFromInt(externalInt),
        "externalMask": config.get("extMsk") or oldConfig.get("External_Mask") or netLib.getNetmaskFromInt(externalInt),
    }

    restartFlask = oldConfig.get("Management_IP") != newConfig.get("managementIp")

    return newConfig, restartFlask

# Writes the backend configuration file
def writeBackendConfig(config):

    mode = config.get("operationMode")
    tempToRead = None
    if mode == "Proxy":
        tempToRead = PROXY_CFG_BACKEND_TEMPLATE
    elif mode == "Passive":
        tempToRead = PASSIVE_CFG_BACKEND_TEMPLATE
    elif mode == "Transparent":
        tempToRead = TRANSPARENT_CFG_BACKEND_TEMPLATE

    writeTemplate(config, tempToRead, PROXY_CFG_FILE_BACKEND) 

# Changes existing settings to newly submitted values
def updateConfig(config, confType, enforce=False):
    mode = config.get("operationMode") or config.get("Operation")
    newConfig = None

    restartFlask = False
    if confType == "mode":
        if mode.lower() == "proxy":
            newConfig = updateProxyConfig(config)
        elif mode.lower() == "passive":
            newConfig = updatePassiveConfig(config)
        elif mode.lower() == "transparent":
            newConfig = updateTransparentConfig(config)

        writeConfig(newConfig, confType)

        if enforce:
            newArmoreConfig, restartFlask = updateArmoreConfig(config)
            writeConfig(newArmoreConfig, "armore")

            amConf = newConfig.copy()
            amConf.update(newArmoreConfig)
            writeBackendConfig(amConf)

            print("# Restarting ARMORE service...")
            writeInterfacesFile(amConf, mode.lower())
            restartProxy()

    elif confType == "armore":
        newArmoreConfig, restartFlask = updateArmoreConfig(config)
        writeConfig(newArmoreConfig, confType)
    elif confType == "backend":
        writeBackendConfig(config)
    
    return False

def writeArmoreConfig(config):
    oldConfig = getConfigFromFile(ARMORE_CFG_FILE)
    newConfigStr = "#!/usr/bin/env bash\n"
    restartFlask = False
    for key in config:
        if type(config[key]) is dict:
            for k in config[key]:
                newConfigStr += "{0}_{1}=\"{2}\"\n".format(key, k, config[key][k])
        elif type(config[key]) is str:
            newConfigStr += "{0}=\"{1}\"\n".format(key, config[key])
    newConfigFile = open(ARMORE_CFG_FILE, 'w')
    newConfigFile.write(newConfigStr)
    newConfigFile.close()
    if bool(oldConfig) and oldConfig["Management_IP"] != config["Management"]["IP"]:
        restartFlask = True
    
    return restartFlask

def writeProxyConfig(config):
    oldConfig = getProxyConfig()
    oldConfigFile = open(PROXY_CFG_FILE, 'r')
    headerLines = oldConfigFile.readlines()[:3]
    oldVals = oldConfigFile.readlines()[3:]
    oldConfigFile.close()

    newConfigFile = open(PROXY_CFG_FILE, 'w')

    for l in headerLines:
        newConfigFile.write(l)

    newConfigFile.write('\n')
    newConfig = {}

    #for c in oldConfig:
    #    newConfig[c] = oldConfig[c]

    for c in config:
        if config[c]:
            newConfig[c] = config[c]

    for c in newConfig:
        newConfigFile.write("{0} = {1}\n".format(c, newConfig[c]))

    newConfigFile.close()
    restartProxy()

def enforceArmoreConfig():
    config = getArmoreConfig()
    if not config:
        return 0
    ipAddrs = {}
    intTypes = ["Management", "External", "Internal"]
    for intType in intTypes:
        keyInt = intType + "_Interface"
        keyIp = intType + "_IP"
        if keyInt in config and keyIp in config:
            ipAddrs[config[keyInt]] = config[keyIp]
        else:
            ipAddrs["eth0"] = '127.0.0.1'
    
    netLib.setIps(ipAddrs)

def updateIpAddrs(ipAddrsConfig):
    del ipAddrsConfig["Operation"]
    ipAddrs = {}
    for intType in ipAddrsConfig:
        ipAddrs[ipAddrsConfig[intType]["Interface"]] = ipAddrsConfig[intType]["IP"]
    
    netLib.setIps(ipAddrs)

# Writes template information into /etc/network/interfaces file based on the mode selected
def writeInterfacesFile(config, configType):

    writeTemplate(config, netFiles[configType], "/etc/network/interfaces")
    #comLib.cmd("service networking restart")

# Gets current mode of operation based on backend 
def getOperationMode():
    with open("/etc/armore/armoreconfig", 'r') as f:
        for l in [x.rstrip() for x in f.readlines()]:
            if re.search("Operation", l):
                _, value = l.replace(" ", "").split("=")
                return value

# Called when ARMORE first starts to ensure config files exist and are synched with backend properly
def createArmoreModeConfigFiles(supportFilePath="/var/webServer/supportFiles.txt"):

    configs = [
        "ArmoreConfig",
        "ProxyConfig",
        "TransparentConfig",
        "PassiveConfig"
    ]

    print("# Creating Config Files")
    backendConfig = getArmoreConfigBackend()
    intIpsDict = netLib.getInterfaceIps()
    for c in configs:

        cPath = comLib.getSupportFilePath(c, supportFilePath)
        ctPath = comLib.getSupportFilePath("{}Template".format(c), supportFilePath)

        if not os.path.exists(cPath):

            theDict = {}
            currConfig = getArmoreConfig()

            if c == "ArmoreConfig":
                theDict["managementIp"] = backendConfig.get("ManagementIp")
                theDict["managementMask"] = backendConfig.get("ManagementMask")
                theDict["managementInterface"] = backendConfig.get("ManagementInt")

                intsUsed = [backendConfig.get("ManagementInt")]
                intToUse = ""
                for i in intIpsDict:
                    if i not in intsUsed:
                        intToUse = i
                        intsUsed.append(i)
                        break

                theDict["internalIp"] = intIpsDict[intToUse]
                theDict["internalMask"] = netLib.getNetmaskFromInt(intToUse)
                theDict["internalInterface"] = intToUse

                for i in intIpsDict:
                    if i not in intsUsed:
                        intToUse = i
                        intsUsed.append(i)
                        break

                theDict["externalIp"] = intIpsDict[intToUse]
                theDict["externalMask"] = netLib.getNetmaskFromInt(intToUse)
                theDict["externalInterface"] = intToUse

                theDict["operation"] = getOperationMode()

            elif c == "ProxyConfig":
                if backendConfig.get("Operation") == "Proxy":
                    theDict["roleType"] = backendConfig.get("Roletype")
                    theDict["port"] = backendConfig.get("Port")
                    theDict["bind"] = backendConfig.get("Bind")
                    theDict["capture"] = backendConfig.get("Capture")
                    theDict["encryption"] = backendConfig.get("Encryption")
                    theDict["firewall"] = backendConfig.get("Firewall")
                    theDict["interface"] = backendConfig.get("Interface")
                else:
                    theDict["roleType"] = "Server"
                    theDict["port"] = "5555"
                    theDict["bind"] = "127.0.0.2"
                    theDict["capture"] = "NetMap"
                    theDict["encryption"] = "Enabled"
                    theDict["firewall"] = "Disabled"
                    theDict["interface"] = "eth1"

            elif c == "TransparentConfig":
                theDict["netmask"] = "255.255.255.0"
                theDict["broadcastIp"] = "127.0.0.2"
                theDict["bridgeIp"] = "127.0.0.3"
                theDict["interface1"] = "eth1"
                theDict["interface2"] = "eth2"
                theDict["gateway"] = "127.0.0.1"
                theDict["route"] = "127.0.0.1/8"

            elif c == "PassiveConfig":
                theDict["monitored_interface"] = "eth1"

            writeTemplate(theDict, ctPath, cPath)
            '''
            t = open(ctPath, 'r')
            temp = Template(t.read())
            t.close()
            f = open(cPath, 'w')
            f.write(temp.substitute(theDict))
            f.close()
            '''


