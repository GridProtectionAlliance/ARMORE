import json
import datetime
import os
import sys
import argparse
import traceback
import locale
import time
import copy
import pprint
from jsondiff import diff
from pymongo import MongoClient
from collections import defaultdict, OrderedDict
import socket
from traceback import print_exc

locale.setlocale(locale.LC_ALL, 'en_US.UTF-8')

STATS_COL_FILENAME = 'stats_col.log'
STATS_COUNT_FILENAME = 'stats_count.log'

# Database global variables
DB_NAME = 'armore'
JLOGS_COLLECTION = 'stats_jlogs'
ANOMALY_COLLECTION = 'stats_anomalies'
dbclient = None

def connect_to_db(c = JLOGS_COLLECTION):
    dbclient = MongoClient('mongodb://localhost')
    db = dbclient[DB_NAME]
    collection = db[c]
    return db, collection

def reset_db(coll=JLOGS_COLLECTION):
    '''
    Remove all json dara stored
    '''
    # Delete collection
    try:
        db, stats_collection = connect_to_db(coll)
        print('Deleting all records...')
        db.drop_collection(coll)
    except Exception as e:  
        print('ERROR> Resetting collection '+coll+'. '+str(e))
        return False
    
def remove_from_db(ts, coll=JLOGS_COLLECTION):
    if not ts:
        return reset_db()
    try:  
        db, collection = connect_to_db(coll)

        print('removing from collection '+coll+' all data created on: '+ts+' from '+str(get_times(coll)))
        if ts in get_times(coll):
            return collection.delete_one({'created_at':ts})
        else:
            print('unknown logs created on: '+ts+' in collection '+coll)
    except Exception as e:  
        print('ERROR> Deleting collection '+str(e))
        return False
    
def add_to_db(jdic, coll=JLOGS_COLLECTION):
    if not jdic:
        print('ERROR> Empty json dic. Cannot add to DB')
        return False
    try:  
        db, stats_collection = connect_to_db(coll)

        creation_time = jdic['created_at']
        print('new logs creation_time: '+creation_time+' with existing '+str(get_times(coll)))
        if creation_time in get_times(coll):
            print('ERROR> Document already exist. Keeping existing for date '+creation_time)
            return False
        print('Storing json dict in database')
        return stats_collection.insert(jdic)
    except Exception as e:  
        print('ERROR> Storing json dict to database. '+str(e))
        return False

def get_documents(coll=JLOGS_COLLECTION):
   
    try:  
        print('Connecting to mongoDB...')
        db, stats_collection = connect_to_db(coll)
        docs = stats_collection.find()
        for doc in docs:
            print(doc.keys())
        return docs
    except Exception as e:  
        print('ERROR> Getting documents '+str(e))
        return False

def get_times(coll=JLOGS_COLLECTION):
    '''
    Retrieve list of creation times of all documents
    '''
    times = []
    try:  
        db, stats_collection = connect_to_db(coll)
        docs = stats_collection.find()
        for doc in docs:
            times.append(doc['created_at'])
        return times
    except Exception as e:  
        print('ERROR> Getting document times '+str(e))
        return False
    
def protocol_filter(proto):
    ''' filter only modbus/dnp3 traffic
    '''
    PROTO_FILTER = ['modbus', 'dnp3']
    return proto.lower() in PROTO_FILTER

def get_ip(ip):
    ''' IP address
    '''
    try:
        return socket.inet_aton(ip)
    except Exception as e:
        #return socket.inet_pton(socket.AF_INET6,ip)
        print('IPv6 address')

