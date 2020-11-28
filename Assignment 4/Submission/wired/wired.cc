/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <fstream>
#include <string>
#include <time.h>
#include <iostream>
#include <map>
#include "ns3/gnuplot.h"
#include "ns3/core-module.h"
#include "ns3/packet-sink.h"
#include "ns3/network-module.h"
//#include "ns3/netanim-module.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/internet-module.h"
//#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-layout-module.h"

using namespace ns3;
using namespace std;

//defining a log component named wired
NS_LOG_COMPONENT_DEFINE ("wired");

//Class for client application, Taken from seven.cc of the exmaple tutorial of ns-3.
class ClientApp : public Application
{
public:
  ClientApp (); //Constructor
  virtual ~ClientApp (); //Deconstructor

  //Initialize the object parameters
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;	//Socket
  Address         m_peer; //Address of receiver
  uint32_t        m_packetSize;	//Packet Size
  uint32_t        m_nPackets;	//Total number of packets to be sent
  DataRate        m_dataRate;	//Data rate
  EventId         m_sendEvent;
  bool            m_running;	//State of runnning
  uint32_t        m_packetsSent; //Number of packets sent
};

//Constructor provides initial value to all the variables.
ClientApp::ClientApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

ClientApp::~ClientApp ()
{
  m_socket = 0;
}

//Setup initializes all the parameters.
void
ClientApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

//Start the application
void
ClientApp::StartApplication (void)
{
  m_running = true; //Set the running state to true
  m_packetsSent = 0; //Set the number of packets sent to 0
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind ();
    }
  else
    {
      m_socket->Bind6 ();
    }
    //Connect to the peer after binding
  m_socket->Connect (m_peer);
  //Send the packets
  SendPacket ();
}

//Stop Application function
void
ClientApp::StopApplication (void)
{
  m_running = false; //Set running state to false

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close (); //close the socket
    }
}

//Funciton to send the packet
void
ClientApp::SendPacket (void)
{
	//Create a new packet of given packet size and send it
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  //In case the last packet is also sent, schedule for closure.
  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
ClientApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &ClientApp::SendPacket, this);
    }
}

