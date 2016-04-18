@load base/frameworks/sumstats

# ref: http://try.bro.org/#/trybro/saved/303f9f04-bee6-4ecd-8d3f-dd7e870f02dc

# First step: create a plugin for Sumstats that uses a new data
# structure. In our case we want a table with string keys and
# count values (to count);

module SumStats;

export {
    redef enum Calculation += { COUNTTABLE };

    redef record ResultVal += {
        counttable: table[string] of count &optional;
    };
}

function add_ct_entry(mytable: table[string] of count, str: string, num: count)
    {
        if ( str !in mytable )
            mytable[str] = 0;

        mytable[str] += num;
    }

hook register_observe_plugins()
    {
    register_observe_plugin(COUNTTABLE, function(r: Reducer, val: double, obs: Observation, rv: ResultVal)
        {
        # observations for us always need a string and a num. Otherwhise - complain.
        if ( ! obs?$str )
            {
            Reporter::error("COUNTTABLE sumstats plugin needs str in observation");
            return;
            }

        local increment = 1;
        if ( obs?$num )
            increment = obs$num;

        if ( ! rv?$counttable )
            rv$counttable = table();

        add_ct_entry(rv$counttable, obs$str, increment);
        });
    }

hook compose_resultvals_hook(result: ResultVal, rv1: ResultVal, rv2: ResultVal)
    {
    if ( ! (rv1?$counttable || rv2?$counttable ) )
        return;

    if ( !rv1?$counttable )
        {
        result$counttable = copy(rv2$counttable);
        return;
        }

    if ( !rv2?$counttable )
        {
        result$counttable = copy(rv1$counttable);
        return;
        }

    result$counttable = copy(rv1$counttable);

    for ( i in rv2$counttable )
        add_ct_entry(result$counttable, i, rv2$counttable[i]);
    }


module GLOBAL;

# Now we can write our script using the new sumstat plugin that we just created.
# This is basically equivalent to what we did before

event bro_init()
    {
    local r1 = SumStats::Reducer($stream="status.code", $apply=set(SumStats::COUNTTABLE));
    SumStats::create([$name="http-status-codes",
        $epoch=1hr, $reducers=set(r1),
        $epoch_result(ts: time, key: SumStats::Key, result: SumStats::Result) =
        {
            local r = result["status.code"];
            # abort if we have no results
            if ( ! r?$counttable )
                return;

            local counttable = r$counttable;
            print fmt("Host: %s", key$host);
            for ( i in counttable )
                print fmt("status code: %s, count: %d", i, counttable[i]);
        }]);
    }

event http_reply(c: connection, version: string, code: count, reason: string)
    {
    SumStats::observe("status.code", [$host=c$id$resp_h], [$str=cat(code), $num=1]);
    }
