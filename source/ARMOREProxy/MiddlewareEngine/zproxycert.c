//******************************************************************************************************
//  zproxycert.c - Gbtc
//
//  Copyright © 2015, Grid Protection Alliance.  All Rights Reserved.
//
//  Licensed to the Grid Protection Alliance (GPA) under one or more contributor license agreements. See
//  the NOTICE file distributed with this work for additional information regarding copyright ownership.
//  The GPA licenses this file to you under the MIT License (MIT), the "License"; you may
//  not use this file except in compliance with the License. You may obtain a copy of the License at:
//
//      http://opensource.org/licenses/MIT
//
//  Unless agreed to in writing, the subject software distributed under the License is distributed on an
//  "AS-IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. Refer to the
//  License for the specific language governing permissions and limitations.
//
//  Code Modification History:
//  ----------------------------------------------------------------------------------------------------
//  09/11/2015 - J. Ritchie Carroll
//       Created new actor based on https://github.com/zeromq/czmq/blob/master/src/zproxy.c
//
//******************************************************************************************************

#include <czmq.h>

typedef struct
{
    zsock_t* pipe;              // Actor command pipe
    zpoller_t *poller;          // Socket poller
    zsock_t* frontend;          // Frontend socket
    zsock_t* backend;           // Backend socket
    zcert_t* certificate;       // Server certificate
    bool terminated;            // Did caller ask us to quit?
    bool verbose;               // Verbose logging enabled?
}
self_t;

static void
s_self_destroy(self_t** self_p)
{
    assert(self_p);
    
    if (*self_p)
    {
        self_t* self = *self_p;
        zsock_destroy(&self->frontend);
        zsock_destroy(&self->backend);
        self->certificate = NULL;
        zpoller_destroy(&self->poller);
        free(self);
        *self_p = NULL;
    }
}

static self_t* 
s_self_new(zsock_t* pipe, zcert_t* certificate)
{
    self_t* self = (self_t*)zmalloc(sizeof(self_t));
    
    if (self)
    {
        self->pipe = pipe;
        self->poller = zpoller_new(self->pipe, NULL);
        self->certificate = certificate;
        
        if (!self->poller)
            s_self_destroy(&self);
    }
    
    return self;
}

static zsock_t* 
s_create_socket(char* type_name, char* endpoints, zcert_t* certificate)
{
    //  This array matches ZMQ type definitions
    assert(ZMQ_PAIR == 0);

    char* type_names [] =
    {
        "PAIR",
        "PUB",
        "SUB",
        "REQ",
        "REP",
        "DEALER",
        "ROUTER",
        "PULL",
        "PUSH",
        "XPUB",
        "XSUB",
        type_name
    };
    
    //  We always match type at least at end of table
    int index;
    
    for (index = 0; strneq(type_name, type_names[index]); index++);
    
    if (index > ZMQ_XSUB)
    {
        zsys_error("zproxycert: invalid socket type '%s'", type_name);
        return NULL;
    }
    
    zsock_t* sock = zsock_new(index);
    
    if (sock)
    {
        if (certificate)
        {
            // Apply local full certificate (public + private) to proxy socket
            zcert_apply(certificate, sock);
        
            // Enable curve server mode for proxy socket
            zsock_set_curve_server(sock, 1);
        }
        
        if (zsock_attach(sock, endpoints, true))
        {
            zsys_error("zproxycert: invalid endpoints '%s'", endpoints);
            zsock_destroy(&sock);
        }
    }
    
    return sock;
}

static void
s_self_configure(self_t* self, zsock_t** sock_p, zmsg_t* request, char* name, bool applyCert)
{
    char* type_name = zmsg_popstr(request);
    assert(type_name);
    
    char* endpoints = zmsg_popstr(request);
    assert(endpoints);
    
    if (self->verbose)
        zsys_info("zproxycert: - %s type=%s attach=%s%s", name, type_name, endpoints, applyCert && self->certificate ? " with certificate" : "");
    
    assert(*sock_p == NULL);    
    *sock_p = s_create_socket(type_name, endpoints, applyCert ? self->certificate : NULL);
    assert(*sock_p);
    
    zpoller_add(self->poller, *sock_p);
    
    zstr_free(&type_name);
    zstr_free(&endpoints);
}

static int
s_self_handle_pipe(self_t* self)
{
    // Get the whole message off the pipe in one go
    zmsg_t* request = zmsg_recv(self->pipe);
    
    if (!request)
        return -1; // Interrupted

    char* command = zmsg_popstr(request);
    assert(command);
    
    if (self->verbose)
        zsys_info("zproxycert: API command=%s", command);

    if (streq(command, "FRONTEND"))
    {
        s_self_configure(self, &self->frontend, request, "frontend", true);
        zsock_signal(self->pipe, 0);
    }
    else if (streq(command, "BACKEND"))
    {
        s_self_configure(self, &self->backend, request, "backend", false);
        zsock_signal(self->pipe, 0);
    }
    else if (streq(command, "VERBOSE"))
    {
        self->verbose = true;
        zsock_signal(self->pipe, 0);
    }
    else if (streq(command, "$TERM"))
    {
        self->terminated = true;
    }
    else
    {
        zsys_error("zproxycert: - invalid command: %s", command);
        assert(false);
    }
    
    zstr_free(&command);
    zmsg_destroy(&request);
    
    return 0;
}

static void
s_self_switch(self_t* self, zsock_t* input, zsock_t* output)
{
    // Low-level libzmq API used for best performance
    void *zmq_input = zsock_resolve(input);
    void *zmq_output = zsock_resolve(output);

    zmq_msg_t msg;
    zmq_msg_init(&msg);
    
    while (true)
    {
        if (zmq_recvmsg(zmq_input, &msg, ZMQ_DONTWAIT) == -1)
            break; // Presumably EAGAIN
        
        int send_flags = zsock_rcvmore(input) ? ZMQ_SNDMORE : 0;
        
        if (zmq_sendmsg(zmq_output, &msg, send_flags) == -1)
        {
            zmq_msg_close(&msg);
            break;
        }
    }
}

void
zproxycert(zsock_t* pipe, void* certificate)
{
    self_t* self = s_self_new(pipe, (zcert_t*)certificate);    
    assert(self);

    // Signal successful initialization
    zsock_signal(pipe, 0);

    while (!self->terminated)
    {
        zsock_t* which = (zsock_t*)zpoller_wait(self->poller, -1);
        
        if (zpoller_terminated(self->poller))
            break; // Interrupted
        else if (which == self->pipe)
            s_self_handle_pipe(self);
        else if (which == self->frontend)
            s_self_switch(self, self->frontend, self->backend);
        else if (which == self->backend)
            s_self_switch(self, self->backend, self->frontend);
    }
    
    s_self_destroy(&self);
}