def process_stats_count_log(in_path=STATS_COUNT_FILENAME):
    '''
    Syntax of input file is:
    #fields    period_id    ts    period    sender    receiver    protocol    uid    func    target    num    bytes    is_request    num_response    avg_delay
    #types    count    time    interval    string    string    string    string    string    string    count    double    bool    count    interval
    '''
    
    # Column headers for stats_count file
    [PERIOD_ID, TS, INTV1, SENDER, RECEIVER, PROTO, UID, FUNC, TARGET, NUM, BYTES, IS_REQ, NUM_RESP, AVG_DELAY] = range(14)

    links_missing_proto = comments = 0
    print('Computing - '+STATS_COUNT_FILENAME+' at '+datetime.datetime.now().time().isoformat()+'...')
    root = {}
    trees = []
    timestamp_to_senders = defaultdict(list)
    sender_to_receivers = defaultdict(dict)
    with open(in_path) as infile:
        lines = infile.readlines()
        print(str(len(lines))+' lines in input file ' + in_path)
        dsender = dreceiver = dproto = dfun = dtarget = ts_tree = None
        timestamps = []
        for line in lines:
            try:
                line = line.strip().replace('\t',' ')
                if line.startswith('#') or not line.strip():
                    comments += 1
                    continue
                data = line.split(' ')
    #             print(data)
                period_id = data[PERIOD_ID]
                ts = data[TS]
                proto = data[PROTO]
                protoid = data[UID]
                if proto == '-': 
                    links_missing_proto += 1
                    continue
                src = data[SENDER]
                dst = data[RECEIVER]
                func = data[FUNC]
                target = data[TARGET]
                formatted_time = time.strftime('%b %d %Y %H:%M:%S', time.gmtime(float(ts)))
    
                # Build dictionary, with values as they are encountered
                if formatted_time not in root.keys():
                    root[formatted_time] = {}
                    timestamps.append(formatted_time)
                if src and src != '-' and src not in root[formatted_time].keys():
                    root[formatted_time][src] = {}
                if dst and dst != '-' and dst not in root[formatted_time][src].keys():
                    root[formatted_time][src][dst] = {}
                if proto  != '-':
                    if proto not in root[formatted_time][src][dst]:
                        root[formatted_time][src][dst][proto] = {}
                if protoid  != '-': # only making sense in Modbus case
                    if protoid not in root[formatted_time][src][dst][proto]:
                        root[formatted_time][src][dst][proto][protoid] = {}
                if func  != '-':
                    if protoid  != '-':
                        if func not in root[formatted_time][src][dst][proto][protoid]:
                            root[formatted_time][src][dst][proto][protoid][func] = []
                    elif func not in root[formatted_time][src][dst][proto]:
                        root[formatted_time][src][dst][proto][func] = []
                if target  != '-':
                    if protoid  != '-':
                        if target not in root[formatted_time][src][dst][proto][protoid][func]:
                            root[formatted_time][src][dst][proto][protoid][func].append(target)
                    elif target not in root[formatted_time][src][dst][proto][func]:
                        root[formatted_time][src][dst][proto][func].append(target)
            except Exception as e:
                print(line)
                traceback.print_exc()
        # Now that all have been extracted, sort them out and build ordered json dict
        for ts, sdrd in root.items():
            ts_tree = {'name':ts, 'type':'timestamp_root', 'children':[]}
            trees.append(ts_tree)
            for sdr in sdrd.keys():
#             for sdr in sorted(sdrd.keys(), key=lambda ip: struct.unpack('!L', get_ip(ip))[0]):
                rcvrd = sdrd[sdr]
                dsender = {'name':sdr, 'type':'sender', 'children':[]}
                ts_tree['children'].append(dsender)
                for rcvr in rcvrd.keys():
#                 for rcvr in sorted(rcvrd.keys(), key=lambda ip: struct.unpack('!L', get_ip(ip))[0]):
                    protod = rcvrd[rcvr]
                    dreceiver = {'name':rcvr, 'type':'receiver', 'children':[]}
                    dsender['children'].append(dreceiver)
                    for proto, protoids in protod.items():
                        dproto = {'name':proto, 'type':'proto', 'children':[]}
                        dreceiver['children'].append(dproto)
                        for protoid, funcd in protoids.items():
                            dprotoid = {'name':protoid, 'type':'protoid', 'children':[]}
                            dproto['children'].append(dprotoid)
                            for func, trgts in funcd.items():
                                dfunc = {'name':func, 'type':'func', 'children':[]}
                                dprotoid['children'].append(dfunc)
                                for trgt in trgts:
                                    dtrgt = {'name':trgt, 'type':'target', 'children':[]}
                                    dfunc['children'].append(dtrgt)

    jtree = {}
    if trees:
        jtree = {'name':'root','children':trees}
    print('Processed ' 
          + locale.format('%d',len(lines), grouping=True) 
          + ' lines excluding ' 
          + locale.format('%d',links_missing_proto, grouping=True) 
          + ' missing protocol and '
          + str(comments)
          +' comments.')

    return jtree

def process_stats_anomaly_log(in_path, timestamp):
    '''
    Parse anomaly log for a given time period
    input file format is:
        #fields    period_id    ts    period    sender    receiver    protocol    uid    func    target    field    desc    score    average    std    current
        #types    count    time    interval    string    string    string    string    string    string    string    enum    double    double    double    double
    '''
        # Column headers for anomaly file
    [PERIOD, TS, INTV, SENDER, RECEIVER, PROTO, UID, FUNC, TARGET, FIELD, DESC, SCORE, AVG, STD, CURRENT] = range(15)

    if not os.path.exists(in_path):
        return
    print('Importing anomalies from - '+in_path+' at '+datetime.datetime.now().time().isoformat()+'...')
    comments = missing_proto = total_anomalies = 0
    anomalies = []
    ano_dic = {'created_at':timestamp, 'anomalies':anomalies} # Maps anomalies keyed by node name
    with open(in_path) as f:
        lines = f.readlines()
        for line in lines:
            try:
                if line.startswith('#'):
                    comments += 1
                    continue
                data = line.split()
                if data[PROTO] == '-': 
                    missing_proto += 1
                    continue
                sender, receiver = data[SENDER], data[RECEIVER]

                # Store anomalies by sender-receiver pair
                k = sender+'-'+receiver
                anomaly = {'sender-receiver':k,
                           'proto':data[PROTO],
                           'uid':data[UID],
                           'func':data[FUNC],
                           'target':data[TARGET],
                           'field':data[FIELD],
                           'desc':data[DESC],
                           'score':data[SCORE],
                           'avg':data[AVG],
                           'std':data[STD],
                           'current':data[CURRENT]
                           }
                total_anomalies += 1
                anomalies.append(anomaly)
            except:
                print('ERROR> Failed to process anomaly: '+line)
                traceback.print_exc()
                continue

    print('Processed ' 
          + locale.format('%d',len(lines), grouping=True) 
          + ' lines excluding ' 
          + locale.format('%d',missing_proto+comments, grouping=True) 
          + ' missing protocol/comments and '
          + str(total_anomalies)
          + ' anomalies.')
    print(ano_dic)
    # Store baseline in database
    return add_to_db(ano_dic, ANOMALY_COLLECTION)
            
