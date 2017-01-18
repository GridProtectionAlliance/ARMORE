module Policy;

export {

    redef enum Log::ID += { LOG };

    type Info: record {
        ts: time            &log;
        uid: string	    &log;
        severity: string    &log;
        reason: string      &log;
        polNum: string      &log;
        acknowledgedBy: int &log &default=-1;
    };
}

type Idx: record {
   ruleNum: string;
};

type Val: record {
    ip: string;
    proto: string;
    func: string;
    timePer: string;
    severity: string;
    filtering: string;
};

type filter: record {
    src: subnet;
    dest: subnet;
    proto: string;
    funcRegex: string;
    func: string;
    timePer: string;
    severity: string;
    flagged: bool &default=F;
    polNum: string &default="-1";
};

global configTable: table[string] of Val = table();
global filtersOther: set[filter];
global filtersModbus: set[filter];
global filtersDnp3: set[filter];

function isBetween(t: time, f: vector of string): bool {
    local hour = to_int(strftime("%H", t));
    local min = to_int(strftime("%m", t));
    local sec = to_int(strftime("%S", t));

    local aHour = f[0];
    local aMin = f[1];
    local aSec = f[2];
    local bHour = f[3];
    local bMin = f[4];
    local bSec = f[5];

    if (aHour == "*") {
        aHour = "0";
    }
    if (bHour == "*") {
        bHour = "23";
    }
    if (aMin == "*") {
        aMin = "0";
    }
    if (bMin == "*") {
        bMin = "59";
    }
    if (aSec == "*") {
        aSec = "0";
    }
    if (bSec == "*") {
        bSec = "59";
    }

    local aTot = (to_int(aHour) * 3600) + (to_int(aMin) * 60) + to_int(aSec); 
    local bTot = (to_int(bHour) * 3600) + (to_int(bMin) * 60) + to_int(bSec);
    local tTot = (hour * 3600) + (min * 60) + sec;    

    if (tTot >= aTot && tTot <= bTot) {
        return T;
    }

    return F;
}

function verifyTime(t: time, f: string): bool {
    local flag: bool = F;
    local dow = strftime("%a", t);

    local times = split_string(f, /,/);

    for (x in times) {
        local fields = split_string(times[x], /:/);
        local fTime = vector(fields[1], fields[2], fields[3], fields[4], fields[5], fields[6]);

        if        (fields[0] == "Daily" && isBetween(t, fTime)) {
            flag = T;
        } else if (fields[0] == "Weekday" && /Mon|Tue|Wed|Thu|Fri/ in dow && isBetween(t, fTime)) {
            flag = T;
        } else if (fields[0] == "Weekend" && /Sat|Sun/ in dow && isBetween(t, fTime)) {
            flag = T;
        } else if (fields[0] == dow && isBetween(t, fTime)) {
            flag = T;
        }
    }

    return flag;
}

function checkPolicy(c: connection, t: string) {
    for (f in filtersOther) {
        local ipFlagged: bool = F;
        local protoFlagged: bool = F;
        local funcFlagged: bool = F;
        local timePerFlagged: bool = F;
        local reason = "";

        # IP
        if (c$id$orig_h in f$src && c$id$resp_h in f$dest) {
            ipFlagged = T;
            reason += fmt("IP=%s -> %s,", c$id$orig_h, c$id$resp_h);
        }
        
        # Protocol
	if (f$proto == "*") {
	   if (c?$modbus || c?$dnp3) {
	      protoFlagged = F;
	   } else {
	      protoFlagged = T;
	      reason += "Protocol=*,";
	   }
	}

	  if (f$func == "*") {
	       funcFlagged = T;
	       reason += fmt("Function=%s,", f$func);
	  }
 
        # Time Frame
        timePerFlagged = verifyTime(c$start_time, f$timePer);
        if (timePerFlagged) {
            reason += fmt("Time=%s,", c$start_time);
        }

        if (ipFlagged && protoFlagged && funcFlagged && timePerFlagged) {
            f$flagged = T;
            local rec: Policy::Info = [$ts=network_time(), $uid=unique_id(c$uid), $severity=f$severity, $reason=reason, $polNum=f$polNum]; 
            Log::write(Policy::LOG, rec);
	    #print fmt("%s - %s: True\n#\t%s\n#\t%s", c$start_time, t, reason, c);
        } else {
	    #print fmt("%s - %s: False - %s\n\t%s", c$start_time, t, reason, c);
	}

    }
}

function checkPolicyModbus(rec: Modbus::Info) {
    for (f in filtersModbus) {
        local ipFlagged: bool = F;
        local protoFlagged: bool = T;
        local funcFlagged: bool = F;
        local timePerFlagged: bool = F;
        local reason = "";

        # IP
        if (rec$id$orig_h in f$src && rec$id$resp_h in f$dest) {
            ipFlagged = T;
            reason += fmt("IP=%s -> %s,", rec$id$orig_h, rec$id$resp_h);
        }
        
        # Protocol
		reason += "Protocol=modbus,";

		# Function - Modbus
	    if (rec?$func) {
	        if (to_lower(f$funcRegex) == "contains") {
	            if (strstr(to_lower(rec$func), to_lower(f$func)) != 0 || f$func == "*") {
			        funcFlagged = T;
			        reason += fmt("Function=%s,", rec$func);
	            }
			} else if (to_lower(rec$func) == to_lower(f$func)) {
			    funcFlagged = T;
				reason += fmt("Function=%s,", rec$func);
			}
		}

	    if (!funcFlagged && f$func == "*") {
			funcFlagged = T;
			reason += fmt("Function=%s,", f$func);
		}

        # Time Frame
        timePerFlagged = verifyTime(rec$ts, f$timePer);
        if (timePerFlagged) {
            reason += fmt("Time=%s,", rec$ts);
        }

        if (ipFlagged && protoFlagged && funcFlagged && timePerFlagged) {
            f$flagged = T;
            local locRec: Policy::Info = [$ts=network_time(), $uid=unique_id(rec$uid), $severity=f$severity, $reason=reason, $polNum=f$polNum]; 
            Log::write(Policy::LOG, locRec);
		    #print fmt("%s - %s: True\n#\t%s\n#\t%s", rec$ts, t, reason, c);
        } else {
	        #print fmt("%s - %s: False - %s\n\t%s", rec$ts, t, reason, c);
		}
    }
}

