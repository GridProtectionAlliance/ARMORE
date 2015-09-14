# Add backend lib to path
import sys
sys.path.append("../lib")

# Import backend library functions
from common import *
from bro import *

def reportConfig():
    # Define dictionaries which will be encoded as json objects
    fullDict = {}
    interfaces = []
    statusDict = {}

    fullDict["enabled"] = getBroStatus()
    fullDict["logging"] = getLoggingConfig()
    fullDict["validation"] = getValidationConfig()
    fullDict["interfaces"] = getMonitoredInterfaces()
    fullDict["polices"] = getPoliciesConfig()

    fullJsonMsg = getJsonStrFormatted(fullDict)
    return fullJsonMsg

#print(getConfigStatus())
