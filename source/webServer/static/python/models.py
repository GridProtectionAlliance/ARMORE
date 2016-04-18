from flask.ext.sqlalchemy import SQLAlchemy
from werkzeug import generate_password_hash, check_password_hash
import binascii
import os
import sqlalchemy

db = SQLAlchemy()

class Users(db.Model):
    __tablebname__ = "Users"
    uid = db.Column(db.Integer, primary_key = True)
    username = db.Column(db.String(100), unique=True)
    pwdhash = db.Column(db.VARBINARY(256))
    roleName = db.Column(db.VARCHAR(100))

    def __init__(self, username, pw, role="user"):
        self.username = username
        self.pwdhash = pw
        self.roleName = role

    def chkPwHash(self, password):
        pwhash_str = self.pwdhash.decode()
        return check_password_hash(pwhash_str, password)

class UserRoles(db.Model):
    __tablename__ = "UserRoles"
    roleName = db.Column(db.VARCHAR(100), primary_key = True)
    roleValue = db.Column(db.Integer)

    def __init__(self, name, value):
        self.roleName = name
        self.roleValue = value

def getUsernames():
    usernames = []
    userObjs = Users.query.all()
    for u in userObjs:
        usernames.append(u.username)
    return usernames

def getValidRoles(theUsername):
    smallestRoleValue = getRoleValue(getRole(theUsername))
    userObjs = UserRoles.query.all()
    roles = []
    rolesDict = {}
    for uo in userObjs:
        rolesDict[uo.roleName] = 1
    for r in list(rolesDict.keys()):
        rd = {}
        rd["value"] = r
        rd["name"] = r.capitalize()
        roles.append(rd)
    if len(roles) == 0:
        roles.append({"value": "", "name": "user"})
    return roles

def getRole(theUsername):
    return Users.query.filter(Users.username == theUsername).first().roleName

def getPwdHash(theUsername):
    return Users.query.filter(Users.username == theUsername).first().pwdhash

def getRoleValue(theRoleName):
    user = Users.query.join(UserRoles, Users.roleName==UserRoles.roleName).add_columns(UserRoles.roleValue).filter(Users.roleName == theRoleName).first()
    if user:
        return user.roleValue
    else:
        return 0

def userAllowed(userRole, *roles):
    roleVals = [getRoleValue(r) for r in roles]
    return getRoleValue(userRole) <= max(roleVals)

def rolesAllowed(*roles):
    def wrapper(f):
        @wraps(f)
        def wrapped(*args, **kwargs):
            if not userAllowed(session['role'], *roles):
                return error_handler()
            return f(*args, **kwargs)
        return wrapped
    return wrapper

def getUsers():
    ret = []
    for u in getUsernames():
        dic = {}
        dic['username'] = u
        dic['role'] = getRole(u)
        ret.append(dic)
    return ret

def isInitialSetup():
    if not os.path.isfile("./database/armore.db"):    
        os.system('mkdir database')
        os.system('touch database/armore.db')
        db.create_all()
        db.session.commit()

    numOfUsers = 0
    try:
        numOfUsers = len(getUsernames())
    except sqlalchemy.exc.OperationalError:
        None

    return numOfUsers == 0