def process_stats_baseline_log(in_path):
    ''' parse baseline log for comparison with stats_logs
    Will add json to DB under timestamp 'baseline'
    :in_path: path/to/stats_bn.log
    :out_path: path/to/json/output
    :returns: nothing
    
    Syntax of input file is:
    #fields    sender    receiver    protocol    uid    func    target    num_avg    num_std    bytes_avg    bytes_std    ratio_res_avg    ratio_res_std    avg_delay_avg    avg_delay_std
    #types    string    string    string    string    string    string    double    double    double    double    double    double    double    double

    '''
    # Column headers for baseline file
    [SENDER, RECEIVER, PROTO, UID, FUNC, TARGET, NUM_AVG, NUM_STD, BYTES_AVG, BYTES_STD, RATIO_AVG, RATIO_STD, DELAY_AVG, DELAY_STD] = range(14)

    nodes = set()
    links = set()

    nodes_spec = {}
    links_spec = {}

    protocols = []
    targets = []
    functions = []

    jtree = {'name':'baseline','children':[]}

    if not os.path.exists(in_path):
        return
    print('Computing - '+in_path+' at '+datetime.datetime.now().time().isoformat()+'...')
    links_missing_proto = comments = 0
    with open(in_path) as f:
        lines = f.readlines()
        for line in lines:
            if line.startswith('#'):
                comments += 1
                continue
            data = line.split()
            if data[PROTO] == '-': 
                links_missing_proto += 1

            send, recv = data[SENDER], data[RECEIVER]

            nodes.add(send)
            nodes.add(recv)

            links.add((send, recv))

        nodes = list(nodes)
        links = list(links)

        for node in nodes:
            nodes_spec[node] = {}
            nodes_spec[node]['s_cnt'] = 0
            nodes_spec[node]['r_cnt'] = 0
            nodes_spec[node]['protocols'] = set()
            nodes_spec[node]['sourceof'] = set()
            nodes_spec[node]['targetof'] = set()
            nodes_spec[node]['funcsrc'] = set()
            nodes_spec[node]['funcdst'] = set()
            nodes_spec[node]['targets'] = set()
            nodes_spec[node]['nodetype'] = 'master'
            
        for link in links:
            links_spec[link] = {}
            links_spec[link]['cnt'] = 0
            links_spec[link]['proto'] = []
            links_spec[link]['func'] = []
            links_spec[link]['target'] = []
            links_spec[link]['uid'] = []
            links_spec[link]['navg'] = []
            links_spec[link]['nstd'] = []
            links_spec[link]['bavg'] = []
            links_spec[link]['bstd'] = []
            links_spec[link]['ravg'] = []
            links_spec[link]['rstd'] = []
            links_spec[link]['davg'] = []
            links_spec[link]['dstd'] = []

        for line in lines:
            try:
                if line.startswith('#'): continue
                data = line.split()
                #NUM_AVG, NUM_STD, BYTES_AVG, BYTES_STD, RATIO_AVG, RATIO_STD, DELAY_AVG, DELAY_STD
                send, recv, proto, uid, func, target, navg, nstd, bavg, bstd, ravg, rstd, davg, dstd = data[SENDER:DELAY_STD+1]
                    
#                 print('\n'+send+'-'+recv)
                    
                nodes_spec[send]['s_cnt'] += 1
                nodes_spec[recv]['r_cnt'] += 1
                links_spec[(send, recv)]['cnt'] += 1
                    
                    
                links_spec[(send, recv)]['proto'].append(proto)
                if proto != '-':
                    nodes_spec[send]['protocols'].add(proto)
                    if proto not in protocols:
                        protocols.append(proto)
                
                func = proto+':'+func
                links_spec[(send, recv)]['func'].append(func)
                if func != '-':
                    nodes_spec[send]['funcsrc'].add(func)
                    nodes_spec[recv]['funcdst'].add(func)
                    if func not in functions:
                        functions.append(func)
                        

                target = proto+':'+target
                links_spec[(send, recv)]['target'].append(target)
                if target != '-':
                    if target not in targets:
                            targets.append(target)
                    nodes_spec[send]['sourceof'].add(recv)
                    nodes_spec[recv]['targetof'].add(send)
                    nodes_spec[send]['targets'].add(proto+':'+target)

