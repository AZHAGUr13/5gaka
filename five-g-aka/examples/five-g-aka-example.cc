/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/point-to-point-module.h"
#include "ns3/mmwave-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/netanim-module.h"
#include "ns3/five-g-aka.h"
#include "ns3/myheader.h"
#include "ns3/custom-data-tag.h"
#include "ns3/mmwave-ue-net-device.h"
 #include <iostream>

using namespace ns3;
using namespace mmwave;
NS_LOG_COMPONENT_DEFINE ("5gAKA");
static void Rx (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &from)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << packet->GetSize () << std::endl;
  CustomDataTag tag;
  MyHeader header;
uint32_t kk=packet->PeekHeader (header);
std::cout<<header.GetData ()<<"  "<<kk<<std::endl;
header.Print(std::cout);
    if (packet->PeekPacketTag (tag))
    {
        std::cout<<"\tFrom Node Id: " << tag.GetNodeId() 
                        << "\tPacket Timestamp: " << tag.GetTimestamp() << " delay="<< Now()-tag.GetTimestamp();
    }
    else
    {
      std::cout<<"Not fond\n";
    }
}

/*static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}*/
int 
main (int argc, char *argv[])
{
   Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (1024 * 1024));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::ResourceBlockNum", UintegerValue (1));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::ChunkPerRB", UintegerValue (72));
  double stopTime = 5.9;
  double simStopTime = 7.00;
  Ipv4Address remoteHostAddr;
 // Command line arguments
  CommandLine cmd;
  cmd.Parse (argc, argv);

  Ptr<MmWaveHelper> mmwaveHelper = CreateObject<MmWaveHelper> ();
  //mmwaveHelper->SetSchedulerType ("ns3::MmWaveFlexTtiMaxWeightMacScheduler");
  mmwaveHelper->SetSchedulerType ("ns3::MmWaveFlexTtiMacScheduler");
  Ptr<MmWavePointToPointEpcHelper>  epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  mmwaveHelper->SetEpcHelper (epcHelper);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // parse again so you can override default values from the command line
  cmd.Parse (argc, argv);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (MicroSeconds (1)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create (1);
  ueNodes.Create (1);

  // Install Mobility Model
  MobilityHelper enbmobility;
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector (0.0, 0.0, 0.0));
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.SetPositionAllocator (enbPositionAlloc);
  enbmobility.Install (enbNodes);

  MobilityHelper uemobility;
  Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
  uePositionAlloc->Add (Vector (30.0, 0.0, 0.0));
  uemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  uemobility.SetPositionAllocator (uePositionAlloc);
  uemobility.Install (ueNodes);

  // Install mmwave Devices to the nodes
  NetDeviceContainer enbDevs = mmwaveHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueDevs = mmwaveHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  // Assign IP address to UEs, and install applications
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));
 
  mmwaveHelper->AttachToClosestEnb (ueDevs, enbDevs);
  mmwaveHelper->EnableTraces ();

Ptr<MmWaveUeNetDevice> jj= CreateObject<MmWaveUeNetDevice> ();
  // Set the default gateway for the UE
  Ptr<Node> ueNode = ueNodes.Get (0);
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  // Install and start applications on UEs and remote host
  uint16_t sinkPort = 20000;

  Address sinkAddress (InetSocketAddress (ueIpIface.GetAddress (0), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (enbNodes.Get (0));
  ApplicationContainer sinkApps1 = packetSinkHelper.Install (enbNodes.Get (0));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (simStopTime));

  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (remoteHostContainer.Get (0), TcpSocketFactory::GetTypeId ());
//Add Application

   Ptr<fiveGAka> app = CreateObject<fiveGAka> ();
  app->Setup (ns3TcpSocket, sinkAddress, 1000, 500, DataRate ("30Mb/s"));
  remoteHostContainer.Get (0)->AddApplication (app);
 AsciiTraceHelper asciiTraceHelper;
 // Ptr<OutputStreamWrapper> stream1 = asciiTraceHelper.CreateFileStream ("mmWave-tcp-window.txt");
  //ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream1));

  Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream ("mmWave-tcp-data.txt");
  sinkApps.Get (0)->TraceConnectWithoutContext ("Rx",MakeBoundCallback (&Rx, stream2));
 //sinkApps.Get (1)->TraceConnectWithoutContext ("Rx",MakeBoundCallback (&Rx1, stream2));
//sinkApps.Get (2)->TraceConnectWithoutContext ("Rx",MakeBoundCallback (&Rx2, stream2));

  app->SetStartTime (Seconds (0.1));
  app->SetStopTime (Seconds (stopTime));
 p2ph.EnablePcapAll ("mmwave-sgi-capture");

  
  AnimationInterface anim("azh.xml");
  anim.SetConstantPosition(ueNodes.Get(0),0.0,75.0);
Simulator::Stop (Seconds (simStopTime));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}


