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

import lib.common as comLib
import network as netLib
import re
import os.path
import threading
from string import Template

ARMORE_CFG_FILE = '/etc/armore/configs/armoreConfig.sh'
PROXY_CFG_FILE = '/etc/armore/armoreconfig'
PROXY_CFG_FILE_NEW = '/etc/armore/configs/proxyConfig.sh'
TRANSPARENT_CFG_FILE = '/etc/armore/configs/transparentConfig.sh'
PASSIVE_CFG_FILE = '/etc/armore/configs/passiveConfig.sh'

PROXY_NETWORK_FILE = './static/python/templates/interfaces_proxy'
PASSIVE_NETWORK_FILE = './static/python/templates/interfaces_passive'
TRANSPARENT_NETWORK_FILE = './static/python/templates/interfaces_transparent'

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
    comLib.cmd('broctl restart')

def restartBro():
    comLib.cmd('broctl restart')

def getConfigFromFile(theFile):

    ret = {}

    if os.path.isfile(theFile):	
        configFile = open(theFile, 'r')
        for l in configFile.readlines():
            if len(l.split('=')) < 2:
                continue
            key, value = [re.sub(r'"', "", x.strip()) for x in l.rstrip().split('=')]
            ret[key] = value

    return ret

def getArmoreConfig():
    return getConfigFromFile(ARMORE_CFG_FILE)

def getProxyConfig():
    return getConfigFromFile(PROXY_CFG_FILE)

def getTransparentConfig():
    return getConfigFromFile(TRANSPARENT_CFG_FILE)

def getPassiveConfig():
    return getConfigFromFile(PASSIVE_CFG_FILE)

def updateConfig(item, configType):
    cfgFile = cfgFiles[configType.lower()]
    cfg = getConfigFromFile(cfgFile)

    for i in item: 
        if i in cfg:
            cfg[i] = item[i]
    
    writeConfig(cfg, configType)

def writeConfig(config, configType='armore'):
    
    if configType.lower() == 'proxy':
        fileToOpen = PROXY_CFG_FILE_NEW
    elif configType.lower() == 'passive':
        fileToOpen = PASSIVE_CFG_FILE
    elif configType.lower() == 'transparent':
        fileToOpen = TRANSPARENT_CFG_FILE
    else:
        fileToOpen = ARMORE_CFG_FILE

    oldConfig = getConfigFromFile(fileToOpen)

    newConfigStr = "#!/usr/bin/env bash\n"
    for key in config:
        if type(config[key]) is dict:
            for k in config[key]:
                newConfigStr += "{0}_{1}=\"{2}\"\n".format(key, k, config[key][k])
        elif type(config[key]) is str:
            newConfigStr += "{0}=\"{1}\"\n".format(key, config[key])

    newConfigFile = open(fileToOpen, 'w')
    newConfigFile.write(newConfigStr)
    newConfigFile.close()

    restartFlask = False
    if configType.lower() == 'armore':
        if oldConfig:
            if "Management_IP" in oldConfig and "Management" in config:
                if oldConfig["Management_IP"] != config["Management"]["IP"]:
                    restartFlask = True
            elif "Management_IP" in oldConfig and "Management_IP" in config:
                if oldConfig["Management_IP"] != config["Management_IP"]:
                    restartFlask = True
            elif "Management" in oldConfig and "Management" in config:
                if oldConfig["Management"]["IP"] != config["Management"]["IP"]:
                    restartFlask = True
            elif "Management" in oldConfig and "Management_IP" in config:
                if oldConfig["Management"]["IP"] != config["Management_IP"]:
                    restartFlask = True
            
    # Remove this once we get a single config file system in place
    if configType != 'armore':
        writeProxyConfig(config)

    restartFlask = False
    return restartFlask

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

def writeInterfacesFile(config, configType):
    fileToOpen = netFiles[configType]
    t = open(fileToOpen, 'r')
    temp = Template(t.read())
    f = open('/etc/network/interfaces','w')
    f.write(temp.substitute(config))
    f.close()
    comLib.cmd("service networking restart")

