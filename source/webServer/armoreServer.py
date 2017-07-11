# # # # #
# armoreServer.py
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

# Import python Flask server
# and add backend files to sys.path

import sys
import re
import domains.support.config as confLib
import domains.support.network as netLib
import domains.support.system as sysLib
from flask import Flask, render_template, redirect, url_for, session, request, Response, send_from_directory
from flask.ext.cors import CORS
from domains.support.models import *
from domains.admin import adminDomain
from domains.dashboard import dashboardDomain
from domains.logs import logDomain
from domains.network import netDomain
from domains.policy import policyDomain
from domains.processes import processDomain
from domains.security import securityDomain
from domains.settings import settingsDomain
from domains.statistics import statsDomain
from domains.visualization import visualDomain
from domains.baseline import baselineDomain
from domains.anomalies import anomalyDomain
from domains.support.lib.common import *
import logging
if sys.platform == 'darwin':
    logging.basicConfig()
log = logging.getLogger('werkzeug')
log.setLevel(logging.DEBUG)

app = Flask(__name__)
CORS(app)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///database/armore.db'
app.config['SQLALCHEMY_BINDS'] = {"policyEvents": 'sqlite:///database/policy.sqlite'}
app.secret_key = "ARMURSEEKRIT"
app.register_blueprint(adminDomain)
app.register_blueprint(dashboardDomain)
app.register_blueprint(logDomain)
app.register_blueprint(netDomain)
app.register_blueprint(policyDomain)
app.register_blueprint(processDomain)
app.register_blueprint(securityDomain)
app.register_blueprint(settingsDomain)
app.register_blueprint(statsDomain)
app.register_blueprint(visualDomain)
app.register_blueprint(baselineDomain)
app.register_blueprint(anomalyDomain)
db.init_app(app)
app.debug = True

@app.route("/")
def home():
    return redirect(url_for(".welcome"))

@app.route("/index.html")
@app.route("/welcome")
def welcome():
    if 'username' in session:
        return redirect(url_for("dashboard.dashboard"))

    if isInitialSetup():
        return redirect("/admin/initialUserSetup")

    return render_template("welcome.html",
        common = sysLib.getCommonInfo({"username": ""}, "welcome")
    )

@app.before_request
def checkInitial():
    if modLib.isInitialSetup() and request.path != "/admin/initialUserSetup" and not re.search('static', request.path):
        return redirect("/admin/initialUserSetup")

@app.after_request
def add_header(response):
    response.cache_control.max_age = 1
    return response

@app.errorhandler(404)
def notFound(e):
    return redirect(url_for('.welcome'))

@app.errorhandler(500)
def notFound(e):
    return redirect(request.referrer)

@app.route('/accessDenied')
def accessDenied():
    return render_template("access_denied.html",
        common = sysLib.getCommonInfo({"username": session["username"]}, "accessDenied"),
    )

@app.route('/favicon.ico')
def getFavicon():
   return send_from_directory(os.path.join(app.root_path, 'static/img'), 'favicon.ico', mimetype='image/vnd.microsoft.icon')

@app.route('/signin', methods=['GET','POST'])
def signin():
    username = request.form.get('username', None)
    password = request.form.get('password', None)

    if request.method == 'POST':
            session['username'] = username 

    return redirect(url_for('dashboard.dashboard'))

@app.errorhandler(401)
@app.route('/signout', methods=['GET', 'POST'])
def signout():
    if modLib.isInitialSetup():
        return redirect("/admin/initialUserSetup")
    session.pop("username", None) 
    session.pop("role", None) 
    return Response('<script> window.location.replace("/welcome")</script>',  401, {'WWWAuthenticate':'Digest realm="Login Required"'})

@app.route("/restartServer", methods=["POST"])
@secure(["admin","user"])
def restartServer():
    f = request.environ.get('werkzeug.server.shutdown')
    f()
    return ""

@app.route("/ping")
@secure(["admin","user"])
def pingServer():
    return ""

if __name__ == "__main__":
    confLib.synchConfigs()
    armoreConf = confLib.getArmoreConfig()
    if not armoreConf:
        ipAddr = netLib.getPrimaryIp()
    else:
        ipAddr = armoreConf.get('Management_IP')

    # Debug code until we get the Management IP, Subnet and Interface into the backend menu
    if ipAddr is None or ipAddr == "None":
        ipAddr = input("Enter IP Address: ") 

    portNum = 8080 if len(sys.argv) < 2 else int(sys.argv[1])
    app.config['portNum'] = portNum
    if sys.platform == 'darwin':
        print("Starting web server on 127.0.0.1:" + str(portNum))
        app.run(host='127.0.0.1', port=portNum, debug=False, use_evalex=False)
    else:
        print("Starting Server On ", ipAddr, ":", portNum)
        app.run(host=ipAddr, port=portNum, debug=True, use_evalex=False)