#                 print (send, recv, navg, nstd, bavg, bstd, ravg, rstd, davg, dstd )
                
                links_spec[(send, recv)]['uid'].append(uid if uid else '-')
                links_spec[(send, recv)]['navg'].append(navg if navg else '-')
                links_spec[(send, recv)]['nstd'].append(nstd if nstd else '-')
                links_spec[(send, recv)]['bavg'].append(bavg if bavg else '-')
                links_spec[(send, recv)]['bstd'].append(bstd if bstd else '-')
                links_spec[(send, recv)]['ravg'].append(ravg if ravg else '-')
                links_spec[(send, recv)]['rstd'].append(rstd if rstd else '-')
                links_spec[(send, recv)]['davg'].append(davg if davg else '-')
                links_spec[(send, recv)]['dstd'].append(dstd if dstd else '-')
                
                pprint.pprint(links_spec[(send, recv)])
            except:
                traceback.print_exc()

    j_nodes = []
    j_links = []

    i = 0
    for node in nodes:
        j_nodes.append({
            'id': i,
            'name': node,
            'group': 1,
            'node_type' : nodes_spec[node]['nodetype'],
            'size': nodes_spec[node]['s_cnt'] + nodes_spec[node]['r_cnt'],
            's_cnt' : nodes_spec[node]['s_cnt'],
            'r_cnt' : nodes_spec[node]['r_cnt'],
            'protocols' : sorted(nodes_spec[node]['protocols']),
            'sourceof' : sorted(nodes_spec[node]['sourceof']),
            'targetof' : sorted(nodes_spec[node]['targetof']),
            'funcsrc' : sorted(nodes_spec[node]['funcsrc']),
            'funcdst' : sorted(nodes_spec[node]['funcdst']),
            'targets' : sorted(nodes_spec[node]['targets'])
        })
        i += 1

    j = 0
    for link in links:
        try:
            print(links_spec[link])
            src = nodes.index(link[0])
            dst = nodes.index(link[1])
            # For node graph
            j_links.append({
                'id': j,
                'source': src,
                'target': dst,
                'source_name': link[0],
                'target_name': link[1],
                'value': links_spec[link]['cnt'],
                'PROTOCOL': [{'name': proto} for proto in links_spec[link]['proto'] ],
                'FUNCTION': [ {'name': func} for func in links_spec[link]['func'] ],
                'TARGET': [ {'name': target} for target in links_spec[link]['target'] ],
                'uid': [ {'name': uid} for uid in links_spec[link]['uid'] ],
                'navg': [ {'name': navg} for navg in links_spec[link]['navg'] ],
                'nstd': [ {'name': nstd} for nstd in links_spec[link]['nstd'] ],
                'bavg': [ {'name': bavg} for bavg in links_spec[link]['bavg'] ],
                'bstd': [ {'name': bstd} for bstd in links_spec[link]['bstd'] ],
                'ravg': [ {'name': ravg} for ravg in links_spec[link]['ravg'] ],
                'rstd': [ {'name': rstd} for rstd in links_spec[link]['rstd'] ],
                'davg': [ {'name': davg} for davg in links_spec[link]['davg'] ],
                'dstd': [ {'name': dstd} for dstd in links_spec[link]['dstd'] ]
            })
            # For tree
            jtree['children'].append({'name':link[0]})
            j += 1
        except:
            traceback.print_exc()

    print('Processed ' 
          + locale.format('%d',len(lines), grouping=True) 
          + ' lines excluding ' 
          + locale.format('%d',links_missing_proto, grouping=True) 
          + ' missing protocol and '
          + str(comments)
          + ' comments.')

    jgraph = {}
    if j_nodes and j_links:
        jgraph = {'nodes': j_nodes,
                   'links': j_links,
                   'globals': {'protocols':protocols,
                               'functions':functions,
                               'targets':targets}}

    json_dict = {'created_at' : 'baseline',
                 'count_log' : jtree,
                 'col_log' : jgraph}
#     pprint.pprint (j_links)
    # Remove from DB any existing baseline
    remove_from_db('baseline')
    # Store baseline in database
    return add_to_db(json_dict)

