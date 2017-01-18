#include <string>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <signal.h>
#include <vector>
#include <algorithm>

#include <asiodnp3/DNP3Manager.h>
#include <asiodnp3/PrintingSOEHandler.h>
#include <asiodnp3/ConsoleLogger.h>
#include <asiodnp3/MeasUpdate.h>

#include <asiopal/UTCTimeSource.h>
#include <opendnp3/outstation/SimpleCommandHandler.h>
#include <opendnp3/outstation/Database.h>
#include <opendnp3/LogLevels.h>

#include <asiodnp3/DefaultMasterApplication.h>
#include <asiodnp3/PrintingCommandCallback.h>


using namespace std;
using namespace opendnp3;
using namespace openpal;
using namespace asiopal;
using namespace asiodnp3;

static void usage(void)
{
    printf(
    //            1         2         3         4         5         6         7         8
    //   12345678901234567890123456789012345678901234567890123456789012345678901234567890
        "\nUsage:\n"
		"    dnp3gen will create either outstations, masters, or both generating traffic\n"
        "    dnp3gen [options] InterfaceName Destination:Port\n"
        "    InterfaceName is used for capturing local traffic - interface must be in\n"
        "    promiscuous mode. Other parameters are used for encapsulated traffic.\n\n"
        "Examples:\n"
        "    dnp3gen -d -v -i 1 eth0 192.168.1.3:5560\n\n"
        "To bind to a specific interface (e.g., 192.168.1.20), use the following syntax:\n\n"
        "  Server mode:\n"
        "    dnp3gen -s -p -v eth0 192.168.1.20:5560\n\n"
        "  Client mode:\n"
        "    dnp3gen -v eth0 192.168.1.20;192.168.1.3:5560\n"
        "                        ^ bind to    ^ connect to\n\n"
        "Options:\n"
        " -h                  Make system transparent to network* (default=opaque)\n"
        " -c netmap|pcap|dpdk Use netmap, PCAP or DPDK** capture engine (default=netmap)\n"
        " -a timeout          MAC address resolution timeout, in seconds (default=10)\n\n");

    exit(EXIT_FAILURE);
}


//string splitting function
vector<string> split(string str, char delim)
{
    vector<string> obj;
    stringstream ss(str);
    string tok;
    while(getline(ss, tok, delim))
    {
	obj.push_back(tok);
    }

    return obj;
}


void cleanup()
{
	int shell;
	int virtnic = 0;
	string subcmd;
	for(int x=0; x<=15; x++)
	{
	
	    subcmd="ifconfig eth2:"+ to_string(virtnic)+ " down";
	    //cout << subcmd;
	    shell = system(subcmd.c_str());
	    //printf("%d", shell);
	    virtnic++;
	}

}

void ConfigureDatabase(DatabaseConfigView view)
{
	// example of configuring analog index 0 for Class2 with floating point variations by default
	view.analogs[0].variation = StaticAnalogVariation::Group30Var5;
	view.analogs[0].metadata.clazz = PointClass::Class2;
	view.analogs[0].metadata.variation = EventAnalogVariation::Group32Var7;
}




void signal_h(int signalType)
{
	printf("Caught Signal, Exiting %d\n", signalType);
	cleanup();
	exit(EXIT_SUCCESS);
}


class network
{
    //private:
    //vector<network*> track;

    public:
    string ipaddress; //ipv4 address
    string role; //master or outstation
    string vnic; //nic location
    string location; //remote or local
    bool allocated; //True or False
    int port; //generally 20000
    uint16_t DNPREPORT;
    uint16_t DNPADDR;

    void allocate()
    {
	int shell;	
	string subcmd;
	subcmd="ifconfig "+ this->vnic+ " " + this->ipaddress;
	//cout << subcmd;
	shell = system(subcmd.c_str());
	//printf("%d", shell);
	//printf("testing");
	//this->track.push_back(this); //keep track this instance by putting on class vector
    }


};

