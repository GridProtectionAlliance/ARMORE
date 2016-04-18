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

from flask import Blueprint, render_template, session, request, flash, redirect
from .lib.common import secure
import system as sysLib
import logs as logsLib

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

@logDomain.route("/logs/<subsystem>")
@secure(["admin","user"])
def logsFiltered(subsystem):

    return render_template("logsFiltered.html", 
        common          = sysLib.getCommonInfo({"username": session['username']}, "logs"),
        logs            = logsLib.getLogsInfo(subsystem)
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

@logDomain.route('/viewLog/')
@secure(["admin","user"])
def view_log():
    filename = request.args.get('filename')
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



