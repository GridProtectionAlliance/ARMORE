@load frameworks/communication/listen

redef Communication::listen_port = 1337/tcp;

redef Communication::nodes += {
	["foo"] = [$host = 127.0.0.1, $events = /my_event_request/, $connect = F]
};

event remote_connection_handshake_done(p: event_peer)
	{
	print fmt("connection established to: %s", p);
	}

global request_count: count = 0;

event my_event_response(details: count)
	{
	print "sent my_event_response", details;
	}

event my_event_request(details: string)
	{
	print "recv my_event_request", details;
	++request_count;
	event my_event_response(request_count);
	}

event remote_connection_closed(p: event_peer)
	{
	print fmt("connection to peer closed: %s", p);
	}