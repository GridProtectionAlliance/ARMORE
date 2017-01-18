/*
 * Licensed to Green Energy Corp (www.greenenergycorp.com) under one or
 * more contributor license agreements. See the NOTICE file distributed
 * with this work for additional information regarding copyright ownership.
 * Green Energy Corp licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This project was forked on 01/01/2013 by Automatak, LLC and modifications
 * may have been made to this file. Automatak, LLC licenses these modifications
 * to you under the terms of the License.
 */
#include <asiodnp3/DNP3Manager.h>
#include <asiodnp3/PrintingSOEHandler.h>
#include <asiodnp3/ConsoleLogger.h>
#include <asiodnp3/MeasUpdate.h>

#include <asiopal/UTCTimeSource.h>
#include <opendnp3/outstation/SimpleCommandHandler.h>

#include <opendnp3/outstation/Database.h>

#include <opendnp3/LogLevels.h>

#include <string>
#include <thread>
#include <iostream>

using namespace std;
using namespace opendnp3;
using namespace openpal;
using namespace asiopal;
using namespace asiodnp3;

void ConfigureDatabase(DatabaseConfigView view)
{
	// example of configuring analog index 0 for Class2 with floating point variations by default
	view.analogs[0].variation = StaticAnalogVariation::Group30Var5;
	view.analogs[0].metadata.clazz = PointClass::Class2;
	view.analogs[0].metadata.variation = EventAnalogVariation::Group32Var7;
}

int main(int argc, char* argv[])
{
	string bind = "127.0.0.1";
	std::cout << "Enter Binding Address:" << std::endl;
	getline(cin, bind);

	// Specify what log levels to use. NORMAL is warning and above
	// You can add all the comms logging by uncommenting below.
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
	stackConfig.link.LocalAddr = 10;
	stackConfig.link.RemoteAddr = 1;

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

	while (true)
	{
		std::cout << "Enter one or more measurement changes then press <enter>" << std::endl;
		std::cout << "c = counter, b = binary, d = doublebit, a = analog, x = exit" << std::endl;
		std::cin >> input;

		for (char & c : input)
		{
			switch (c)
			{
			case('c') :
				{
					MeasUpdate tx(outstation, UTCTimeSource::Instance().Now());
					tx.Update(Counter(count), 0);
					++count;
					break;
				}
			case('a') :
				{
					MeasUpdate tx(outstation, UTCTimeSource::Instance().Now());
					tx.Update(Analog(value), 0);
					value += 1;
					break;
				}
			case('b') :
				{
					MeasUpdate tx(outstation, UTCTimeSource::Instance().Now());
					tx.Update(Binary(binary), 0);
					binary = !binary;
					break;
				}
			case('d') :
				{
					MeasUpdate tx(outstation, UTCTimeSource::Instance().Now());
					tx.Update(DoubleBitBinary(dbit), 0);
					dbit = (dbit == DoubleBit::DETERMINED_OFF) ? DoubleBit::DETERMINED_ON : DoubleBit::DETERMINED_OFF;
					break;
				}
			case('t') :
				{
					channelCommsLoggingEnabled = !channelCommsLoggingEnabled;
					auto levels = channelCommsLoggingEnabled ? levels::ALL_COMMS : levels::NORMAL;
					channel->SetLogFilters(levels);
					std::cout << "Channel logging set to: " << levels << std::endl;
					break;
				}
			case('u') :
				{
					outstationCommsLoggingEnabled = !outstationCommsLoggingEnabled;
					auto levels = outstationCommsLoggingEnabled ? levels::ALL_COMMS : levels::NORMAL;
					outstation->SetLogFilters(levels);
					std::cout << "Outstation logging set to: " << levels << std::endl;
					break;
				}
			case('x') :

				// DNP3Manager destructor cleanups up everything automagically
				return 0;

			default:
				std::cout << "No action registered for: " << c << std::endl;
				break;
			}
		}

	}

	return 0;
}
