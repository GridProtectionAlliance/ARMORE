# # # # #
# mainServer.py
#
# This file is used to serve up
# RESTful links that can be
# consumed by a frontend system

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

# Import python bottle server
# and add backend files to sys.path
from bottle import route, run, BaseResponse, response, request, hook, static_file
import sys
sys.path.append("./status")
sys.path.append("./config")
sys.path.append("./logging")
sys.path.append("./version")
sys.path.append("./lib")

from reportStatus import *
from reportConfig import *
from reportLogs import *
from reportVersion import *
from networking import blockTraffic, flushTables

@hook('after_request')
def enableCors():
    response.headers['Access-Control-Allow-Origin'] = '*'

# If a user tries to do anything with the homepage
#@route('/')
@route('/armore')
def home():
    return '''<b>Welcome to Armore!</b><br/><br/>
    We are currently under construction and will be for the forseeable future.  Please contact your local ARMORE representative for any ARMORE related questions.<br/><br/>

    Have a nice day!'''

# Returns JSON message containing status information
# about this ARMORE node
@route('/armore/status', method='GET')
def status():
    print("Status Requested")
    theStatus = reportStatus()
    return theStatus

# Returns JSON message containing config information
# about this ARMORE node
@route('/armore/config', method='GET')
def status():
    print("Config Requested")
    theConfig = reportConfig()
    return theConfig

# Returns JSON message containing version information
# about this ARMORE node
@route('/armore/version', method='GET')
def version():
    print("Version Requested")
    theVersion = reportVersion()
    return theVersion

# Returns JSON message containing logs contained
# on this ARMORE node
@route('/armore/logs', method='GET')
@route('/armore/logs/', method='GET')
@route('/armore/logs/<logType>', method='GET')
def logs(logType=None):
    print("Logs Requested")

    year = request.query.year
    mon = request.query.mon if request.query.mon else "Jan"
    day = request.query.day if request.query.day else 1
    hh = request.query.mm if request.query.hh else 0
    mm = request.query.hh if request.query.mm else 0
    ss = request.query.ss if request.query.ss else 0

    theLogs = reportLogsSince(logType, year, mon, day, hh, mm, ss)

    return theLogs

@route('/armore/config/ipBlock/<ipAddr>/<proto>', method='POST')
def ipBlock(ipAddr, proto):
    print("Blocking traffic from " + ipAddr)

    blockTraffic(
            ipSrc = ipAddr,
            proto = proto
            )

@route('/armore/config/flushIpTable', method='POST')
def flushIpTable():
    flushTables()

@route('/<filepath:path>')
@route('/webAdmin/<filepath:path>', method='GET')
def webAdmin(filepath):
       return static_file(filepath, root="../src/WebAdmin")

run(host=sys.argv[1], port=int(sys.argv[2]))

