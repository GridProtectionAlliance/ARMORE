# # # # #
# common.py
#
# Contains methods used across
# multiple backend files
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

from datetime import datetime
from subprocess import check_output
import json
import threading
import os
import re

from flask_httpauth import HTTPDigestAuth
from functools import wraps
import domains.support.models as modLib
from flask import session, request, flash, redirect, url_for, Response

auth = HTTPDigestAuth(realm="ARMORE", use_ha1_pw=True)

defaultCreds = { "armore": auth.generate_ha1("armore", "armore") }

@auth.error_handler
def secureError():
    return Response('<script> window.location.replace("/welcome")</script>',  401, {'WWWAuthenticate':'Digest realm="Login Required"'})

def notAuthorized():
    flash("Not Authorized to View This Page")
    if not request.referrer:
        return redirect(url_for('.welcome'))
    return redirect(request.referrer)

# Decorator for determining if user is authenticated and authorized to view resource
# If 'roles' is a string, resource requires AT LEAST the specified level of authorization
# if 'roles' is an array of strings, resource requires ANY of the specified levels of authorization
def secure(roles):
    def wrapper(f):
        @wraps(f)
        @auth.login_required
        def wrapped(*args, **kwargs):
            if modLib.isInitialSetup():
                return redirect("/admin/initialUserSetup")
            if 'username' not in session:
                session['username'] = auth.username()
                session['role'] = modLib.getRole(session['username'])
            if type(roles) is list:
                if session['role'] not in roles:
                    return notAuthorized()
            elif type(roles) is str:
                if modLib.getRoleValue(session['role']) > modLib.getRoleValue(roles):
                    return notAuthorized()
            else:
                print("#### ERROR: 'roles' NOT A VALID TYPE ####")
                return secureError() 
            return f(*args, **kwargs)
        return wrapped
    return wrapper

# Gets password from Users database.  Defines function for flask-httpauth
@auth.get_password
def getPw(theUsername):
    try:
        if not modLib.isInitialSetup():
            user = modLib.Users.query.filter_by(username = theUsername).all()
            if len(user) > 0:
                return user[0].pwdhash.decode('utf-8')
            if modLib.isInitialSetup():
                if theUsername in defaultCreds:
                    return defaultCreds.get(theUsername)
            else:
                if theUsername in getUsernames():
                    return userCreds.get(theUsername)
    except:
        None

    return None

# Flash is how Flask displays error messages to the user.  These are helper functions for that
def clearFlash():
    if 'flash' in session:
        del session['flash']

def addToFlash(err):
    if err != "":
        if 'flash' in session:
            session['flash'] += err + "\n"
        else:
            session['flash'] = err

def getCurrentTimestampAsStr():
    return str(datetime.now())

def getTemplateAsString(pathToTemplate):
    with open(pathToTemplate, "r") as temp:
        tString = temp.read()
    return tString

def getPocPhoneNumber():
    return "(555) 123-4567 (Test Number Only)"

def getLocation():
    return "Olympus Mons (Test Location Only)"

def getDescription():
    return "Winter getaway, snowfall rare (Test Description Only)"

def getJsonStrUnformatted(inputDict):
    return json.loads(json.JSONEncoder().encode(inputDict))

def getJsonStrFormatted(inputDict):
    return json.dumps(getJsonStrUnformatted(inputDict), sort_keys=True, indent=4)

def monToNum(mon):
    return {
            'Jan': 1,
            'Feb': 2,
            'Mar': 3,
            'Apr': 4,
            'May': 5,
            'Jun': 6,
            'Jul': 7,
            'Aug': 8,
            'Sep': 9,
            'Oct': 10,
            'Nov': 11,
            'Dec': 12,
            }[mon]

def toTimestamp(mon, day, hh, mm, ss, year=None):
    today = datetime.today()
    if year == None:
        year = today.year
        if mon > today.month:
            year -= 1
    return datetime(year, mon, day, hh, mm, ss).timestamp()

# Runs system commands
def cmd(command, isShell=False):
    if type(command) is list: 
        return check_output(command, shell=isShell).decode('utf-8')
    else:
        return check_output(command.split(' '), shell=isShell).decode('utf-8')

def timestampToPrettyDate(timestamp):
    return datetime.fromtimestamp(timestamp).strftime('%Y/%m/%d %H:%M:%S')

def getFileType(theFile):
    return cmd('file ' + theFile).split(': ')[1]

def startNewThread(method, args=()):
    t = threading.Thread(target=method, args=args)
    t.start()

def getKeysByValue(inpDict, value):
    return [k for k,v in inpDict.items() if v == value]

def getFileList(theDir, recursive=False, match='.*'):
    fileList = {}
    for (dirpath, dirnames, filenames) in os.walk(theDir):
        if dirpath == 'static/rrd' or re.match('.*Debian.*amd', dirpath) or not re.match(match, dirpath.split('/')[-1]):
            continue
        tempDict = {}
        for filename in filenames:
            tempId = {filename: {"ds": [], "cf": []}}
            tempDict.update(tempId)
        fileList.update({dirpath: tempDict})
        if not recursive:
            break

    return fileList

# Gets file location based on supportFiles.txt 
def getSupportFilePath(desiredFile, supportFilePath=None):

    if supportFilePath is None:
        supportFilePath = "/var/webServer/supportFiles.txt"

    with open(supportFilePath, 'r') as theFile:
        theFileLines = [x.rstrip() for x in theFile.readlines()]
        for l in theFileLines:
            if re.search("=", l):
                key, value = l.split('=')
                if key == desiredFile:
                    return value

    return None

def sortOrder(array, sortBy, orderBy):
    ret = []
    used = {}

    for i in range(1, len(array)):
        j = i
        while j > 0 and array[j][sortBy] < array[j-1][sortBy]:
            array[j], array[j-1] = array[j-1], array[j]
            j -= 1

    if orderBy == "desc":
        array = array[::-1]

    return array


