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

from flask import Blueprint, render_template, redirect, session, request, url_for, flash, make_response, Markup
from werkzeug.utils import secure_filename

from domains.support.lib.common import secure
import domains.support.models as modLib
import domains.support.system as sysLib
import domains.support.keys as keysLib
import domains.support.initialization as initLib

import re
import os.path

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
            return redirect("/admin/users")

    return render_template("addUser.html",
        common          = sysLib.getCommonInfo({"username": session['username']}, "admin"),
        roles           = modLib.getValidRoles(session['username']),
        isInitial       = False
    )

@adminDomain.route("/admin/initialUserSetup", methods=["GET", "POST"])
def initialUserSetup():
    if not modLib.isInitialSetup():
        return redirect(url_for("admin.welcome"))
 
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

    initLib.initAll()

    return render_template("addUser.html",
        common          = sysLib.getCommonInfo({"username": "DEFAULT"}, "initialUserSetup"),
        roles           = [{"value": "admin", "name":"Admin"}],
        isInitial       = True
    )   

@adminDomain.route("/admin/users")
@secure("admin")
def users():

   return render_template("users.html",
        common          = sysLib.getCommonInfo({"username": session['username']}, "admin"),
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

    if session['role'] != "admin" and username != session['username']:
        flash("Invalid Profile: Only able to view your own profile")
        return redirect(url_for("admin.profile", username=session['username']))

    return render_template("profile.html",
        common          = sysLib.getCommonInfo({"username": session['username']}, "admin"),
        roles           = modLib.getValidRoles(session['username']),
        username        = {"name": username, "role": modLib.getRole(username)}
    )

@adminDomain.route("/admin/updatePwd/<username>", methods=["POST"])
@secure("user")
def updatePwd(username):

    md5_Digest = request.form.get('md5_Digest', None)
    md5_Digest_Old = request.form.get('md5_Digest_Old', None)

    if md5_Digest_Old.encode('utf-8') == modLib.getPwdHash(username):
        modLib.changePassword(username, md5_Digest)
    else:
        flash("Passwords don't match")

    return redirect(url_for("admin.profile", username=username))

@adminDomain.route("/admin/user/delete/<username>")
@secure("admin")
def deleteUser(username):
    success = modLib.deleteUser(username)
    if not success:
       flash("Unable to delete this user from database".format(username))

    return redirect("/admin/users")

@adminDomain.route("/admin/keys")
@secure("admin")
def keys():

   sort = request.args.get("sort") or "name"
   order = request.args.get("order") or "desc"

   if sort not in ["name", "type", "mtime"]:
      sort = "name"

   if order not in ["asc", "desc"]:
      order = asc

   return render_template("keyManagement.html",
      common = sysLib.getCommonInfo({"username": session['username']}, "admin"),
      roles = modLib.getValidRoles(session['username']),
      keys = keysLib.getKeys(sort, order),
      sort = sort,
      order = order
   )

@adminDomain.route('/admin/keys/download')
@secure("admin")
def downloadKey():

    fileName = request.args.get('name')
    keyType = request.args.get('type')

    response = make_response(keysLib.getKeyContent(fileName, keyType))
    response.headers["Content-Disposition"] = "attachment; filename={}".format(fileName)
    return response

@adminDomain.route("/admin/keys/upload", methods=["POST"])
@secure("admin")
def uploadFile():
   if "keyFile" not in request.files:
      flash("ERROR: No File Part")
   elif request.files['keyFile'].filename == '':
      flash("ERROR: No File Selected")
   else:
      theFile = request.files.get('keyFile')
      keyType = request.form.get('keyType')

      fileName = secure_filename(theFile.filename)
      fileOK, errs = keysLib.verifyFileName(theFile.filename, keyType)

      if fileOK:
         msg = keysLib.saveFile(keyType, theFile)
         print(msg)
         if len(msg) > 0:
            flash(Markup(msg))
      else:
         flash(errs)

   return redirect(request.referrer)

@adminDomain.route("/admin/keys/delete/<string:keyType>/<string:filename>")
@secure("admin")
def deleteKey(keyType, filename):
   msg = keysLib.deleteFile(keyType, filename)
   if len(msg) > 0:
      flash(Markup(msg))

   return redirect(request.referrer)


