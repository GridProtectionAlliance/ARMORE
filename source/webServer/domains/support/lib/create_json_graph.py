import json
import datetime
import os
import sys
import argparse
import traceback
import locale
import time
from pymongo import MongoClient
from collections import defaultdict, OrderedDict
import socket
import struct
from enum import Enum

locale.setlocale(locale.LC_ALL, 'en_US.UTF-8')

STATS_COL_FILENAME = 'stats_col.log'
STATS_COUNT_FILENAME = 'stats_count.log'

# Database global variables
DB_NAME = 'armore'
COLLECTION_NAME = 'stats_jlogs'
dbclient = None

# Column headers for stats_col file
[TS_L, LEN_CNT_L, SENDER_L, RECEIVER_L, PROTO_L, UID_L, FUNC_L, TARGET_L, IS_RESP_L] = range(9)

# Column headers for stats_count file
[PERIOD_ID, TS, INTV1, SENDER, RECEIVER, PROTO, UID, FUNC, TARGET, NUM, BYTES, IS_REQ, NUM_RESP, AVG_DELAY] = range(14)

def connect_to_db():
    dbclient = MongoClient('mongodb://localhost')
    db = dbclient[DB_NAME]
    collection = db[COLLECTION_NAME]
    return db, collection

def reset_db():
    '''
    Remove all json dara stored
    '''
    # Delete collection
    try:
        print('Connecting to mongoDB...')
        db, stats_collection = connect_to_db()
        print('Deleting all records...')
        db.drop_collection(COLLECTION_NAME)
#         print(db.collection_names())
    except Exception as e:  
        print('ERROR> Resetting stats_logs collection. '+str(e))
        return False
    
def add_to_db(jdic):
    if not jdic:
        return False
    try:  
        print('Connecting to mongoDB...')
        db, stats_collection = connect_to_db()

        creation_time = jdic['created_at']
        print('new logs creation_time: '+creation_time+' with existing '+str(get_times()))
        if creation_time in get_times():
            print('ERROR> Document already exist. Keeping existing for date '+creation_time)
            return False
        print('Storing json dict in database')
        return stats_collection.insert(jdic)
    except Exception as e:  
        print('ERROR> Storing json dict to database. '+str(e))
        return False

def get_documents():
   
    try:  
        print('Connecting to mongoDB...')
        db, stats_collection = connect_to_db()
        docs = stats_collection.find()
        for doc in docs:
            print(doc.keys())
        return docs
    except Exception as e:  
        print('ERROR> Getting documents '+str(e))
        return False

def get_times():
    '''
    Retrieve list of creation times of all documents
    '''
    times = []
    try:  
        print('Connecting to mongoDB...')
        db, stats_collection = connect_to_db()
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

def process_stats_col_log(in_path=STATS_COL_FILENAME):
    nodes = set()
    links = set()

    nodes_spec = {}
    links_spec = {}

    protocols = []

    ''' generate json data file for graph visualization

    :in_path: path/to/stats_col.log
    :out_path: path/to/json/output
    :returns: nothing

    '''
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
            links_spec[link]['proto'] = set()
            links_spec[link]['func'] = set()
            links_spec[link]['target'] = set()

        for line in lines:
            if line.startswith('#'): continue
            data = line.split()
            send, recv, proto, uid, func, target, is_response = data[SENDER_L:TARGET_L+2]

            if not protocol_filter(proto): continue
            nodes_spec[send]['s_cnt'] += 1
            nodes_spec[recv]['r_cnt'] += 1
            links_spec[(send, recv)]['cnt'] += 1
            if proto != '-':
                links_spec[(send, recv)]['proto'].add(proto)
                if proto not in protocols:
                    protocols.append(proto)
                nodes_spec[send]['protocols'].add(proto)
            if func != '-':
                links_spec[(send, recv)]['func'].add(func)
                nodes_spec[send]['funcsrc'].add(func)
                nodes_spec[recv]['funcdst'].add(func)
            if target != '-':
                links_spec[(send, recv)]['target'].add(target)
                nodes_spec[send]['sourceof'].add(recv)
                nodes_spec[recv]['targetof'].add(send)
                if is_response == 'T':
                    nodes_spec[send]['nodetype'] = 'slave'
                nodes_spec[send]['targets'].add(proto+':'+target)

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
        j_links.append({
            'id': j,
            'source': nodes.index(link[0]),
            'target': nodes.index(link[1]),
            'value': links_spec[link]['cnt'],
            'PROTOCOL': [ {'name': proto} for proto in links_spec[link]['proto'] ],
            'FUNCTION': [ {'name': func} for func in links_spec[link]['func'] ],
            'TARGET': [ {'name': target} for target in links_spec[link]['target'] ],
        })
        j += 1

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
                       'globals': {'protocols':protocols}}
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
    elif statscol and statscount and timestamp:
        colf = statscol
        countf = statscount
        print(colf)
        print(countf)
    else:
        print('ERROR> Missing input. Please provide either an input directory or file paths')
        return False

    if not os.path.isfile(countf):
        print('ERROR> '+STATS_COUNT_FILENAME+' is missing')
        return False
    if not os.path.isfile(colf):
        print('ERROR> '+STATS_COL_FILENAME+' is missing')
        return False
    
    # Produce json dictionaries
    jcount_dict = process_stats_count_log(countf)
    jcol_dict = process_stats_col_log(colf)
    
    failure = False
    if not jcount_dict:
        print('ERROR> Failed to produce stats_count.json')
        failure = True
    if not jcol_dict:
        print('ERROR> Failed to produce stats_col.json')
        failure = True

    if failure:
        return False
    
    json_dict = {'created_at' : stats_dir if not timestamp else timestamp,
                 'count_log' : jcount_dict,
                 'col_log' : jcol_dict}
    
    # Store in database
    return add_to_db(json_dict)

def main(argv):
    parser = argparse.ArgumentParser()
    
    parser.add_argument('-t', '--timestamp', help='name to give the json data to be stored in the database')
    
    subparsers = parser.add_subparsers(help='Subcommands')

    # create the parser for the 'db' command
    parser_d = subparsers.add_parser('db', help='Edit database')
    parser_d.add_argument('-d', '--delete',  action='store_true', help='delete all records from database')
    
    # create the parser for the 'input_directory' command
    parser_a = subparsers.add_parser('directory', help='Indicate directory containing files named stats_col.log and stats_count.log file')
    parser_a.add_argument('-i', '--inputdir', help='path to directory containing files named stats_col.log and stats_count.log file', default='')

# create the parser for the 'input_files' command
    parser_b = subparsers.add_parser('files', help='Indicate individual stats files with arbitrary names')
    parser_b.add_argument('-l', '--statscol', help='path to stats_col.log file', default='')
    parser_b.add_argument('-c', '--statscount', help='path to stats_count.log file', default='')
    
    opt = parser.parse_args()
    print(opt)
    try:
        if 'delete' in opt and opt.delete:
            reset_db()
        elif 'inputdir' in opt and opt.inputdir:
            # Process input directory and store produced json dict in mongoDB
            process_stats_logs(opt.inputdir, None, None, opt.timestamp if 'timestamp' in opt else None)
        elif 'statscount' in opt and 'statscol' in opt and opt.statscount and opt.statscol:
            now = datetime.datetime.now()
            process_stats_logs(None, opt.statscol, opt.statscount, opt.timestamp if 'timestamp' in opt else str(now))
        else:
            print(parser.parse_args(['--help']))
    except BaseException:
        pass
    except:
        traceback.print_exc()

if __name__ == '__main__':
    main(sys.argv[1:])



