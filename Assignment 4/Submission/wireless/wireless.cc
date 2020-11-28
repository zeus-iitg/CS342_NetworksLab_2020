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
#include <map>
#include <iostream>
#include "ns3/enum.h"
#include "ns3/gnuplot.h"
#include "ns3/core-module.h"
#include "ns3/packet-sink.h"
#include "ns3/wifi-module.h"
#include "ns3/network-module.h"
//#include "ns3/netanim-module.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-layout-module.h"

using namespace ns3;
using namespace std;

//defining a log component named wireless
NS_LOG_COMPONENT_DEFINE ("wireless");

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
main(int argc, char** argv)
{

  	Time::SetResolution (Time::NS);
  	//enabling the log component
  	LogComponentEnable("wireless" , LOG_INFO);

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
  	string graphicsFileName        = "wirelessTcp" + agent + ".png";
  	string plotFileName            = "wirelessTcp" + agent + ".plt";
  	string plotTitle               = "Throughput plot for TCP " + agent;
  	string dataTitle               = "TCP " + agent;
  	string traceFileName           = "wirelessTcp" + agent + "_trace.txt";
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
	    // Configure Default Wifi Parameters
	    Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("999999"));
	    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("999999"));
	    Config::SetDefault("ns3::WifiMacQueue::DropPolicy", EnumValue(WifiMacQueue::DROP_NEWEST));

    	//creating nodes
    	NodeContainer nodes;
    	nodes.Create(4);
    
    	//setting up Wired link

    	//setting up p2p link
    	PointToPointHelper p2pForBS;
    	p2pForBS.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    	p2pForBS.SetChannelAttribute("Delay", StringValue("100ms"));

    	//compute maximum packets queue can handle
    	maxPacketsInQueue = (10*100*1000)/(8*packetSize);

    	//setting up queue size
    	p2pForBS.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(to_string(maxPacketsInQueue) + "p"));
    
    	//creating net devices by setting up links between nodes 
    	NetDeviceContainer bsDevices = p2pForBS.Install(nodes.Get(1), nodes.Get(2));

    	//setting up Wireless link
    	YansWifiChannelHelper channelHelperFirst = YansWifiChannelHelper::Default();
    	Ptr<YansWifiChannel> channelFirst = channelHelperFirst.Create();
    
    	YansWifiChannelHelper channelHelperSecond = YansWifiChannelHelper::Default();
    	Ptr<YansWifiChannel> channelSecond = channelHelperSecond.Create();

    	//creating physical helpers
    	YansWifiPhyHelper phyHelperFirst = YansWifiPhyHelper::Default();
    
    	YansWifiPhyHelper phyHelperSecond = YansWifiPhyHelper::Default();

    	//creating wifi helper
    	WifiHelper wifi;
    	wifi.SetRemoteStationManager("ns3::AarfWifiManager"); 
    	WifiMacHelper mac;

    	Ssid wifiSsid = Ssid("ns3-wifi");

    	//setting access point configuration
    	mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(wifiSsid));

    	phyHelperFirst.SetChannel(channelFirst);
    	phyHelperSecond.SetChannel(channelSecond);

    	// This is the device on the side of BS0 from N0
    	NetDeviceContainer accessPointFirst = wifi.Install (phyHelperFirst, mac, nodes.Get(1));

    	// This is the device on the side of BS1 to N1
    	NetDeviceContainer accessPointSecond = wifi.Install (phyHelperSecond, mac, nodes.Get(2));

    	//Station configuration for N0/N1
    	mac.SetType ("ns3::StaWifiMac","Ssid", SsidValue(wifiSsid));

    	// This is the device on the side of N0 to BS0
    	NetDeviceContainer endDeviceFirst = wifi.Install(phyHelperFirst, mac, nodes.Get(0));

    	// This is the device on the side of N1 from BS1
    	NetDeviceContainer endDeviceSecond = wifi.Install(phyHelperSecond, mac, nodes.Get(3));


    	// Set constant positions for all the devices
    	// AnimationInterface anim ("animation.xml");
    	MobilityHelper mobility;

    	Ptr<ListPositionAllocator> locationVector = CreateObject<ListPositionAllocator> ();
    
	    locationVector -> Add(Vector(0.0, 0.0, 0.0));
	    locationVector -> Add(Vector(20.0, 20.0, 0.0));
	    locationVector -> Add(Vector(40.0, 40.0, 0.0));
	    locationVector -> Add(Vector(60.0, 60.0, 0.0));

    	mobility.SetPositionAllocator (locationVector);
    	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

    	mobility.Install(nodes.Get(0));
    	mobility.Install(nodes.Get(1));
    	mobility.Install(nodes.Get(2));
    	mobility.Install(nodes.Get(3));

    	//setting up internet stack and installing all nodes onto it
    	InternetStackHelper stackHelper;
    	stackHelper.Install(nodes); 

    	//setting up ipv4 ip addresses
    	Ipv4AddressHelper addressHelper;

    	NetDeviceContainer pathLeft(endDeviceFirst, accessPointFirst);
    	NetDeviceContainer pathRight(accessPointSecond, endDeviceSecond);

	    addressHelper.SetBase("10.1.1.0", "255.255.255.0");
	    Ipv4InterfaceContainer InterfaceLeft = addressHelper.Assign(pathLeft);
	    
	    addressHelper.SetBase("10.1.2.0", "255.255.255.0");
	    Ipv4InterfaceContainer InterfaceMiddle = addressHelper.Assign(bsDevices);

	    addressHelper.SetBase("10.1.3.0", "255.255.255.0");
	    Ipv4InterfaceContainer InterfaceRight = addressHelper.Assign(pathRight);

    	//populating routing tables
    	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    	//choosing a random port number using rand()
    	uint16_t port_num = 8888 + (rand()%2000);
 
    	//setting up sink address
    	Address sink_addr = InetSocketAddress(InterfaceRight.GetAddress(1), port_num);


    	//setting up packet helper for sink
    	PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port_num));
    	ApplicationContainer Sink = sink.Install(nodes.Get(3));

    	// Set timeframe for sink Application
    	Sink.Start(Seconds(0.0));
    	Sink.Stop(Seconds(20.0));


    	// Create a TCP based client that works on the Sender side.
    	Ptr<Socket> tcpSocket = Socket::CreateSocket(nodes.Get(0), TcpSocketFactory::GetTypeId());
    	Ptr<ClientApp> source = CreateObject<ClientApp>();

    	// Send a large number of packets to attain steady state
    	source -> Setup(tcpSocket, sink_addr, packetSize, 1000, DataRate("100Mbps"));
    	nodes.Get(0) -> AddApplication(source);

    	//setting start and stop times
    	source -> SetStartTime(Seconds(0.0));
    	source -> SetStopTime(Seconds(20.0));
    
    	// Install Flow Monitors on all devices
    	Ptr<FlowMonitor> monitor = (new FlowMonitorHelper()) -> InstallAll();

    	// Run the Simulator
    	Simulator::Stop(Seconds(20.0));
    	Simulator::Run();

    	// Get all flow statistics
    	map < FlowId, FlowMonitor::FlowStats > flowStats = monitor -> GetFlowStats();

    	auto iter = flowStats.begin();

    	double sumThr, sumsqThr;
		sumThr = 0;
		sumsqThr = 0;
		int coun = 0;

    	// Compute required statistics and display them.
    	double rxBits = 8.0 * iter->second.rxBytes;

    	// Total time is just delay between last packet received
    	// and first packet sent
    	double timeTaken = iter->second.timeLastRxPacket.GetSeconds() - iter->second.timeFirstTxPacket.GetSeconds();
    
    	double throughput =  rxBits / (1000*timeTaken);

    	// Compute Jains fairness index
		sumThr+= throughput;
		sumsqThr += throughput*throughput;
		coun++;

		//Output thr information
		double fairness = (sumThr*sumThr)/(sumsqThr*(coun+0.0));

    	//Also output the serialized data in xml format
		monitor->SerializeToXmlFile("wireless_"+agent+"_"+std::to_string(packetSize)+".xml", true, true);

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