int
main (int argc, char *argv[])
{

	Time::SetResolution (Time::NS);
	//enabling the log component
	LogComponentEnable("wired" , LOG_INFO);

	//string variable which stores the TCP agent
	string agent;

	//Taking the TCP agent via a command line parameter
	CommandLine cmd;
	cmd.AddValue("agent" , "Specifies the TCP Agent to be used, available options are Westwood,Veno and Vegas" , agent);
	cmd.Parse (argc, argv);

	//setting TCP agent entered by user and checking if invalid input given
	if(agent == "Westwood")
    {
    	Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpWestwood"));
  	}
  	else if(agent == "Veno")
    {
  	  	Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVeno"));
  	}
  	else if(agent == "Vegas")
    {
  	  	Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVegas"));
  	}
  	else
    {
      	NS_LOG_INFO("Invalid TCP Agent entered. Exiting.");
  	  	exit(0);
    }
	
    NS_LOG_INFO("TCP Agent set to TCP " + agent);

    //setting seed of rand() using system time
	srand(time(NULL));

  	//setting options for generating plot file using Gnuplot
  	string graphicsFileName        = "wiredTcp" + agent + ".png";
  	string plotFileName            = "wiredTcp" + agent + ".plt";
  	string plotTitle               = "Throughput plot for TCP " + agent;
  	string dataTitle               = "TCP " + agent;
  	string traceFileName           = "wiredTcp" + agent + "_trace.txt";
  	string subheader               = "Packet Size     Throughput    Fairness Index\n-----------     ----------    --------------\n";
  	string header                  = "TCP " + agent + "\n" + subheader;

  	Gnuplot plot (graphicsFileName);
  	plot.SetTitle (plotTitle);

  	plot.SetTerminal ("png");

  	plot.SetLegend ("Packet Size (in bytes)", "Throughput (in Kbps)");

  	plot.AppendExtra ("set xrange [20:1520]");

  	Gnuplot2dDataset dataset;
  	dataset.SetTitle (dataTitle);
  	dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  	//trace helper for creating trace files
  	AsciiTraceHelper traceHelper;

  	//setting output stream which will write into file
  	Ptr<OutputStreamWrapper> fout = traceHelper.CreateFileStream (traceFileName);
  	*fout->GetStream () << header;
  	cout << subheader;

  	//setting packet size values as given in question
  	int numSize = 10;
  	int size[numSize] = {40, 44, 48, 52, 60, 552, 576, 628, 1420, 1500};
  	int maxPacketsInQueue;

  	for(int i=0;i<numSize;i++)
    {
      	int packetSize=size[i];
      	//setting segment size
      	Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (packetSize));

      	//creating nodes
      	NodeContainer p2pNodes;
      	p2pNodes.Create (4);

      	//creating different types of links
      	PointToPointHelper HundredMbpsTwentymsLink;
      	HundredMbpsTwentymsLink.SetDeviceAttribute("DataRate" , StringValue("100Mbps") );
      	HundredMbpsTwentymsLink.SetChannelAttribute("Delay" , StringValue("20ms") );
      	maxPacketsInQueue = (100*20*1000)/(8*packetSize);
      	HundredMbpsTwentymsLink.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", StringValue(to_string(maxPacketsInQueue)+"p")); 

      	PointToPointHelper TenMbpsFiftymsLink;
      	TenMbpsFiftymsLink.SetDeviceAttribute("DataRate" , StringValue("10Mbps") );
      	TenMbpsFiftymsLink.SetChannelAttribute("Delay" , StringValue("50ms") );
      	maxPacketsInQueue = (10*50*1000)/(8*packetSize);
      	TenMbpsFiftymsLink.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", StringValue(to_string(maxPacketsInQueue)+"p"));

      	//creating net devices by setting up links between nodes 
      	NetDeviceContainer Node2R1 , R1R2 , R2Node3;
      	Node2R1 = HundredMbpsTwentymsLink.Install( p2pNodes.Get(0) , p2pNodes.Get(1) );
      	R1R2 = TenMbpsFiftymsLink.Install(p2pNodes.Get(1) , p2pNodes.Get(2) );
      	R2Node3 = HundredMbpsTwentymsLink.Install(p2pNodes.Get(2) , p2pNodes.Get(3) );

      	//setting up internet stack and installing all nodes onto it
      	InternetStackHelper internet;
      	internet.Install(p2pNodes);

      	//setting up ipv4 ip addresses
      	Ipv4AddressHelper ipv4_Node2R1;
      	ipv4_Node2R1.SetBase( "10.1.1.0" , "255.255.255.0" );
      	Ipv4InterfaceContainer Node2R1Interface = ipv4_Node2R1.Assign ( Node2R1 );

      	Ipv4AddressHelper ipv4_R1R2;
      	ipv4_R1R2.SetBase( "10.1.2.0" , "255.255.255.0" );
      	Ipv4InterfaceContainer R1R2Interface = ipv4_R1R2.Assign ( R1R2 );

      	Ipv4AddressHelper ipv4_R2Node3;
      	ipv4_R2Node3.SetBase( "10.1.3.0" , "255.255.255.0" );
      	Ipv4InterfaceContainer R2Node3Interface = ipv4_R2Node3.Assign ( R2Node3 );

      	//choosing a random port number using rand()
      	uint16_t port_num = 8888 + (rand()%2000);

      	//setting up server address
      	Address server_addr = InetSocketAddress (R2Node3Interface.GetAddress(1),port_num);
      	Address any_addr = InetSocketAddress(Ipv4Address::GetAny(),port_num);

      	//populating routing tables
      	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

      	//creating application container for sink
      	ApplicationContainer Sink;

      	//setting up packet helper for sink
      	PacketSinkHelper sink ("ns3::TcpSocketFactory", any_addr);
      	Sink = (sink.Install(p2pNodes.Get(3)));

      	//Create a client socket
      	Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (p2pNodes.Get (0), TcpSocketFactory::GetTypeId ());
      	//Create an application
      	Ptr<ClientApp> clientApp = CreateObject<ClientApp> ();
      	//Setup the application
      	clientApp->Setup (ns3TcpSocket, server_addr, packetSize, 10000, DataRate ("20Mbps"));
      	//Add the application onto the client
      	p2pNodes.Get (0)->AddApplication (clientApp);
      	//Set the start and stop times for the client
      	clientApp->SetStartTime (Seconds (1.));
      	clientApp->SetStopTime (Seconds (20.));

        //Create a flow monitor
		FlowMonitorHelper flowmon;
		Ptr<FlowMonitor> monitor = flowmon.InstallAll();
		
		//Start the simulation
		Simulator::Stop (Seconds (20));
	 //    AnimationInterface anim ("animation.xml");
	 //    MobilityHelper mobility;
	 //    Ptr<ListPositionAllocator> locationVector = CreateObject<ListPositionAllocator> ();
	 //    locationVector->Add (Vector (0.0, 0.0, 0.0));
	 //    locationVector->Add (Vector (20.0, 20.0, 0.0));
	 //    locationVector->Add (Vector (40.0, 40.0, 0.0));
	 //    locationVector->Add (Vector (60.0, 60.0, 0.0));

	 //    mobility.SetPositionAllocator (locationVector);
	 //    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	 //    mobility.Install (p2pNodes.Get(0));
	 //    mobility.Install (p2pNodes.Get(1));
	 //    mobility.Install (p2pNodes.Get(2));
	 //    mobility.Install (p2pNodes.Get(3));
		
		Simulator::Run ();

		//Get the stats from the flow monitor
		map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

		//initialize a few variables
		double totTime, totBits, throughput, sumThr, sumsqThr;
		sumThr = 0;
		sumsqThr = 0;
		int coun = 0;

		//Update the total bits sent, and the total time.
		auto iter = stats.begin();
		totBits = 8.0 * iter->second.rxBytes;
		totTime = iter->second.timeLastRxPacket.GetSeconds();
		totTime -= iter->second.timeFirstTxPacket.GetSeconds();

		//calculate throughput and find the fairness index.
		throughput = totBits/(1000*totTime);
		sumThr+= throughput;
		sumsqThr += throughput*throughput;
		coun++;

		//Output thr information
		double fairness = (sumThr*sumThr)/(sumsqThr*(coun+0.0));

		//Also output the serialized data in xml format
		monitor->SerializeToXmlFile("wired_"+agent+"_"+std::to_string(packetSize)+".xml", true, true);

		//end simulation
		Simulator::Destroy ();

		//writing out to file
      	*fout->GetStream () <<  "   " <<std::to_string(packetSize) << "\t\t" << std::to_string(throughput) <<"\t    "<<fairness << "\n";
      	cout <<  "   " <<std::to_string(packetSize) << "\t\t" << std::to_string(throughput) <<"\t    "<<fairness << "\n";

      	//adding values to dataset
      	dataset.Add (packetSize, throughput);

    }

    //adding dataset and generating output file
  	plot.AddDataset (dataset);

  	ofstream plotFile (plotFileName.c_str());

  	plot.GenerateOutput (plotFile);

  	plotFile.close ();

  	return 0;  
}