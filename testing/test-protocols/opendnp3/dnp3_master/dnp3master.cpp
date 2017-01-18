#include <asiodnp3/DNP3Manager.h>
#include <asiodnp3/PrintingSOEHandler.h>
#include <asiodnp3/ConsoleLogger.h>
#include <asiodnp3/DefaultMasterApplication.h>
#include <asiodnp3/PrintingCommandCallback.h>

#include <asiopal/UTCTimeSource.h>

#include <opendnp3/LogLevels.h>

using namespace std;
using namespace openpal;
using namespace asiopal;
using namespace asiodnp3;
using namespace opendnp3;

int main(int argc, char* argv[])
{
	string ipv4 = "127.0.0.1";
	std::cout << "Enter IPV4 address of outstation:" << std::endl;
	getline(cin, ipv4);
	
	string bind = "127.0.0.1";
	std::cout << "Enter Binding Address:" << std::endl;
	getline(cin, bind);

	// Specify what log levels to use. NORMAL is warning and above
	// You can add all the comms logging by uncommenting below
	const uint32_t FILTERS = levels::NORMAL | levels::ALL_APP_COMMS;

	// This is the main point of interaction with the stack
	DNP3Manager manager(1, ConsoleLogger::Create());

	// Connect via a TCPClient socket to a outstation
	auto pChannel = manager.AddTCPClient("tcpclient", FILTERS, ChannelRetry::Default(), ipv4, bind, 20000);

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
	stackConfig.link.LocalAddr = 1;
	stackConfig.link.RemoteAddr = 10;

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

	while (true)
	{
		std::cout << "Enter a command" << std::endl;
		std::cout << "x - exits program" << std::endl;
		std::cout << "a - performs and ad-hoc range scan" << std::endl;
		std::cout << "i - integrity demand scan" << std::endl;
		std::cout << "e - exception demand scan" << std::endl;
		std::cout << "d - diable unsolcited" << std::endl;
		std::cout << "r - cold restart" << std::endl;
		std::cout << "c - send crob" << std::endl;

		char cmd;
		std::cin >> cmd;
		switch(cmd)
		{
		case('a') :
			master->ScanRange(GroupVariationID(1, 2), 0, 3);
			break;
		case('d') :
			master->PerformFunction("disable unsol", FunctionCode::DISABLE_UNSOLICITED,
			{ Header::AllObjects(60, 2), Header::AllObjects(60, 3), Header::AllObjects(60, 4) }
			                       );
			break;
		case('r') :
			{
				auto print = [](const RestartOperationResult & result)
				{
					if(result.summary == TaskCompletion::SUCCESS)
					{
						std::cout << "Success, Time: " << result.restartTime.GetMilliseconds() << std::endl;
					}
					else
					{
						std::cout << "Failure: " << TaskCompletionToString(result.summary) << std::endl;
					}
				};
				master->Restart(RestartType::COLD, print);
				break;
			}
		case('x'):
			// C++ destructor on DNP3Manager cleans everything up for you
			return 0;
		case('i'):
			integrityScan.Demand();
			break;
		case('e'):
			exceptionScan.Demand();
			break;
		case('c'):
			{
				ControlRelayOutputBlock crob(ControlCode::LATCH_ON);
				master->SelectAndOperate(crob, 0, PrintingCommandCallback::Get());
				break;
			}
		case('t') :
			{
				channelCommsLoggingEnabled = !channelCommsLoggingEnabled;
				auto levels = channelCommsLoggingEnabled ? levels::ALL_COMMS : levels::NORMAL;
				pChannel->SetLogFilters(levels);
				std::cout << "Channel logging set to: " << levels << std::endl;
				break;
			}
		case('u') :
			{
				masterCommsLoggingEnabled = !masterCommsLoggingEnabled;
				auto levels = masterCommsLoggingEnabled ? levels::ALL_COMMS : levels::NORMAL;
				master->SetLogFilters(levels);
				std::cout << "Master logging set to: " << levels << std::endl;
				break;
			}
		default:
			std::cout << "Unknown action: " << cmd << std::endl;
			break;
		}
	}
	return 0;

}

