# # # # #
# logs.py #
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

from flask import Blueprint, render_template, session, request, flash, redirect, send_from_directory, send_file, make_response
from domains.support.lib.common import secure
import domains.support.system as sysLib
import domains.support.logs as logsLib
from domains.support.lib.common import addToFlash
from datetime import datetime, timedelta

logDomain = Blueprint('logs', __name__)

@logDomain.route("/example/logs")
@secure(["admin","user"])
def example_logs():

    return render_template("example/logs.html", 
        common          = sysLib.getCommonInfo({"username": session['username']}, "logs"),
        logs            = logsLib.getLogsInfo()
    )

@logDomain.route("/logs")
@secure(["admin","user"])
def logs():

    return render_template("logs.html", 
        common          = sysLib.getCommonInfo({"username": session['username']}, "logs"),
    )

#@logDomain.route("/logs/<subsystem>")
@logDomain.route("/logs/")
@logDomain.route("/logs/<path:thePath>")
@secure(["admin","user"])
def logsFiltered(thePath=None):

    sort = request.args.get("sort")
    order = request.args.get("order")

    if sort == "" or sort is None:
        sort = "mtime"
    if order == "" or order is None:
        order = "desc"

    if thePath is None:
        thePath = "/var/log"
    else:
        thePath = "/var/log/{}".format(thePath)
    if not logsLib.pathIsValid(thePath):
        addToFlash("Path is invalid: redirecting to {}".format(thePath))
        return redirect("/logs")

    return render_template("logsFiltered.html", 
        common          = sysLib.getCommonInfo({"username": session['username']}, "logs"),
        logs            = logsLib.getLogsInfo(thePath, sort, order),
        headerName      = thePath,
        path            = thePath,
        sort            = sort,
        order           = order
    )

@logDomain.route('/example/viewLog/')
@secure(["admin","user"])
def example_view_log():
    filename = request.args.get('filename')
    seekTail = request.args.get('seek_tail')
    if seekTail:
        return logsLib.getLogContent(filename)
    else:
        return render_template("example/log.html", 
            common          = sysLib.getCommonInfo({"username": session['username']}, "logs"),
            content         = logsLib.getLogContent(filename),
            filename        = filename,
        )

@logDomain.route('/viewLog')
@secure(["admin","user"])
def view_log():
    filename = request.args.get('filename')
    if not logsLib.pathIsValid(filename):
        flash("Invalid file path")
        return redirect("/logs")

    seekTail = request.args.get('seek_tail')
    content = logsLib.getLogContent(filename)
    if content == '':
        flash("Unable to load file")
        return redirect(request.referrer)
    if seekTail:
        return content 
    else:

        return render_template("log.html", 
            common          = sysLib.getCommonInfo({"username": session['username']}, "logs"),
            content         = content,
            filename        = filename,
        )

@logDomain.route('/searchLog/')
@secure(["admin","user"])
def search_log():
    filename = request.args.get('filename')
    filterBy = request.args.get('text')
    nextSrch = request.args.get('next')
    nextSrch = 0 if not nextSrch else nextSrch
    return logsLib.getLogContentFiltered(filename, filterBy, int(nextSrch))

@logDomain.route('/logs/download')
@secure("user")
def downloadLog():

    filepath = request.args.get('filepath')

    if not logsLib.pathIsValid(filepath):
        flash("Invalid file path")
        return redirect("/logs")

    fDir = "/".join(filepath.split('/')[0:-1])
    fName = filepath.split('/')[-1]
    response = make_response(logsLib.getLogContent(filepath))
    response.headers["Content-Disposition"] = "attachment; filename={}".format(fName)
    return response

