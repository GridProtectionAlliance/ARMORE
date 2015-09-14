# # # # #
# common.py
#
# Contains methods used across
# multiple backend files
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

from datetime import datetime
from subprocess import check_output
import json

def getCurrentTimestampAsStr():
    return str(datetime.now())

def getTemplateAsString(pathToTemplate):
    with open(pathToTemplate, "r") as temp:
        tString = temp.read()
    return tString

def getPocPhoneNumber():
    return "(555) 123-4567 (Test Number Only)"

def getLocation():
    return "Olympus Mons (Test Location Only)"

def getDescription():
    return "Winter getaway, snowfall rare (Test Description Only)"

def getJsonStrUnformatted(inputDict):
    return json.loads(json.JSONEncoder().encode(inputDict))

def getJsonStrFormatted(inputDict):
    return json.dumps(getJsonStrUnformatted(inputDict), sort_keys=True, indent=4)

def monToNum(mon):
    return {
            'Jan': 1,
            'Feb': 2,
            'Mar': 3,
            'Apr': 4,
            'May': 5,
            'Jun': 6,
            'Jul': 7,
            'Aug': 8,
            'Sep': 9,
            'Oct': 10,
            'Nov': 11,
            'Dec': 12,
            }[mon]

def toTimestamp(mon, day, hh, mm, ss, year=None):
    today = datetime.today()
    if year == None:
        year = today.year
        if mon > today.month:
            year -= 1
    return datetime(year, mon, day, hh, mm, ss).timestamp()

def cmd(command):
    return check_output(command.split(' ')).decode('utf-8')
