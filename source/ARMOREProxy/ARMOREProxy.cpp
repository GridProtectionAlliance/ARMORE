//******************************************************************************************************
//  ARMOREProxy.cpp - Gbtc
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
//  06/10/2015 - J. Ritchie Carroll
//       Generated original version of source code.
//
//******************************************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/poll.h>
#include <signal.h>
#include <stdexcept>

#include "UUID.h"
#include "Buffer.h"
#include "PacketProcessor.h"

#include "ConsoleLogger/ConsoleLogger.h"
#include "ConsoleLogger/ZeroMQLogger.h"
#include "ConsoleLogger/StandardLogger.h"

#include "CaptureEngine/CaptureEngine.h"
#include "CaptureEngine/NetmapEngine.h"
#include "CaptureEngine/PcapEngine.h"
#include "CaptureEngine/DpdkEngine.h"

#include "MiddlewareEngine/MiddlewareEngine.h"
#include "MiddlewareEngine/ZeroMQEngine.h"
#include "MiddlewareEngine/DdsEngine.h"

using namespace std;

ConsoleLogger* consoleLogger = nullptr;
CaptureEngine* captureEngine = nullptr;
MiddlewareEngine* middlewareEngine = nullptr;
PacketProcessor* packetProcessor = nullptr;
bool interrupted = false;

static void ReceivedBuffer(const UUID& connectionID, const Buffer& buffer);

static void ReceivedSignal(int signalNumber)
{
    if (signalNumber == SIGINT || signalNumber == SIGTERM)
    {
        interrupted = true;
        log_info("Process interrupted by %s, initiating graceful shutdown...\n", 
            signalNumber == SIGINT ? "Ctrl+C" : "SIGTERM");
    }
}