class master
{
    public:
	string outstationaddr;
	string bind;
	string remote;
	int port;
	uint16_t DNPREPORT;
	uint16_t DNPADDR;

	void run(void)
	{

	    // Specify what log levels to use. NORMAL is warning and above
	    // You can add all the comms logging by uncommenting below
	    const uint32_t FILTERS = levels::NORMAL | levels::ALL_APP_COMMS;

	    // This is the main point of interaction with the stack
	    DNP3Manager manager(1, ConsoleLogger::Create());

	    // Connect via a TCPClient socket to a outstation
	    auto pChannel = manager.AddTCPClient("tcpclient", FILTERS, ChannelRetry::Default(), outstationaddr, bind, 20000);
	    //auto zChannel = manager.AddTCPClient("tcpclient", FILTERS, ChannelRetry::Default(), ipv4, bind, 20000);

	    // Optionally, you can bind listeners to the channel to get state change notifications
	    // This listener just prints the changes to the console
	    pChannel->AddStateListener([](ChannelState state)
		    {
		    std::cout << "channel state: " << ChannelStateToString(state) << std::endl;
		    });

	    // The master config object for a master. The default are
	    // useable, but understanding the options are important.
	    MasterStackConfig stackConfig;

	    // you can override application layer settings for the master here
	    // in this example, we've change the application layer timeout to 2 seconds
	    stackConfig.master.responseTimeout = TimeDuration::Seconds(2);
	    stackConfig.master.disableUnsolOnStartup = true;

	    // You can override the default link layer settings here
	    // in this example we've changed the default link layer addressing
	    stackConfig.link.LocalAddr = this->DNPADDR;
	    stackConfig.link.RemoteAddr = this->DNPREPORT; 

	    // Create a new master on a previously declared port, with a
	    // name, log level, command acceptor, and config info. This
	    // returns a thread-safe interface used for sending commands.
	    auto master = pChannel->AddMaster(
		    "master",										// id for logging
		    PrintingSOEHandler::Instance(),					// callback for data processing
		    asiodnp3::DefaultMasterApplication::Instance(),	// master application instance
		    stackConfig										// stack configuration
		    );


	    // do an integrity poll (Class 3/2/1/0) once per minute
	    auto integrityScan = master->AddClassScan(ClassField::AllClasses(), TimeDuration::Minutes(1));

	    // do a Class 1 exception poll every 5 seconds
	    auto exceptionScan = master->AddClassScan(ClassField(ClassField::CLASS_1), TimeDuration::Seconds(2));

	    // Enable the master. This will start communications.
	    master->Enable();

	    bool channelCommsLoggingEnabled = true;
	    bool masterCommsLoggingEnabled = true;

	    while(true)
	    {
		master->ScanRange(GroupVariationID(1, 2), 0, 3);
		usleep(5000);
	    }

	}
};

class outstation
{
    public:
	string bind;
	int port;
	uint16_t DNPREPORT;
	uint16_t DNPADDR;

