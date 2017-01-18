# # # # #
# processes.py
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

import domains.support.system as sysLib
from domains.support.lib.common import *
from flask import Blueprint, render_template, session, request

processDomain = Blueprint('processes', __name__)

@processDomain.route("/example/processes/")
@processDomain.route("/processes/")
@secure(["admin","user"])
def processes(sort=None, order=None, filter=None, pid=None, section=None):

    sort = request.args.get("sort")
    order = request.args.get("order")
    filter = request.args.get("filter")
    pid = request.args.get("pid")
    section = request.args.get("section")

    return render_template("example/processes.html", 
        common          = sysLib.getCommonInfo({"username": session['username']}, "processes"),
        order           = order,
        sort            = sort,
        num_procs       = sysLib.getNumberOfProcs()[0],
        num_user_procs  = sysLib.getNumberOfProcs()[1],
        processes       = sysLib.getProcessStats(sort, order, filter, pid, section),
        filter          = filter
    )

