# # # # #
# admin.py
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

from flask import Blueprint, render_template, redirect, session, request, url_for
from .lib.common import secure
import models as modLib
import system as sysLib
import re
import os.path
import sqlite3

adminDomain = Blueprint('admin', __name__)

@adminDomain.route("/admin/")
@secure("admin")
def admin():

    return render_template("admin.html",
        common          = sysLib.getCommonInfo({"username": session['username']}, "admin"),
    )

@adminDomain.route("/admin/addUser", methods=["GET", "POST"])
@secure("admin")
def addUser():

    if request.method == 'POST':
        email = request.form.get('email', None)
        password = request.form.get('password', None)
        cPassword = request.form.get('confirm_password', None)
        md5_Digest = request.form.get('md5_Digest', None)
        role = request.form.get('role')
        if email not in modLib.getUsernames():
            newuser = modLib.Users(email, md5_Digest.encode('utf-8'), role)
            modLib.db.session.add(newuser)
            modLib.db.session.commit()

    return render_template("addUser.html",
        common          = sysLib.getCommonInfo({"username": session['username']}, "admin/addUser"),
        roles           = modLib.getValidRoles(session['username']),
        isInitial       = False
    )

@adminDomain.route("/admin/initialUserSetup", methods=["GET", "POST"])
def initialUserSetup():
    if not modLib.isInitialSetup():
        return redirect(url_for("welcome"))
 
    if request.method == 'POST':
        email = request.form.get('email', None)
        password = request.form.get('password', None)
        cPassword = request.form.get('confirm_password', None)
        md5_Digest = request.form.get('md5_Digest', None)
        role = request.form.get('role')
        if email not in modLib.getUsernames():
            newuser = modLib.Users(email, md5_Digest.encode('utf-8'), role)
            modLib.db.session.add(newuser)
            modLib.db.session.commit()
        return redirect(url_for("signout"))
        

    return render_template("addUser.html",
        common          = sysLib.getCommonInfo({"username": "DEFAULT"}, "initialUserSetup"),
        roles           = [{"value": "admin", "name":"Admin"}],
        isInitial       = True
    )   

@adminDomain.route("/admin/users")
@secure("admin")
def users():

   return render_template("users.html",
        common          = sysLib.getCommonInfo({"username": session['username']}, "admin/users"),
        roles           = modLib.getValidRoles(session['username']),
        users           = modLib.getUsers()
    )

@adminDomain.route("/admin/checkUsername")
@secure("admin")
def checkUsername():
    username = request.args.get('username')
    if username in modLib.getUsernames():
        return "Username Already Exists"
    else:
        return "Username OK"

@adminDomain.route("/admin/profile/<username>", methods=["GET","POST"])
@secure("user")
def profile(username):
    
    return render_template("profile.html",
        common          = sysLib.getCommonInfo({"username": session['username']}, "profile"),
        roles           = modLib.getValidRoles(session['username']),
        username        = {"name": username, "role": modLib.getRole(username)}
    )

@adminDomain.route("/admin/updatePwd/<username>", methods=["POST"])
@secure("user")
def updatePwd(username):

    md5_Digest = request.form.get('md5_Digest', None)
    if md5_Digest.encode('utf-8') == modLib.getPwdHash(username):
        user = modLib.Users.query.filter_by(username=username)
        user.pwdhash = md5_Digest.encode('utf-8')
        modLib.db.session.commit()

    return redirect(url_for("admin.profile", username=username))