def process_stats_col_log(in_path=STATS_COL_FILENAME):
    ''' generate json data file for graph visualization

    :in_path: path/to/stats_col.log
    :out_path: path/to/json/output
    :returns: nothing

    Syntax of input file is:
    #fields    ts    len    sender    receiver    protocol    uid    func    target    is_response
    #types    time    count    addr    addr    string    string    string    string    bool
    '''
    # Column headers for stats_col file
    [TS_L, LEN_CNT_L, SENDER_L, RECEIVER_L, PROTO_L, UID_L, FUNC_L, TARGET_L, IS_RESP_L] = range(9)

    nodes = set()
    links = set()

    nodes_spec = {}
    links_spec = {}

    protocols = []
    targets = []
    functions = []
    if not os.path.exists(in_path):
        return
    print('Computing - '+STATS_COL_FILENAME+' at '+datetime.datetime.now().time().isoformat()+'...')
    links_missing_proto = comments = 0
    with open(in_path) as f:
        lines = f.readlines()
        for line in lines:
            if line.startswith('#'):
                comments += 1
                continue
            data = line.split()
            if data[PROTO_L] == '-': 
                links_missing_proto += 1
                continue
            send, recv = data[SENDER_L], data[RECEIVER_L]
            nodes.add(send)
            nodes.add(recv)

            links.add((send, recv))

        nodes = list(nodes)
        links = list(links)

        for node in nodes:
            nodes_spec[node] = {}
            nodes_spec[node]['s_cnt'] = 0
            nodes_spec[node]['r_cnt'] = 0
            nodes_spec[node]['uids'] = set()
            nodes_spec[node]['protocols'] = set()
            nodes_spec[node]['sourceof'] = set()
            nodes_spec[node]['targetof'] = set()
            nodes_spec[node]['funcsrc'] = set()
            nodes_spec[node]['funcdst'] = set()
            nodes_spec[node]['targets'] = set()
            nodes_spec[node]['nodetype'] = 'master'
            
        for link in links:
            links_spec[link] = {}
            links_spec[link]['cnt'] = 0
            links_spec[link]['proto_func_target'] = ''
            links_spec[link]['uid'] = ''
            links_spec[link]['proto'] = set()
            links_spec[link]['func'] = set()
            links_spec[link]['target'] = set()

        for line in lines:
            try:
                if line.startswith('#'): continue
                data = line.split()
                send, recv, proto, uid, func, target, is_response = data[SENDER_L:TARGET_L+2]
    
                if not protocol_filter(proto): continue
                nodes_spec[send]['s_cnt'] += 1
                nodes_spec[recv]['r_cnt'] += 1
                links_spec[(send, recv)]['cnt'] += 1
                links_spec[(send, recv)]['proto_func_target'] = proto + ':' + func + ':' + target
                links_spec[(send, recv)]['uid'] = uid
                if proto != '-':
                    links_spec[(send, recv)]['proto'].add(proto)
                    if proto not in protocols:
                        protocols.append(proto)
                    nodes_spec[send]['protocols'].add(proto)
                
                if func != '-':
                    func = proto+':'+func
                    links_spec[(send, recv)]['func'].add(func)
                    nodes_spec[send]['funcsrc'].add(func)
                    nodes_spec[recv]['funcdst'].add(func)
                    if func not in functions:
                        functions.append(func)
                if target != '-':
                    target = proto+':'+target
                    links_spec[(send, recv)]['target'].add(target)
                    if target not in targets:
                        targets.append(target)
                    nodes_spec[send]['sourceof'].add(recv)
                    nodes_spec[recv]['targetof'].add(send)
                    if is_response == 'T':
                        nodes_spec[send]['nodetype'] = 'slave'
                        if uid:
                            nodes_spec[send]['uids'].add(str(uid))
                    nodes_spec[send]['targets'].add(proto+':'+target)
            except Exception as e:  
                print_exc()

    j_nodes = []
    j_links = []

    i = 0
    for node in nodes:
        j_nodes.append({
            'id': i,
            'name': node,
            'group': 1,
            'node_type' : nodes_spec[node]['nodetype'],
            'uids' : sorted(list(nodes_spec[node]['uids'])),
            'size': nodes_spec[node]['s_cnt'] + nodes_spec[node]['r_cnt'],
            's_cnt' : nodes_spec[node]['s_cnt'],
            'r_cnt' : nodes_spec[node]['r_cnt'],
            'protocols' : sorted(list(nodes_spec[node]['protocols'])),
            'sourceof' : sorted(list(nodes_spec[node]['sourceof'])),
            'targetof' : sorted(list(nodes_spec[node]['targetof'])),
            'funcsrc' : sorted(list(nodes_spec[node]['funcsrc'])),
            'funcdst' : sorted(list(nodes_spec[node]['funcdst'])),
            'targets' : sorted(list(nodes_spec[node]['targets']))
        })
        i += 1

    j = 0
    for link in links:
        try:
            j_links.append({
                'id': j,
                'source': nodes.index(link[0]),
                'target': nodes.index(link[1]),
                'source_name': link[0],
                'target_name': link[1],
                'proto_func_target': links_spec[link]['proto_func_target'],
                'uid': links_spec[link]['uid'],
                'value': links_spec[link]['cnt'],
                'PROTOCOL': [ {'name': proto} for proto in links_spec[link]['proto'] ],
                'FUNCTION': [ {'name': func} for func in links_spec[link]['func'] ],
                'TARGET': [ {'name': target} for target in links_spec[link]['target'] ],
            })
            j += 1
        except Exception as e:  
            print_exc()

    print('Processed ' 
          + locale.format('%d',len(lines), grouping=True) 
          + ' lines excluding ' 
          + locale.format('%d',links_missing_proto, grouping=True) 
          + ' missing protocol and '
          + str(comments)
          + ' comments.')

    jgraph = {}
    if j_nodes and j_links:
        jgraph = {'nodes': j_nodes,
                   'links': j_links,
                   'globals': {'protocols':protocols,
                               'functions':functions,
                               'targets':targets}}
    return jgraph

