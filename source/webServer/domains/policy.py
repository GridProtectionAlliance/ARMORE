# # # # #
# policy.py
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
import domains.support.policy as polLib
from domains.support.lib.common import *
from flask import Blueprint, render_template, redirect, url_for, session, request

policyDomain = Blueprint('policy', __name__)

@policyDomain.route("/policy/edit/<string:num>", methods=["GET", "POST"])
@policyDomain.route("/policy/add", methods=["GET", "POST"])
@secure(["admin","user"])
def addPolicy(num=None):

    if request.method == 'POST':
        polLib.updatePolicyConfig(request.form, num)
        #comLib.startNewThread(confLib.restartBro)
        return redirect(url_for(".currentPolicies"))

    return render_template("addPolicy.html", 
        common = sysLib.getCommonInfo({"username": session["username"]}, "policy"),
        policy = polLib.getPolicy(num)
    )

@policyDomain.route("/policy", methods=["GET"])
@secure(["admin","user"])
def currentPolicies():

    return render_template("policy.html", 
        common = sysLib.getCommonInfo({"username": session["username"]}, "policy"),
        policies = polLib.getPolicies()
    )

@policyDomain.route("/policy/delete/<string:theId>", methods=["POST", "GET"])
@secure(["admin","user"])
def deletePolicy(theId):

    polLib.delPolicy(theId)

    return redirect(url_for(".currentPolicies"))