	void run(void)
	{
	    //string bind = "192.168.5.90";
	    string bind = this->bind;
	    const uint32_t FILTERS = levels::NORMAL | levels::ALL_COMMS;

	    // This is the main point of interaction with the stack
	    // Allocate a single thread to the pool since this is a single outstation
	    // Log messages to the console
	    DNP3Manager manager(1, ConsoleLogger::Create());

	    // Create a TCP server (listener)
	    auto channel = manager.AddTCPServer("server", FILTERS, ChannelRetry::Default(), bind, 20000);

	    // Optionally, you can bind listeners to the channel to get state change notifications
	    // This listener just prints the changes to the console
	    channel->AddStateListener([](ChannelState state)
		    {
		    std::cout << "channel state: " << ChannelStateToString(state) << std::endl;
		    });

	    // The main object for a outstation. The defaults are useable,
	    // but understanding the options are important.
	    OutstationStackConfig stackConfig;

	    // You must specify the shape of your database and the size of the event buffers
	    stackConfig.dbTemplate = DatabaseTemplate::AllTypes(10);
	    stackConfig.outstation.eventBufferConfig = EventBufferConfig::AllTypes(10);

	    // you can override an default outstation parameters here
	    // in this example, we've enabled the oustation to use unsolicted reporting
	    // if the master enables it
	    stackConfig.outstation.params.allowUnsolicited = true;

	    // You can override the default link layer settings here
	    // in this example we've changed the default link layer addressing
	    stackConfig.link.LocalAddr = this->DNPADDR;
	    stackConfig.link.RemoteAddr = this->DNPREPORT;

	    // Create a new outstation with a log level, command handler, and
	    // config info this	returns a thread-safe interface used for
	    // updating the outstation's database.
	    auto outstation = channel->AddOutstation("outstation", SuccessCommandHandler::Instance(), DefaultOutstationApplication::Instance(), stackConfig);

	    // You can optionally change the default reporting variations or class assignment prior to enabling the outstation
	    ConfigureDatabase(outstation->GetConfigView());

	    // Enable the outstation and start communications
	    outstation->Enable();

	    // variables used in example loop
	    string input;
	    uint32_t count = 0;
	    double value = 0;
	    bool binary = false;
	    DoubleBit dbit = DoubleBit::DETERMINED_OFF;
	    bool channelCommsLoggingEnabled = true;
	    bool outstationCommsLoggingEnabled = true;


	    //threading event handler if something happens
	    while(true)
	    {
		usleep(5000);
	    }
	}
};

int ipcheck(vector<network>& dnpnet, string ipaddress)
{
	vector<network>::iterator vect;
	for(vect = dnpnet.begin(); vect != dnpnet.end(); ++vect)
	{
	    if(vect->ipaddress == ipaddress)
		return 1; //address in use and found
	}

    return 0; //was not found
}

int dnpcheck(vector<network>& dnpnet, int ednpaddr)
{
	vector<network>::iterator vect;
	for(vect = dnpnet.begin(); vect != dnpnet.end(); ++vect)
	{
	    if(vect->DNPADDR== ednpaddr)
		return 1; //dnp address in use and found
	}

    return 0; //dnp address was not found
}