def process_stats_logs(stats_dir, statscol=None, statscount=None, timestamp=None):
    '''
    Convert stats log files to json dictionaries and store in
    mongoDB. Takes as input either:
    - a directory containing the 2 log files. Directory name
    will be used to name json data stored in MongoDB
    - the 2 file paths and a timestamp used to name data stored in DB 
    @param stats_dir: directory containing stats logs
    @param statscol: the path to the stats_col.log file
    @param statscount: the path to the stats_count.log file
    @param timestamp: what to name the resulting json data in the DB
    @return true on success, false otherwise
    '''
    colf = countf = None
    if stats_dir:
        if not os.path.isdir(stats_dir):
            print('ERROR> Invalid input directory')
            return False
    
        colf = os.path.join(stats_dir,STATS_COL_FILENAME)
        countf = os.path.join(stats_dir,STATS_COUNT_FILENAME)
    elif statscol or statscount and timestamp:
        colf = statscol
        countf = statscount
#     else:
#         print('ERROR> Missing input. Please provide either an input directory or file paths')
#         return False

    # Produce json dictionaries
    failure = False
    jcount_dict = jcol_dict = []

    if countf:
        if not os.path.isfile(countf):
            print('ERROR> '+STATS_COUNT_FILENAME+' is missing')
            return False
        jcount_dict = process_stats_count_log(countf)
#     print('jcount = '+str(jcount_dict))

    if colf:
        if not os.path.isfile(colf):
            print('ERROR> '+STATS_COL_FILENAME+' is missing')
            return False
        jcol_dict = process_stats_col_log(colf)
#     print('jcol = ' + str(jcol_dict))
    
    if countf and not jcount_dict:
        print('ERROR> Failed to produce stats_count.json')
        failure = True

    if colf and not jcol_dict:
        print('ERROR> Failed to produce stats_col.json')
        failure = True

    if failure:
        return False
    
    json_dict = {'created_at' : stats_dir if not timestamp else timestamp,
                 'count_log' : jcount_dict,
                 'col_log' : jcol_dict}
#     print (json_dict)
    # Store in database
    return add_to_db(json_dict)

class LogDBUtils:
    @staticmethod
    def findNodeByName(node, nodes):
        '''
        Search json col dictionary for node by name
        @type node (either a string for node name or the full node dict)
        @return the node dict for the node matching the providing name,
        None if no match was found
        '''
        node_name = node['name'] if type(node) == dict else node
        for nd in nodes:
            if nd['name'] == node_name:
                return nd
        return None

    @staticmethod
    def findLinkByEndpoints(link, jstatscol):
        '''
        Search json col dictionary for link by endpoint ids
        @param link: the original link
        @param links: the dict to search for match
        @return the link dict for the link matching
        None if no match was found
        '''
        for lk in jstatscol['links']:
            lksrc = lk['source_name']
            lkdst = lk['target_name']
            lksrcnd = LogDBUtils.findNodeByName(lksrc,jstatscol['nodes'])
            lkdstnd = LogDBUtils.findNodeByName(lkdst,jstatscol['nodes'])
            # Link exists in new dict, update node
            if lksrcnd and lkdstnd:
                return lk
        return None
    
    @staticmethod
    def diffColLogs (clg1, clg2, frm, to):
        print('Comparing nodes in col_logs... '+str(len(clg1['nodes']))+'-'+str(len(clg2['nodes'])))
        result = {'globals':{}, 'nodes':[], 'links':[]}
        node_mapping = {'inserted':{}, 'deleted':{}, 'same':{}}
        node_name_id = {} # 1x1 mapping node name to new id
        nm = ni = ndl = lm = li = ld = idx = 0 
        for orig_node in clg1['nodes']:
            other_nd = LogDBUtils.findNodeByName(orig_node, clg2['nodes'])
            if other_nd:
                df = diff(orig_node, other_nd, syntax='symmetric')
