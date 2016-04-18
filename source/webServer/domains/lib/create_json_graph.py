import json
import random
import os

INPATH = 'stats_col.log'
OUTPATH = 'data.json'

[TS, SENDER, RECEIVER, PROTO, FUNC, TARGET, IS_RESP] = range(7)

def protocol_filter(proto):
    """ filter only modbus/dnp3 traffic

    """
    PROTO_FILTER = ["modbus", "dnp3"]
    return proto.lower() in PROTO_FILTER

def visualize(in_path=INPATH, out_path=OUTPATH):

    if os.path.exists(in_path) and os.path.exists(out_path):
        if os.path.getmtime(in_path) < os.path.getmtime(out_path):
            return

    nodes = set()
    links = set()

    nodes_spec = {}
    links_spec = {}

    """ generate json data file for visualization

    :in_path: path/to/stats_col.log
    :out_path: path/to/json/output
    :returns: nothing

    """
    with open(in_path) as f:
        lines = f.readlines()
        for line in lines:
            if line.startswith("#"): continue
            data = line.split()
            send, recv = data[SENDER], data[RECEIVER]
            nodes.add(send)
            nodes.add(recv)

            links.add((send, recv))

        nodes = list(nodes)
        links = list(links)

        for node in nodes:
            nodes_spec[node] = {}
            nodes_spec[node]["s_cnt"] = 0
            nodes_spec[node]["r_cnt"] = 0
        for link in links:
            links_spec[link] = {}
            links_spec[link]["cnt"] = 0
            links_spec[link]["proto"] = set()
            links_spec[link]["func"] = set()
            links_spec[link]["target"] = set()

        for line in lines:
            if line.startswith("#"): continue
            data = line.split()
            send, recv, proto, func, target = data[SENDER:TARGET+1]

            if not protocol_filter(proto): continue
            nodes_spec[send]["s_cnt"] += 1
            nodes_spec[recv]["r_cnt"] += 1
            links_spec[(send, recv)]["cnt"] += 1
            if proto != '-':
                links_spec[(send, recv)]["proto"].add(proto)
            if func != '-':
                links_spec[(send, recv)]["func"].add(func)
            if target != '-':
                links_spec[(send, recv)]["target"].add(target)


    j_nodes = []
    j_links = []

    for node in nodes:
        j_nodes.append({
            "name": node,
            "group": 1,
            "size": nodes_spec[node]["s_cnt"] + nodes_spec[node]["r_cnt"]
        })

    for link in links:
        j_links.append({
            "source": nodes.index(link[0]),
            "target": nodes.index(link[1]),
            "value": links_spec[link]["cnt"],
            "PROTOCOL": [ {"name": proto} for proto in links_spec[link]["proto"] ],
            "FUNCTION": [ {"name": func} for func in links_spec[link]["func"] ],
            "TARGET": [ {"name": target} for target in links_spec[link]["target"] ],
        })


    with open(out_path, 'w') as outfile:
        json.dump({"nodes": j_nodes, "links": j_links}, outfile)




            