function checkPolicyDnp3(rec: DNP3::Info) {
	for (f in filtersDnp3) {
		local ipFlagged: bool = F;
		local protoFlagged: bool = T;
		local funcFlagged: bool = F;
		local timePerFlagged: bool = F;
		local reason = "";

		# IP
		if (rec$id$orig_h in f$src && rec$id$resp_h in f$dest) {
			ipFlagged = T;
			reason += fmt("IP=%s -> %s,", rec$id$orig_h, rec$id$resp_h);
		}
	
		# Protocol
		reason += "Protocol=dnp3,";

	        local functionStr: string = "";
		# Function - DNP3
		if (to_lower(f$funcRegex) == "contains") {
			if (rec?$fc_request && (strstr(to_lower(rec$fc_request), to_lower(f$func)) != 0 || f$func == "*")) {
				funcFlagged = T;
				functionStr += rec$fc_request;
			}
			if (rec?$fc_reply && (strstr(to_lower(rec$fc_reply), to_lower(f$func)) != 0 || f$func == "*")) {
				funcFlagged = T;
				functionStr = fmt("%s;%s", functionStr, rec$fc_reply);
			}
		} else {
			if (rec?$fc_request && rec$fc_request == f$func) {
				funcFlagged = T;
				functionStr += rec$fc_request;
			}
			if (rec?$fc_reply && rec$fc_reply == f$func) {
				funcFlagged = T;
				functionStr = fmt("%s;%s", functionStr, rec$fc_reply);
			}
		}

		reason += fmt("Function=%s,", functionStr);

		if (!funcFlagged && f$func == "*") {
			funcFlagged = T;
			reason += fmt("Function=%s,", f$func);
		}

		# Time Frame
		timePerFlagged = verifyTime(rec$ts, f$timePer);
		if (timePerFlagged) {
			reason += fmt("Time=%s,", rec$ts);
		}

		if (ipFlagged && protoFlagged && funcFlagged && timePerFlagged) {
			f$flagged = T;
			local locRec: Policy::Info = [$ts=network_time(), $uid=unique_id(rec$uid), $severity=f$severity, $reason=reason, $polNum=f$polNum]; 
			Log::write(Policy::LOG, locRec);
			#print fmt("%s - %s: True\n#\t%s\n#\t%s", rec$ts, t, reason, c);
		} else {
			#print fmt("%s - %s: False - %s\n\t%s", rec$start_time, t, reason, c);
		}
	}
}

event bro_init() {
	Input::add_table([$source="/var/webServer/policy/policy.config", $name="policy", $idx=Idx, $val=Val, $destination=configTable, $mode=Input::REREAD]);

	#Input::remove("policy");

	Log::create_stream(Policy::LOG, [$columns=Info]);

	local policyFilter: Log::Filter = [
		$name="sqlite",
		$path="/var/webServer/database/policy",
		$config=table(["tablename"] = "PolicyEvents"),
		$writer=Log::WRITER_SQLITE
	];

	if (Log::add_filter(Policy::LOG, policyFilter)) {
		print fmt("Successfully added SQL filter");
	} else {
		print fmt("Failed adding SQL filter");
	}

	Log::remove_filter(Policy::LOG, "default");
}

event Input::end_of_data(name: string, source: string) {
        # Clear existing filters from memory
        filtersModbus = set();
        filtersDnp3 = set();
        filtersOther = set();

	for (x in configTable) {
	    print configTable[x];

		local tempFilter: filter;
		local d = split_string(configTable[x]$ip, /,/);
		local fr = split_string(configTable[x]$func, /:/);
		tempFilter$src = to_subnet(split_string(d[0], /=/)[1]);
		tempFilter$dest = to_subnet(split_string(d[1], /=/)[1]);

		tempFilter$funcRegex = fr[0];
		tempFilter$func = fr[1];
		tempFilter$proto = configTable[x]$proto;
		tempFilter$timePer = configTable[x]$timePer;

		tempFilter$severity = configTable[x]$severity;
		tempFilter$polNum = x;
			 
		if (tempFilter$proto == "modbus" || tempFilter$proto == "*") {
			add filtersModbus[tempFilter];
		} 
		if (tempFilter$proto == "dnp3" || tempFilter$proto == "*") {
			add filtersDnp3[tempFilter];
		}
		if (tempFilter$proto == "*") {
			add filtersOther[tempFilter];
		}
	}
}

event partial_connection(c: connection) {
	Policy::checkPolicy(c, "Partial");
}
event connection_attempt(c: connection) {
	Policy::checkPolicy(c, "Attempt");
}
event connection_half_finished(c: connection) {
	Policy::checkPolicy(c, "Half");
}
event connection_pending(c: connection) {
	Policy::checkPolicy(c, "Pending");
}
event connection_established(c: connection) {
	Policy::checkPolicy(c, "Established");
}
event Modbus::log_modbus (rec: Modbus::Info) {
	Policy::checkPolicyModbus(rec);
}
event DNP3::log_dnp3 (rec: DNP3::Info) {
	Policy::checkPolicyDnp3(rec);
}
