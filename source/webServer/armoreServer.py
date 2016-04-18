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
sys.path.append("./static/python")
sys.path.append("./static/python/lib")

import datetime
import system as sysLib
import config as confLib
import network as netLib
from flask import Flask, render_template, redirect, url_for, session, request, Response, make_response, flash
from flask.ext.cors import CORS, cross_origin
from models import *
from domains.admin import adminDomain
from domains.logs import logDomain
from domains.settings import settingsDomain
from domains.lib.common import *
from domains.lib import create_json_graph as vis
import time

import logging
log = logging.getLogger('werkzeug')
log.setLevel(logging.DEBUG)

app = Flask(__name__)
CORS(app)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///database/armore.db'
app.secret_key = "ARMURSEEKRIT"
app.register_blueprint(adminDomain)
app.register_blueprint(logDomain)
app.register_blueprint(settingsDomain)
db.init_app(app)

@app.route("/")
def home():
    return redirect(url_for("welcome"))

@app.route("/index.html")
@app.route("/welcome")
def welcome():
    if 'username' in session:
        return redirect(url_for("dashboard"))

    if isInitialSetup():
        return redirect("/admin/initialUserSetup")

    return render_template("welcome.html",
        common          = sysLib.getCommonInfo({"username": ""}, "welcome")
    )

@app.errorhandler(404)
def notFound(e):
    return redirect(url_for('welcome'))

@app.errorhandler(500)
def notFound(e):
    return redirect(request.referrer)

@app.route('/accessDenied')
def accessDenied():
    return render_template("access_denied.html",
        common          = sysLib.getCommonInfo({"username": session["username"]}, "accessDenied"),
        #theUsers        = sysLib.getUserStats(),
        #uptime          = sysLib.getUptime(),
        #currUser        = "username": session["username"]}
    )

@app.route('/signin', methods=['GET','POST'])
def signin():
    username = request.form.get('username', None)
    password = request.form.get('password', None)

    print(username, password)
    if request.method == 'POST':
            session['username'] = username 

    return redirect(url_for('dashboard'))

@app.errorhandler(401)
@app.route('/signout', methods=['GET', 'POST'])
def signout():
   session.pop("username", None) 
   session.pop("role", None) 
   return Response('<script> window.location.replace("/welcome")</script>',  401, {'WWWAuthenticate':'Digest realm="Login Required"'})

@app.route("/example/dashboard")
@secure(["admin","user"])
def example_dashboard():

    return render_template("example/dashboard.html",
        common          = sysLib.getCommonInfo({"username": session["username"]}, "dashboard"),
        uptime          = sysLib.getUptime(),
        currUser        = {"username": session['username']},
        page            = "dashboard",
        load_avg        = getLoadAvg(),
        cpu             = getCpuStats(),
        num_cpus        = getNumOfCores(),
        memory          = getMemoryStats(),
        net_interfaces  = getInterfaceStats(),
        disks           = getDiskStats(),
        swap            = getSwapStats(),
        theUsers        = sysLib.getUserStats()
    )

@app.route("/dashboard")
@secure(["admin","user"])
def dashboard():

    return render_template("dashboard.html",
        common          = sysLib.getCommonInfo({"username": session["username"]}, "dashboard"),
        uptime          = sysLib.getUptime(),
        currUser        = {"username": session['username']},
        page            = "dashboard",
        systems         = sysLib.getDashboardInfo()
    )

@app.route("/example/security")
@secure(["admin","user"])
def example_security():

    return render_template("example/security.html", 
        common          = sysLib.getCommonInfo({"username": session["username"]}, "security"),
        uptime          = sysLib.getUptime(),
        currUser        = {"username": session['username']},
        page            = "security",
    )

@app.route("/security")
@secure(["admin","user"])
def security():

    return render_template("security.html", 
        common          = sysLib.getCommonInfo({"username": session["username"]}, "security"),
        systems         = sysLib.getStatuses(['bro', 'encryption'])
    )

@app.route("/policy")
@secure(["admin","user"])
def policy():

    return render_template("policy.html", 
        common          = sysLib.getCommonInfo({"username": session["username"]}, "policy"),
    )

@app.route("/example/statistics")
@secure(["admin","user"])
def example_statistics():

    return render_template("example/statistics.html", 
        common          = sysLib.getCommonInfo({"username": session["username"]}, "statistics"),
    )

@app.route("/statistics")
@secure(["admin","user"])
def statistics():

    return render_template("statistics.html", 
        common          = sysLib.getCommonInfo({"username": session["username"]}, "statistics"),
    )

@app.route("/example/network")
@secure(["admin","user"])
def example_network():

    connections, types, families, theStates = netLib.getConnections()

    return render_template("example/network.html", 
        common          = sysLib.getCommonInfo({"username": session["username"]}, "network"),
        network_interfaces  = netLib.getInterfaceStats(),
        socket_types        = types,
        states              = theStates,
        socket_families     = families,
        connections         = connections
    )

@app.route("/network")
@secure(["admin","user"])
def network():
    return example_network()

@app.route("/example/processes/")
@app.route("/processes/")
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

@app.route("/visualization")
@secure(["admin","user"])
def visualizePage():
    return render_template("visualization.html",
        common          = sysLib.getCommonInfo({"username": session["username"]}, "visualization"),
    )

@app.route("/data.json")
def jsonData():
    vis.visualize('stats_col.log','templates/data.json')
    time.sleep(3)
    return render_template("data.json")

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
   # confLib.enforceArmoreConfig()
    armoreConf = confLib.getArmoreConfig()
    if not armoreConf:
        ipAddr = netLib.getPrimaryIp()
    else:
        ipAddr = armoreConf['Management_IP']
    portNum = 8080 if len(sys.argv) < 2 else sys.argv[1]
    app.config['portNum'] = portNum
    print("Starting Server On ", ipAddr, ":", portNum, sep="")

    app.run(host=ipAddr, port=portNum, debug=True, use_evalex=False)