int main(int argc, char *argv[])
{
	//Handle Ctrl-C event 
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = signal_h;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);


	int ch;
	int shell;
	string remotemanualOutstations; //IPv4 addresses for manually added outstations
	string remotemanualMasters; //IPv4 addresses for manually added masters
	int masterStations; //Total number of master Stations to simulate
	int simOutstations; //Total number of outstations to simulate
	string ipv4space; //ipv4 address space with nic
	string nic; //interface to use for virtualnic


	

	//Parse out command line options
	while ((ch = getopt(argc, argv, "m:no:ns:na:ni:nj:n")) != -1)
	{
		switch (ch)
		{
			case 'o':
				printf("Remote Outstation Addresses:: %s \n", optarg);
				remotemanualOutstations = optarg;
				break;
			case 'j':
				printf("Remote Master Addresses:: %s \n", optarg);
				remotemanualMasters= optarg;
				break;
			case 'm':
				printf("Number of Simulated Master stations:: %s \n", optarg);
				masterStations = std::stoi(optarg);
				break;
			case 's':
				printf("Number of Simulated Outstations:: %s \n", optarg);
				simOutstations = std::stoi(optarg);
				break;
			case 'a':
				printf("IPV4 address:: %s \n", optarg);
				ipv4space= optarg;
				//ipv4space = std::strstr(optarg, "/");
				//ipv4space = split(optarg,delim, ipv4space);
				break;
			case 'i':
				printf("Virtual Interface:: %s \n", optarg);
				nic = optarg;
				break;
			default:
				usage();
				break;


		}
	}

	
	//Check we can output
	if (system(NULL))
		puts("OK");
	else
		exit(EXIT_FAILURE);

	//begin to allocate ip address objects to pass to outstations and masters
	
	int i_remotemanualOutstations= (std::count(remotemanualOutstations.begin(), remotemanualOutstations.end(), '.')/3);
	int i_remotemanualMasters= (std::count(remotemanualMasters.begin(), remotemanualMasters.end(), '.')/3);  
	int vnictrack= 0;
	int octettrack=1;

	vector<network> dnp3objs; //create a vector to keep track of network objects
	//--------------------------------------------------------
	//----Manually add remote workstations addresses
	//--------------------------------------------------------
	//--------------------------------------------------------
	//--------------------------------------------------------
  
	if (i_remotemanualOutstations != 0)
	{
	    //--//uint16_t mDNPREP;
	    uint16_t mDNPADDR;
	    //--//string DNPREPORT_s;
	    string DNPADDR_s;

	    //split string and insert in our network vector
	    vector<string> seperated = split(remotemanualOutstations, ',');
	    for(string address : seperated)
	    {
		//Prompt user for DNP addresses to report to
		//--//printf("Enter DNP REPORT for %s:: \n", address.c_str());
		//--//getline(cin, DNPREPORT_s);

		//while(dnpcheck(dnp3objs, ))
		//{
		printf("Enter DNP Address for %s:: \n", address.c_str());
		getline(cin, DNPADDR_s);
		//}

		//--//stringstream(DNPREPORT_s) >> mDNPREP;
		stringstream(DNPADDR_s) >> mDNPADDR;

		network *station = new network;
		station->ipaddress = address;
		station->role = "Outstation";
		//station->vnic = nic+":"+to_string(vnictrack);
		//vnictrack++;
		station->allocated = "True"; //don't actually allocate but prevent others from taking address
		station->location = "Remote";
		station->port = 20000;
		//--//station->DNPREPORT = mDNPREP;
		station->DNPADDR = mDNPADDR;
		//station->allocate();
		dnp3objs.push_back(*station); //push object pointer onto vector to keep track

	    }
	}
	
	if (i_remotemanualMasters!= 0)
	{
	    //--//uint16_t mDNPREP;
	    uint16_t mDNPADDR;
	    //--//string DNPREPORT_s;
	    string DNPADDR_s;


	    //split string and insert in our network vector
	    vector<string> seperated = split(remotemanualMasters, ',');
	    for(string address : seperated)
	    {
		//Prompt user for DNP addresses to report to
		//--//printf("Enter DNP REPORT for %s:: \n", address.c_str());
		//--//getline(cin, DNPREPORT_s);

		printf("Enter DNP Address for %s:: \n", address.c_str());
		getline(cin, DNPADDR_s);

		//--//stringstream(DNPREPORT_s) >> mDNPREP;
		stringstream(DNPADDR_s) >> mDNPADDR;

		network *mstation = new network;
		mstation->ipaddress = address;
		mstation->role = "Master";
		//mstation->vnic = nic+":"+to_string(vnictrack);
		//vnictrack++;
		mstation->allocated = "True";
		mstation->location = "Remote";
		mstation->port = 20000;
		//--//mstation->DNPREPORT = mDNPREP;
		mstation->DNPADDR = mDNPADDR;
		//mstation->allocate();
		dnp3objs.push_back(*mstation); //push object pointer onto vector to keep track

	    }
	} 
	//--------------------------------------------------------
	//----Manually add local workstations addresses(todo)
	//--------------------------------------------------------
	//--------------------------------------------------------
	//--------------------------------------------------------
	
	
	//--------------------------------------------------------
	//----Add simulated workstations addresses
	//--------------------------------------------------------
	//--------------------------------------------------------
	//--------------------------------------------------------
	
	
	for(int x=1; x<= masterStations; x++)
	{
	    string addrcheck;
	    network *msimstation = new network;
	    //start a 1-254, and see if exists for a /24 network for now
	    for(int octet=octettrack; octet<=254; octet++)
	    {
		addrcheck=ipv4space+ to_string(octet); 
		if(!ipcheck(dnp3objs,addrcheck))
		{
		    msimstation->ipaddress = addrcheck;
		    break; 
		}
	    }

	    msimstation->role = "Master";
	    msimstation->vnic = nic+":"+to_string(vnictrack);
	    vnictrack++;
	    msimstation->allocated = "True";
	    msimstation->location = "Local";
	    msimstation->port = 20000;
	    //--//msimstation->DNPREPORT = mDNPREP;
	    for(int dnpacheck=0; dnpacheck<=65530; dnpacheck++)
	    {
		if(!dnpcheck(dnp3objs,dnpacheck))
		{
		    msimstation->DNPADDR = dnpacheck;
		    break;
		}
	    }
	    msimstation->allocate();
	    dnp3objs.push_back(*msimstation); //push object pointer onto vector to keep track
	}
	
	for(int x=1; x<= simOutstations; x++)
	{
	    string addrcheck;
	    network *osimstation = new network;
	    //start a 1-254, and see if exists for a /24 network for now
	    for(int octet=octettrack; octet<=254; octet++)
	    {
		addrcheck=ipv4space+ to_string(octet); 
		if(!ipcheck(dnp3objs,addrcheck))
		{
		    osimstation->ipaddress = addrcheck;
		    break; 
		}
	    }

	    osimstation->role = "Outstation";
	    osimstation->vnic = nic+":"+to_string(vnictrack);
	    vnictrack++;
	    osimstation->allocated = "True";
	    osimstation->location = "Local";
	    osimstation->port = 20000;
	    //--//osimstation->DNPREPORT = mDNPREP;
	    for(int dnpacheck=0; dnpacheck<=65530; dnpacheck++)
	    {
		if(!dnpcheck(dnp3objs,dnpacheck))
		{
		    osimstation->DNPADDR = dnpacheck;
		    break;
		}
	    }
	    osimstation->allocate();
	    dnp3objs.push_back(*osimstation); //push object pointer onto vector to keep track
	}

	//almost all objects should be allocated

	int totaloutstation_i=0;
	int totalmaster_i=0;
	vector<network>::iterator vect;
	for(vect = dnp3objs.begin(); vect != dnp3objs.end(); ++vect)
	{
	    printf("\n>>>>IPADDRESS:%s NIC:%s  DNPADDR:%i  ROLE:%s  Location:%s", vect->ipaddress.c_str(), vect->vnic.c_str(), vect->DNPADDR,\
		    vect->role.c_str(), vect->location.c_str());
	    if(vect->role == "Outstation")
		totaloutstation++;
	    if(vect->role == "Master")
		totalmaster++;
	}

	printf("HIT ENTER TO START");
