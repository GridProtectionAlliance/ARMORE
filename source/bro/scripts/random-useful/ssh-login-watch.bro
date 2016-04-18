@load base/frameworks/notice
@load base/protocols/ssh

module SSH;

export {
    const watched_servers: set[addr] = {
        192.168.1.100,
        192.168.1.101,
        192.168.1.102,
    } &redef;

    redef enum Notice::Type += {
        Watched_Login
    };
}

redef Notice::emailed_types += { Watched_Login };

event heuristic_successful_login(c: connection)
    {
    if ( c$id$resp_h in watched_servers )
        NOTICE([$note=Watched_Login,
                $msg=fmt("Possible SSH login to watched server: %s:%s",
                         c$id$resp_h, c$id$resp_p),
                $conn=c]);
    }