static void Usage(void)
{
    printf(
    //            1         2         3         4         5         6         7         8
    //   12345678901234567890123456789012345678901234567890123456789012345678901234567890
        "\nUsage:\n"
        "    ARMOREProxy [options] InterfaceName Destination:Port\n"
        "                                        ^ IP or DNS name - can use * for server\n\n"
        "    InterfaceName is used for capturing local traffic - interface must be in\n"
        "    promiscuous mode. Other parameters are used for encapsulated traffic.\n\n"
        "Examples:\n"
        "    ARMOREProxy -s -p -d -v -c pcap eth0 *:5560\n"
        "    ARMOREProxy -d -v -i 1 eth0 192.168.1.3:5560\n\n"
        "To bind to a specific interface (e.g., 192.168.1.20), use the following syntax:\n\n"
        "  Server mode:\n"
        "    ARMOREProxy -s -p -v eth0 192.168.1.20:5560\n\n"
        "  Client mode:\n"
        "    ARMOREProxy -v eth0 192.168.1.20;192.168.1.3:5560\n"
        "                        ^ bind to    ^ connect to\n\n"
        "Options:\n"
        " -h                  Make system transparent to network* (default=opaque)\n"
        " -c netmap|pcap|dpdk Use netmap, PCAP or DPDK** capture engine (default=netmap)\n"
        " -m zeromq|dds       Use zeromq or DDS** middleware engine (default=zeromq)\n"
        " -s                  Instantiate as server node (default=client mode)\n"
        " -e                  Enable encryption and authentication, e.g., ECC for zeromq\n"
        " -p                  Permit local segmented client traffic via server proxy\n"
        " -d                  Display console output (default=no console output)\n"
        " -o zeromq|std       Console output style for \"-d\" option (default=zeromq)\n"
        " -v                  Verbose processing output - use \"-i 1\" for full detail\n"
        " -i n                Packet feedback interval, i.e., show every \"n\" packets\n"
        " -n netmask          IPv4 netmask (default=captured interface configured mask)\n"
        " -l timeout          Client inactivity timeout, in seconds (default=60)\n"
        " -t timeout          IP inactivity timeout, in seconds (default=60)\n"
        " -a timeout          MAC address resolution timeout, in seconds (default=10)\n\n"
        "* When system is visible to network, machine IP will be used for proxied ARP\n"
        " requests between connected networks. In order to enable transparent operation,\n"
        " network configuration must allow IP forwarding and enable promiscuous mode.\n"
        "** Option has not been fully implemented in this version.\n\n");

    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{   
    ConsoleLoggerOption consoleLoggerOption = ConsoleLoggerOption::Default;
    CaptureEngineOption captureEngineOption = CaptureEngineOption::Default;
    MiddlewareEngineOption middlewareEngineOption = MiddlewareEngineOption::Default;

    bool transparentOperation = false;  // Enable transparent operation
    bool serverMode = false;            // Instantiate middleware as a "server" else a "client"
    bool securityEnabled = false;       // Enable middleware security, e.g., encryption and authentication
    bool permissiveMode = false;        // Permit server to proxy packets between clients on a segmented local network
    bool logToConsole = false;          // Show console based output
    bool verboseOutput = false;         // Show verbose processing output, e.g., parsed packet detail
    long feedbackInterval = 1000;       // Packet detail display interval
    int inactiveClientTimeout = 60;     // Inactive client timeout
    int inactiveIPTimeout = 60;         // Inactive IP timeout
    int macResolutionTimeout = 10;      // MAC resolution timeout
    char* interfaceName;                // Interface name used for packet capture
    char* endPoint;                     // End-point binding
    IPAddress ipv4Netmask;              // Netmask to use for IPv4 addresses, e.g., when captured interface has none
    pollfd fds[2];                      // File descriptors for polling
    int exitcode = EXIT_SUCCESS;
    int ch;

    // Parse command line options
    while ((ch = getopt(argc, argv, "hsepdvi:n:l:t:a:c:m:o:")) != -1)
    {
        switch (ch)
        {
            case 'h':   // Hidden mode
                transparentOperation = true;
                break;
            case 's':   // Server mode
                serverMode = true;
                break;
            case 'e':   // Encryption and authentication mode
                securityEnabled = true;
                break;
            case 'p':   // Permissive mode
                permissiveMode = true;
                break;
            case 'd':   // Display output
                logToConsole = true;
                break;
            case 'v':   // Verbose mode
                verboseOutput = true;
                break;
            case 'i':   // Interval for messages
                feedbackInterval = atol(optarg);
                break;
            case 'n':   // Netmask for IP validation
                ipv4Netmask = IPAddress::Parse(optarg);
                break;
            case 'l':   // Timeout for inactive clients
                inactiveClientTimeout = atoi(optarg);
                break;
            case 't':   // Timeout for inactive IPs
                inactiveIPTimeout = atoi(optarg);
                break;
            case 'a':   // Address resolution timeout
                macResolutionTimeout = atoi(optarg);
                break;
            case 'c':   // Capture engine option
                if (strcasecmp("netmap", optarg) == 0)
                    captureEngineOption = CaptureEngineOption::Netmap;
                else if (strcasecmp("pcap", optarg) == 0)
                    captureEngineOption = CaptureEngineOption::PCAP;
                else if (strcasecmp("dpdk", optarg) == 0)
                    captureEngineOption = CaptureEngineOption::DPDK;
                else
                    Usage();
                break;
            case 'm':   // Middleware engine option
                if (strcasecmp("zeromq", optarg) == 0)
                    middlewareEngineOption = MiddlewareEngineOption::ZeroMQ;
                else if (strcasecmp("dds", optarg) == 0)
                    middlewareEngineOption = MiddlewareEngineOption::DDS;
                else
                    Usage();
                break;
            case 'o':   // Output for console option
                if (strcasecmp("zeromq", optarg) == 0)
                    consoleLoggerOption = ConsoleLoggerOption::ZeroMQ;
                else if (strcasecmp("std", optarg) == 0)
                    consoleLoggerOption = ConsoleLoggerOption::Standard;
                else
                    Usage();
                break;
            default:
                Usage();
                break;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc != 2)
        Usage();

    // Get remaining command line parameters
    interfaceName = argv[0];
    endPoint = argv[1];

    try
    {
        // Establish common logging functions
        switch (consoleLoggerOption)
        {
            case ConsoleLoggerOption::ZeroMQ:
                consoleLogger = new ZeroMQLogger("ARMOREProxy", logToConsole);
                break;
            case ConsoleLoggerOption::Standard:
                consoleLogger = new StandardLogger(logToConsole);
                break;
            default:
                throw runtime_error("Invalid console logger option");
        }

        InitializeConsoleLogger(consoleLogger);
    
        log_info("\nStarting ARMOREProxy...\n");
    
        // Setup signal interrupt captures
        signal(SIGINT, ReceivedSignal);
        signal(SIGTERM, ReceivedSignal);

        // Establish middleware engine
        switch (middlewareEngineOption)
        {
            case MiddlewareEngineOption::ZeroMQ:
                log_info("Establishing ZeroMQ %s based middleware engine%s...\n", serverMode ? "ROUTER (server)" : "DEALER (client)", securityEnabled ? " with security enabled" : "");
                middlewareEngine = new ZeroMQEngine(&ReceivedBuffer, serverMode, securityEnabled, inactiveClientTimeout, verboseOutput);
                break;
            case MiddlewareEngineOption::DDS:
                log_info("Establishing DDS %s based middleware engine%s...\n", serverMode ? "server" : "client", securityEnabled ? " with security enabled" : "");
                middlewareEngine = new DdsEngine(&ReceivedBuffer, serverMode, securityEnabled, inactiveClientTimeout, verboseOutput);
                break;
            default:
                throw runtime_error("Invalid middleware engine option");
        }

        // Establish packet capture engine
        switch (captureEngineOption)
        {
            case CaptureEngineOption::Netmap:
                log_info("Establishing netmap based packet capture engine...\n");
                captureEngine = new NetmapEngine(interfaceName);
                break;
            case CaptureEngineOption::PCAP:
                log_info("Establishing PCAP based packet capture engine...\n");
                captureEngine = new PcapEngine(interfaceName);
                break;
            case CaptureEngineOption::DPDK:
                log_info("Establishing DPDK based packet capture engine...\n");
                captureEngine = new DpdkEngine(interfaceName);  
                break;
            default:
                throw runtime_error("Invalid packet capture engine option");
        }

        if (!serverMode && permissiveMode)
        {
            permissiveMode = false;
            log_info("WARNING: Permissive mode ignored for client instantiations.\n");            
        }
        
        // Get host name of local machine
        char hostName[HOST_NAME_MAX + 1];
        hostName[HOST_NAME_MAX] = 0;

        if (gethostname(hostName, HOST_NAME_MAX) != 0)
            strcpy(hostName, "localhost");
        
        // Initialize middleware engine for end-point
        log_info("Connecting %s middleware %s \"%s\"...\n", hostName, serverMode ? "on" : "to", endPoint);
        middlewareEngine->Initialize(endPoint, hostName, captureEngine->LocalMACAddress.ToString());

        // Open packet capture engine for interface
        log_info("Starting packet capture on \"%s\"...\n", interfaceName);
        captureEngine->Open();
    
        // Initialize packet processor
        packetProcessor = new PacketProcessor(
                                captureEngine, 
                                middlewareEngine,
                                transparentOperation,
                                permissiveMode,
                                ipv4Netmask,
                                inactiveIPTimeout,
                                macResolutionTimeout,
                                verboseOutput,
                                feedbackInterval);

        // Monitor captured interface for new outgoing packets - this descriptor
        // is used control when data I/O is sent and received.
        fds[0].fd = captureEngine->GetFileDescriptor();
        fds[0].events = POLLIN;
    
        // Monitor packet processor for new incoming packets - this descriptor
        // is only used to wake up poll, notifying that data received from the
        // middleware engine is ready to be injected onto captured interface.
        fds[1].fd = packetProcessor->GetFileDescriptor();
        fds[1].events = POLLIN;

        // Start packet processing
        int result;
        
        while (!interrupted)
        {
            // If there is data ready to be injected onto capture interface,
            // get poll to process POLLOUT on capture engine file descriptor
            // so we will know when its safe to process received data.
            if (packetProcessor->GetQueuedBufferCount())
                fds[0].events |= POLLOUT;
            else
                fds[0].events &= ~POLLOUT;

            result = poll(fds, 2, 250);

            if (result > 0)
            {                        
                if (fds[0].revents & POLLIN)
                    packetProcessor->EncodePackets();
            
                if (fds[0].revents & POLLOUT)
                    packetProcessor->DecodePackets();
            }
            else
            {
                if (errno != EAGAIN && errno != EINTR && errno != EBADF)
                    log_error("Error during primary poll: %s\n", strerror(errno));
            }
        }
    }
    catch (exception& ex)
    {
        log_error("Exception: %s\n", ex.what());
        exitcode = EXIT_FAILURE;
    }
    catch (...)
    {
        log_error("Exception encountered.\n");
        exitcode = EXIT_FAILURE;
    }

    // Close packet capture engine
    if (captureEngine != nullptr)
    {
        log_info("Stopping packet capture...\n");
        captureEngine->Close();
        delete captureEngine;
    }

    // Shutdown middleware engine
    if (middlewareEngine != nullptr)
    {
        log_info("Disconnecting middleware...\n");
        middlewareEngine->Shutdown();
        delete middlewareEngine;
    }
    
    // Release packet processor
    if (packetProcessor != nullptr)
        delete packetProcessor;

    // Delete console logger
    if (consoleLogger != nullptr)
        delete consoleLogger;

    if (logToConsole)
        printf("Exiting <= %d...\n\n", exitcode);
    
    return exitcode;
}

static void ReceivedBuffer(const UUID& connectionID, const Buffer& buffer)
{
    if (packetProcessor != nullptr)
        packetProcessor->QueueReceivedBuffer(connectionID, buffer);
}