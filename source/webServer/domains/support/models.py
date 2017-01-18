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

def getUserId(theUsername):
    return Users.query.filter(Users.username == theUsername).first().uid

def getPwdHash(theUsername):
    return Users.query.filter(Users.username == theUsername).first().pwdhash

def uidToUsername(theUid):
    ret = "-1"
    user = Users.query.filter(Users.uid == theUid).first()
    if not user is None:
        ret = user.username
        
    return ret

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

def changePassword(username, md5_Digest):
    user = Users.query.filter_by(username=username).first()
    user.pwdhash = md5_Digest.encode('utf-8')
    db.session.commit()

def deleteAllowed(username):
    users = Users.query.filter(Users.roleName == "admin").all()
    if len(users) == 1 and getRole(username) == "admin":
        return False

    return True

def deleteUser(username):
    if deleteAllowed(username):
        Users.query.filter(Users.username == username).delete()
        db.session.commit()
        return True
    else:
        return False

def getUsers():
    ret = []
    for u in getUsernames():
        dic = {}
        dic['username'] = u
        dic['role'] = getRole(u)
        ret.append(dic)
    return ret

def isInitialSetup():

    numOfUsers = 0
    try:
        numOfUsers = len(getUsernames())
    except sqlalchemy.exc.OperationalError:
        None

    return numOfUsers == 0

class PolicyEvents(db.Model):
    __bind_key__ = 'policyEvents'
    __tablename__ = 'PolicyEvents'

    ts = db.Column(db.Float)
    uid = db.Column(db.String(200), primary_key=True)
    severity = db.Column(db.String(50))
    reason = db.Column(db.String(500))
    polNum = db.Column(db.Integer)
    acknowledgedBy = db.Column(db.Integer)

    def getCol(col):
        if col == "timestamp":
            return PolicyEvents.ts
        if col == "uid":
            return PolicyEvents.uid
        if col == "severity":
            return PolicyEvents.severity
        if col == "reason":
            return PolicyEvents.reason
        if col == "polNum":
            return PolicyEvents.polNum
        if col == "ackBy":
            return PolicyEvents.acknowledgedBy

def getPolicyEvents(amt, orderBy="timestamp", sort="desc", theFilter=None, severity="all", att=0):
    att += 1
    try:
        theCol = getattr(PolicyEvents.getCol(orderBy), sort)()
        q = PolicyEvents.query
        if theFilter == "acked":
            q = q.filter(PolicyEvents.acknowledgedBy > -1)
        elif theFilter == "unacked":
            q = q.filter(PolicyEvents.acknowledgedBy == -1)
        if severity == "notice":
            q = q.filter(PolicyEvents.severity == "notice")
        elif severity == "warning":
            q = q.filter(PolicyEvents.severity == "warning")
        elif severity == "alert":
            q = q.filter(PolicyEvents.severity == "alert")
        q = q.order_by(theCol).limit(amt)
        return [x for x in q]
    except sqlalchemy.exc.OperationalError as e:
        if att > 10:
            return []
        return getPolicyEvents(amt, orderBy, sort, theFilter, severity, att)
    except Exception as e:
        print("Exception: {}".format(e))
        return []

def getPolicyEventsCount(amt, orderBy="timestamp", sort="desc", theFilter=None, severity="all", att=0):
    att += 1
    try:
        theCol = getattr(PolicyEvents.getCol(orderBy), sort)()
        q = PolicyEvents.query
        if theFilter == "acked":
            q = q.filter(PolicyEvents.acknowledgedBy > -1)
        elif theFilter == "unacked":
            q = q.filter(PolicyEvents.acknowledgedBy == -1)
        if severity == "notice":
            q = q.filter(PolicyEvents.severity == "notice")
        elif severity == "warning":
            q = q.filter(PolicyEvents.severity == "warning")
        elif severity == "alert":
            q = q.filter(PolicyEvents.severity == "alert")
        q = q.count()
        return [x for x in q]
    except sqlalchemy.exc.OperationalError as e:
        if att > 10:
            return []
        return getPolicyEvents(amt, orderBy, sort, theFilter, severity, att)
    except Exception as e:
        print("Exception: {}".format(e))
        return []

def getAllPolicyEvents(amt, orderBy="timestamp", sort="desc", att=0):
    att += 1
    try:
        theCol = getattr(PolicyEvents.getCol(orderBy), sort)()
        return [x for x in PolicyEvents.query.order_by(theCol).limit(amt)]
    except sqlalchemy.exc.OperationalError as e:
        if att > 10:
            return []
        return getAllPolicyEvents(amt, orderBy, sort, att)
    except Exception as e:
        print("Exception: {}".format(e))
        return []

def getUnAckedPolicyEvents(amt, orderBy="timestamp", sort="desc", att=0):
    att += 1
    try:
        theCol = getattr(PolicyEvents.getCol(orderBy), sort)()
        return [x for x in PolicyEvents.query.filter(PolicyEvents.acknowledgedBy == -1).order_by(theCol).limit(amt)]
    except sqlalchemy.exc.OperationalError as e:
        if att > 10:
            return []
        return getUnAckedPolicyEvents(amt, orderBy, sort, att)
    except Exception as e:
        print("Exception: {}".format(e))
        return []

def getAckedPolicyEvents(amt, orderBy="timestamp", sort="desc", att=0):
    att += 1
    try:
        theCol = getattr(PolicyEvents.getCol(orderBy), sort)()
        return [x for x in PolicyEvents.query.filter(PolicyEvents.acknowledgedBy > -1).order_by(theCol).limit(amt)]
    except sqlalchemy.exc.OperationalError as e:
        if att > 10:
            return []
        return getAckedPolicyEvents(amt, orderBy, sort, att)
    except Exception as e:
        print("Exception: {}".format(e))
        return []

def getAllPolicyEventsCount(att=0):
    att += 1
    try:
        return PolicyEvents.query.count()
    except sqlalchemy.exc.OperationalError as e:
        if att > 10:
            return []
        return getAllPolicyEventsCount(att)
    except Exception as e:
        print("Exception: {}".format(e))
        return -1

def getUnAckedPolicyEventsCount(att=0):
    att += 1
    try:
        return PolicyEvents.query.filter(PolicyEvents.acknowledgedBy == -1).count()
    except sqlalchemy.exc.OperationalError as e:
        if att > 10:
            return []
        return getUnAckedPolicyEventsCount(att)
    except Exception as e:
        print("Exception: {}".format(e))
        return -1

def getAckedPolicyEventsCount(att=0):
    att += 1
    try:
        return PolicyEvents.query.filter(PolicyEvents.acknowledgedBy > -1).count()
    except sqlalchemy.exc.OperationalError as e:
        if att > 10:
            return []
        return getAckedPolicyEventsCount(att)
    except Exception as e:
        print("Exception: {}".format(e))
        return -1

def ackPolicyEvents(uids, userId, att=0):
    att += 1
    try:
        for theUid in uids.split(','):
            theRow = PolicyEvents.query.filter(PolicyEvents.uid==theUid).update({"acknowledgedBy": userId})
            db.session.commit()
    except sqlalchemy.exc.OperationalError as e:
        if att > 10:
            return []
        return ackPolicyEvents(uids, userId, att)
    except Exception as e:
        print("Exception: {}".format(e))
        return []
