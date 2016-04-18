redef Communication::nodes += {
	["foo"] = [$host = 127.0.0.1, $p=1337/tcp, $events = /my_event_response/, $connect=T]
};

event my_event_request(details: string)
	{
	print "sent my_event_request", details;
	}

event my_event_response(details: count)
	{
	print "recv my_event_response", details;
	print "terminating";
	terminate();
	}

event remote_connection_handshake_done(p: event_peer)
	{
	print fmt("connection established to: %s", p);
	event my_event_request("hello");
	}