#                 pprint.pprint(df)
                # Add node to resulting dict assigning new node id
                rnd = copy.deepcopy(other_nd)
                rnd['id'] = idx
                result['nodes'].append(rnd)
                node_name_id[rnd['name']] = rnd['id']
                node_mapping['same'][rnd['name']] = rnd
                difrc = {'from':frm, 'to':to, 'diff':'modified'}
                if not 'diffs' in rnd:
                    rnd['diffs'] = []
                rnd['diffs'].append(difrc)
                idx += 1
                nm += 1
#                 print ('\n',orig_node['name']+' found\nORIG')
#                 pprint.pprint(orig_node)
#                 print('\nOTHER')
#                 pprint.pprint(other_nd)
#                 print('\nDIFF')
#                 pprint.pprint(df)
            else:
                # Add node to resulting dict assigning new node id
                rnd = copy.deepcopy(orig_node)
                rnd['id'] = idx
                result['nodes'].append(rnd)
                node_name_id[rnd['name']] = rnd['id']
                node_mapping['deleted'][rnd['name']] = rnd
                difrc = {'from':frm, 'to':to, 'diff':'deleted'}
                if not 'diffs' in rnd:
                    rnd['diffs'] = []
                rnd['diffs'].append(difrc)
                idx += 1
                ndl += 1
#                 print ('\n',orig_node['name']+' gone')    
        # Identify new nodes only
        for other_node in clg2['nodes']:
            if not LogDBUtils.findNodeByName(other_node, clg1['nodes']):
                # Add node to resulting dict assigning new node id
                rnd = copy.deepcopy(other_node)
                rnd['id'] = idx
                result['nodes'].append(rnd)
                node_name_id[rnd['name']] = rnd['id']
                node_mapping['inserted'][rnd['name']] = rnd
                difrc = {'from':frm, 'to':to, 'diff':'inserted'}
                if not 'diffs' in rnd:
                    rnd['diffs'] = []
                rnd['diffs'].append(difrc)
                idx += 1
                ni += 1

        # Build a new json combining both links
        matched_links = []
        def link_footprint(link):
            return link['source_name']+'-'+link['target_name']

        for lk in clg1['links']:
            rlk = None
            # Check if link still exists between these endpoint in new dict
            if LogDBUtils.findLinkByEndpoints(lk, clg2):
                if lk['source_name'] in node_name_id.keys() and\
                lk['target_name'] in node_name_id.keys():
                    # Update link with new endpoint ids and add difference found
                    rlk = copy.deepcopy(lk)
                    rlk['source'] = node_name_id[lk['source_name']]
                    rlk['target'] = node_name_id[lk['target_name']]
                    difrc = {'from':frm, 'to':to, 'diff':'modified'}
                    if not 'diffs' in rlk:
                        rlk['diffs'] = []
                    rlk['diffs'].append(difrc)
#                     print ('existing link',link_footprint(rlk))
            else: # Deleted link
                rlk = copy.deepcopy(lk)
                rlk['source'] = node_name_id[lk['source_name']]
                rlk['target'] = node_name_id[lk['target_name']]
                difrc = {'from':frm, 'to':to, 'diff':'deleted'}
                if not 'diffs' in rlk:
                    rlk['diffs'] = []
                rlk['diffs'].append(difrc)
#                 print ('deleted link',link_footprint(rlk))
            if rlk:
                result['links'].append(rlk)
                matched_links.append(link_footprint(rlk))
        # Look for new links
        for lk in clg2['links']:
            # Ignored, alread processed links
            if link_footprint(lk) in matched_links:
                continue
            rlk = copy.deepcopy(lk)
            rlk['source'] = node_name_id[lk['source_name']]
            rlk['target'] = node_name_id[lk['target_name']]
            difrc = {'from':frm, 'to':to, 'diff':'inserted'}
            if not 'diffs' in rlk:
                rlk['diffs'] = []
            rlk['diffs'].append(difrc)
#             print ('new link',link_footprint(lk) )
            if rlk:
                result['links'].append(rlk)
                
#         pprint.pprint(result['links'])
        
        # Merge all globals
        funcs1 = len(clg1['globals']['functions'])
        funcs2 = len(clg2['globals']['functions'])
        protos1 = len(clg1['globals']['protocols'])
        protos2 = len(clg2['globals']['protocols'])
        trgts1 = len(clg1['globals']['targets'])
        trgt2 = len(clg2['globals']['targets'])
        result['globals']['functions'] = [x for x in set(clg1['globals']['functions']+clg2['globals']['functions'])]
        result['globals']['protocols'] = [x for x in set(clg1['globals']['protocols']+clg2['globals']['protocols'])]
        result['globals']['targets'] = [x for x in set(clg1['globals']['targets']+clg2['globals']['targets'])]
        print('Identified '+
              str(nm)+'/'+str(ndl)+'/'+str(ni)+
              ' nodes and '+
              str(lm)+'/'+str(ld)+'/'+str(li)+
              ' links modified/deleted/inserted. '+
              str(funcs1 - funcs2) + ' functions, '+
              str(protos1 - protos2) + ' protocols, '+
              str(trgts1 - trgt2) + ' targets.'
              )
        result['globals']['changes'] = {
            'from':frm,
            'to':to,
            'nodes':{'new':ni, 'modified':nm,'deleted':ndl},
            'links':{'new':li, 'modified':lm,'deleted':ld}
            }
