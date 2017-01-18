# # # # #
# visualization.py
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
from pymongo import MongoClient
from domains.support.lib.common import *
from domains.lib.create_json_graph import LogDBUtils
import json, pprint
from bson import json_util
from bson.json_util import dumps
from flask import Blueprint, render_template, session, request, make_response

MONGODB_HOST = 'localhost'
MONGODB_PORT = 27017
DBS_NAME = 'armore'
JLOGS_COLLECTION = 'stats_jlogs'
ANOMALY_COLLECTION = 'stats_anomalies'

visualDomain = Blueprint("visualization", __name__)

@visualDomain.route("/visualization")
@secure(["admin","user"])
def visualizePage():
    return render_template("visualization.html",
        common = sysLib.getCommonInfo({"username": session["username"]}, "visualization"),
    )

@visualDomain.route("/stats_anomalies")
def stats_anomalies(collectionTime=None):
    collectionTime = request.args.get('collectionTime')
    connection = MongoClient(MONGODB_HOST, MONGODB_PORT)
    collection = connection[DBS_NAME][ANOMALY_COLLECTION]
    if collectionTime:
        docs = collection.find({'created_at':collectionTime});
    else:
        docs = collection.find()
    json_docs = []
    for json_doc in docs:
        json_docs.append(json_doc)
    json_docs = json.dumps(json_docs, default=json_util.default)
    connection.close()
    return json_docs

@visualDomain.route("/stats_jlogs")
def stats_jlogs(collectionTime=None):
    collectionTime = request.args.get('collectionTime')
    connection = MongoClient(MONGODB_HOST, MONGODB_PORT)
    collection = connection[DBS_NAME][JLOGS_COLLECTION]
    if collectionTime:
        docs = collection.find({'created_at':collectionTime});
    else:
        docs = collection.find()
    json_docs = []
    for json_doc in docs:
        json_docs.append(json_doc)
    json_docs = json.dumps(json_docs, default=json_util.default)
    connection.close()
    return json_docs

@visualDomain.route("/stats_jlogs_times")
def stats_jlogs_times():
    connection = MongoClient(MONGODB_HOST, MONGODB_PORT)
    collection = connection[DBS_NAME][JLOGS_COLLECTION]
    docs = collection.distinct('created_at')
    json_docs = []
    for json_doc in docs:
        json_docs.append(json_doc)
    json_docs = json.dumps(json_docs, default=json_util.default)
    connection.close()
    return json_docs

@visualDomain.route("/updateNode", methods=["POST"])
def updateNode():
    timestamp = request.form.get("timestamp", None)
    nodeName = request.form.get("nodeName", None)
    nodeLabel = request.form.get("nodeLabel", None)
    nodeType = request.form.get("nodeType", None)
    connection = MongoClient(MONGODB_HOST, MONGODB_PORT)
    collection = connection[DBS_NAME][JLOGS_COLLECTION]
    if nodeLabel != None:
        res = collection.update_one(
            {"created_at":timestamp, "col_log.nodes.name":nodeName},
            {"$set" : {"col_log.nodes.$.label":nodeLabel}})
    if nodeType != None:
        res = collection.update_one(
            {"created_at":timestamp, "col_log.nodes.name":nodeName},
            {"$set" : {"col_log.nodes.$.node_type":nodeType}})
    connection.close()
    resp = make_response('{"matched": '+str(res.matched_count)+'}')
    resp.headers['Content-Type'] = "application/json"
    return resp

@visualDomain.route("/compare", methods=["GET"])
def compareDates():
    timestamp1 = request.args.get("timestamp1", None)
    timestamp2 = request.args.get("timestamp2", None)
    if timestamp1 and timestamp2:
        res = LogDBUtils.diffDocuments(timestamp1, timestamp2)
#         pprint.pprint(res)
        return json.dumps([res], default=json_util.default)
    return ''