//DONE ALLOCATING OUTSTATIONS, MASTERS, REMOTEOUTSTATIONS, REMOTEMASTERS
//TODO(ManualLocalMasters and ManualLocalOutstations)
	pause();

	//--------------------------------
	//--------------------------------
	//--------------------------------
	//--------------------------------
	//--------------------------------
	//--------------begin all outstation threads//
	//--------------------------------
	//--------------------------------
	//--------------------------------
	//--------------------------------
	vector<outstation> test;

	/*reference
	uint16_t rep=90;
	uint16_t addr=91;
	
	int i_remotemanualOutstations= (std::count(remotemanualOutstations.begin(), remotemanualOutstations.end(), '.')/3);
	int i_remotemanualMasters= (std::count(remotemanualMasters.begin(), remotemanualMasters.end(), '.')/3);  
	masterStations
	simOutstations; //Total number of outstations to simulate
	*/

	for(int x=1; x<=(simOutstations+i_remoteOutstations); x++)
	{
	    outstation *a1 = new outstation;
	    a1->bind=address+to_string(x);
	    a1->port=9000;
	    a1->DNPREPORT = rep;
	    a1->DNPADDR = addr;

	    //rep++;
	    addr++;

	    //keep track of objects
	    test.push_back(*a1);

	    thread functorTest(&outstation::run, a1);
	    functorTest.detach();
	}

	return 0;
}
