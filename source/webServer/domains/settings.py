# # # # #
# settings.py
#
# This file is used to serve up
# RESTful links that can be
# consumed by a frontend system
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
import sys
from flask import Blueprint, render_template, redirect, session, request, url_for, flash, Response
from flask.ext.cors import CORS, cross_origin
from domains.support.lib.common import *
import domains.support.lib.common as comLib
import domains.support.system as sysLib
import domains.support.network as netLib
import domains.support.config as confLib
import time
import re

settingsDomain = Blueprint('settings', __name__)
CORS(settingsDomain)

@settingsDomain.route("/example/settings")
@secure(["admin","user"])
def example_settings():

    return render_template("example/settings.html", 
        common          = sysLib.getCommonInfo({"username": session['username']}, "settings"),
    )

@settingsDomain.route("/settings", methods=['GET','POST'])
@secure(["admin","user"])
@cross_origin()
def settings():
    err = ""
    config = {}
    try:
        config = confLib.getCurrentConfig()
    except:
        #err = "Error reading config: " + sys.exc_info()[0]
        session['flash'] = "Unable to get current config"

    if err != "":
        if 'flash' in session:
            if session['flash']:
                flash(session['flash'])

    confLib.synchConfigs()
    if 'Operation' not in config:
        config['Operation'] = None
    if 'Roletype' not in config:
        config['Roletype'] = None
    if 'Interface' not in config:
        config['Interface'] = None
    if 'Encryption' not in config:
        config['Encryption'] = None
    if 'Capture' not in config:
        config['Capture'] = None
    if 'Bind' not in config:
        config['Bind'] = None
    if 'Port' not in config:
        config['Port'] = None

    proxyState = sysLib.getProcessDict("ARMOREProxy")
    broState = sysLib.getProcessDict("bro")
    armoreState = sysLib.getArmoreStatus()
    #armoreState = sysLib.getArmoreState(proxyState, broState)

    return render_template("settings.html", 
        common          = sysLib.getCommonInfo({"username": session['username']}, "settings"),
        interfaces      = netLib.getInterfaceIps(),
        armoreCfg       = confLib.getArmoreConfig(),
        proxyCfg        = confLib.getProxyConfig(),
        passvCfg        = confLib.getPassiveConfig(),
        transCfg        = confLib.getTransparentConfig(),
        armore          = armoreState,
        bro             = broState,
    )

@settingsDomain.route("/redirect/<ip>", methods=["POST","GET"])
@secure(["admin","user"])
def redirectTo(ip):
    return render_template("redirect.html",
        common   = sysLib.getCommonInfo({"username": session['username']}, "redirect"),
        dest     = ip + ":8080/signout"
    )

@settingsDomain.route("/settings/armore", methods=['POST'])
@secure(["admin","user"])
def postArmoreSettings():
    config = {}
    err = ""
    restartFlask = confLib.updateConfig(request.form, "mode", enforce=True)
            
    if restartFlask:
        return redirect(url_for("settings.redirectTo", ip=config["Management_IP"]))
    else:
        return redirect(url_for("settings.settings"))

@settingsDomain.route("/settings/proxy", methods=['POST'])
@secure(["admin","user"])
def postProxySettings():
    config = {}
    netConfig = {}
    err = ""
    #try:
    config['Operation'] = 'Proxy'
    config['Interface'] = request.form.get('intInt')
    config['Roletype'] = request.form.get('networkRole')
    config['Encryption'] = request.form.get('encryption')
    config['Capture'] = request.form.get('captureMode')
    config['Bind'] = request.form.get('targetIp')
    config['Port'] = request.form.get('targetPort')
    confLib.writeConfig(config, 'proxy')

    netConfig = config
    netConfig['ExternalInterface'] = request.form.get('extInt')
    netConfig['InternalInterface'] = config['Interface']
    confLib.writeInterfacesFile(netConfig, 'proxy')
    #confLib.writeProxyConfig(config)
    confLib.updateConfig(config, 'armore')
    #except:
    #    err = "Unable to set Proxy config"
    #    print("Error writing config:", sys.exc_info()[0])

    addToFlash(err)
    
    return redirect(url_for("settings.settings"))

@settingsDomain.route("/settings/passive", methods=['POST'])
@secure(["admin","user"])
def postPassiveSettings():
    config = {}
    err = ""
    try:
        config['Operation'] = 'Passive'
        config['Monitored_Interface'] = request.form.get('monInt')
        confLib.writeConfig(config, 'passive')
        confLib.updateConfig(config, 'armore')
    except:
        err = "Unable to set Passive Mode settings"

    addToFlash(err)

    return redirect(url_for("settings.settings"))

@settingsDomain.route("/settings/transparent", methods=['POST'])
@secure(["admin","user"])
def postTransparentSettings():
    config = {}
    err = ""
    try:
        config['Operation'] = 'Transparent'
        config['Interface1'] = request.form.get('brdgInt1')
        config['Interface2'] = request.form.get('brdgInt2')
        config['BridgeIp'] = request.form.get('bridgeIp')
        config['BroadcastIp'] = request.form.get('broadcastIp')
        config['Netmask'] = request.form.get('netmask')
        config['Gateway'] = request.form.get('gateway')
        config['Route'] = request.form.get('route')
        confLib.writeConfig(config, 'transparent')
        confLib.updateConfig(config, 'armore')
    except:
        err = "Unable to set Transparent Mode settings"

    addToFlash(err)

    return redirect(url_for("settings.settings"))

@settingsDomain.route("/settings/restart", methods=['POST'])
@secure(["admin","user"])
def restart():

    try:
        system = request.form.get('restart')
        if system == "ARMOREProxy":
            comLib.startNewThread(confLib.restartArmoreProxy)
        elif system == "Bro":
            comLib.startNewThread(confLib.restartBro)

        time.sleep(3)
        if 'flash' in session:
            del session['flash']
    except:
        print("Error restarting {0}: {1}:".format(system, sys.exc_info()[0]))
        session['flash'] = "Unable to restart " + system

    return redirect(request.referrer)
