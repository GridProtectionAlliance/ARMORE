from flask_httpauth import HTTPDigestAuth
from functools import wraps
import models as modLib
from flask import session, request, flash, redirect, url_for, Response

auth = HTTPDigestAuth(realm="ARMORE", use_ha1_pw=True)

defaultCreds = { "armore": auth.generate_ha1("armore", "armore") }

@auth.error_handler
def secureError():
    return Response('<script> window.location.replace("/welcome")</script>',  401, {'WWWAuthenticate':'Digest realm="Login Required"'})

def notAuthorized():
    flash("Not Authorized to View This Page")
    if not request.referrer:
        return redirect(url_for('welcome'))
    return redirect(request.referrer)

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

@auth.get_password
def getPw(theUsername):
    try:
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

def clearFlash():
    if 'flash' in session:
        del session['flash']

def addToFlash(err):
    if err != "":
        if 'flash' in session:
            session['flash'] += err + "\n"
        else:
            session['flash'] = err