#         pprint.pprint(result['globals'])
        return result

    @staticmethod
    def diffCountLogs(cntlg1, cntlg2, frm, to):
        '''
        Compare 2 stats_count json dicts
        @return json dict with the original json dicts augmented with differences found
        '''
        #@todo 
        return cntlg1

    @staticmethod
    def diffDocuments (timestamp1, timestamp2):
        '''
        Compare 2 stats_jlogs json dicts
        @return json dict with the original json dicts augmented with differences found
        '''
        try:  
            print('Connecting to mongoDB...')
            db, stats_collection = connect_to_db()
            doc1 = stats_collection.find({'created_at':timestamp1})
            doc2 = stats_collection.find({'created_at':timestamp2})
            coldiff = LogDBUtils.diffColLogs(doc1[0]['col_log'], doc2[0]['col_log'], timestamp1, timestamp2)
            countdiff = LogDBUtils.diffCountLogs(doc1[0]['count_log'], doc2[0]['count_log'], timestamp1, timestamp2)
#             pprint.pprint(res)
            # Return a new json dump that ressembles the original (timestamp1 document)
            # but augmented with differences found
            res = doc1[0]
            res['col_log'] = coldiff
            return res
        except Exception as e:  
            print_exc()
            print('ERROR> Getting comparing documents '+str(e))
            return ''

def main(argv):
    parser = argparse.ArgumentParser()
    
    parser.add_argument('-t', '--timestamp', help='name to give the json data to be stored in the database', default=None)
    
    parser.add_argument('-b', '--baseline', help='path to the baseline file')
    
    parser.add_argument('-a', '--anomaly', help='path to an anomaly file')
    
    subparsers = parser.add_subparsers(help='Subcommands')

    # create the parser for the 'db' command
    parser_d = subparsers.add_parser('db', help='Edit database')
    parser_d.add_argument('-d', '--delete',  action='store_true', help='delete records from database (will delete all if -t not provided)')
    parser_d.add_argument('-c', '--collection', help='name of collection', default=JLOGS_COLLECTION)
    
    # create the parser for input directory
    parser_a = subparsers.add_parser('directory', help='Indicate directory containing files named stats_col.log and stats_count.log file')
    parser_a.add_argument('-i', '--inputdir', help='path to directory containing files named stats_col.log and stats_count.log file', default='')

    # create the parser for input files
    parser_b = subparsers.add_parser('files', help='Indicate individual stats files with arbitrary names')
    parser_b.add_argument('-l', '--statscol', help='path to stats_col.log file', default='')
    parser_b.add_argument('-c', '--statscount', help='path to stats_count.log file', default='')
    
    # create the parser for diff command
    parser_c = subparsers.add_parser('diff', help='Compare 2 dated records')
    parser_c.add_argument('-f', '--first', help='first timestamp', default='')
    parser_c.add_argument('-s', '--second', help='second timestamp', default='')
    
    opt = parser.parse_args()
#     print(opt)
    try:
        if 'delete' in opt and opt.delete:
            if 'timestamp' in opt and opt.timestamp != None:
                remove_from_db(opt.timestamp, opt.collection);
            else:
                reset_db(opt.collection)
        elif 'anomaly' in opt and opt.anomaly:
            if 'timestamp' in opt and opt.timestamp != None:
                process_stats_anomaly_log(opt.anomaly, opt.timestamp)
            else:
                print("Timestamp required to process anomalies.")
                print(parser.parse_args(['--help']))
        elif 'baseline' in opt and opt.baseline:
            process_stats_baseline_log(opt.baseline)
        elif 'inputdir' in opt and opt.inputdir:
            # Process input directory and store produced json dict in mongoDB
            process_stats_logs(opt.inputdir, None, None, opt.timestamp if 'timestamp' in opt else None)
        elif ('statscount' in opt or 'statscol' in opt) and (opt.statscount or opt.statscol):
            now = datetime.datetime.now()
            process_stats_logs(None, opt.statscol, opt.statscount, opt.timestamp if 'timestamp' in opt else str(now))
        elif 'first' in opt and 'second' in opt:
            LogDBUtils.diffDocuments(opt.first, opt.second)
        else:
            print(parser.parse_args(['--help']))
    except BaseException:
        pass
    except:
        traceback.print_exc()

if __name__ == '__main__':
    main(sys.argv[1:])



