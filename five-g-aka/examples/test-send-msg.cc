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
#include "ns3/custom-data-tag.h"
#include "ns3/serving-network-aka.h"
#include "ns3/user-equipment-aka.h"
#include "ns3/myheader.h"
#include <iostream>

using namespace ns3;
using namespace mmwave;
NS_LOG_COMPONENT_DEFINE ("test");

  UserEqupi ue;
  ServingNetworkAka se;
  void ReceivePacketSN (Ptr<Socket> socket)
{
	if(Ptr<Packet> packet=socket->Recv()){

		std::cout<<"SN e\n";
    MyHeader header;
    uint32_t kk=packet->PeekHeader (header);

    std::cout<<"\n In Rev\n"<<kk;
   u8 SUCI[16];
    header.GetData(SUCI);
    for(unsigned int i=0;i<16;i++)
    {
      std::cout<<SUCI[i]<<" ";
    }
    //ue.setString(R1,SUCI);
    std::cout<<"\n In Rev\n";

 

  }
    
}
  static void sendSUCI (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval, u8 suci[])
{
  if (pktCount > 0)
    {
      Ptr<Packet> packet = Create<Packet> (pktSize);
      //Ptr<MyHeader> header = CreateObject<MyHeader>();
      MyHeader h1;
      h1.SetData (suci,uint16_t(16));
      // Fill out udpHeader fields appropriately
      packet->AddHeader (h1);
      socket->Send (packet);

      	std::cout<<"Send SUCI\n";
    }
  else
    {
      socket->Close ();
    }
}

void ReceivePacket (Ptr<Socket> socket)
{
	if(Ptr<Packet> packet=socket->Recv()){

		std::cout<<"One e\n";
    MyHeader header;
    uint32_t kk=packet->PeekHeader (header);

    std::cout<<"\n In Rev\n"<<kk;
    Address addr;
    socket->GetSockName(addr);
    InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(addr);
    std::cout<<iaddr.GetIpv4().Get();;
    u8 R1[16],SUCI[16];
    header.GetData(R1);
    for(unsigned int i=0;i<16;i++)
    {
      std::cout<<R1[i]<<" ";
    }
    ue.setString(R1,SUCI);

    std::cout<<"\n In Rev\n";

  // SEND SUCI To SN
    Simulator::ScheduleWithContext (socket->GetNode ()->GetId (),
                                  Seconds (1.0), &sendSUCI,
                                  socket, int(1000), int(1), Seconds(1),SUCI);

  }
    
}
/*static void sendToUE (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval, ServingNetworkAka se)
{
  if (pktCount > 0)
    {
      Ptr<Packet> packet = Create<Packet> (pktSize);
      //Ptr<MyHeader> header = CreateObject<MyHeader>();
      MyHeader h1;
  u8 R1[16];
  se.getR1(R1);
      h1.SetData (R1,uint16_t(16));
      // Fill out udpHeader fields appropriately
      packet->AddHeader (h1);
      socket->Send (packet);

      	std::cout<<"Send \n";
    }
  else
    {
      socket->Close ();
    }
}*/


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
 // double simStopTime = 7.00;
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
  
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);


  // Install mmwave Devices to the nodes
  NetDeviceContainer enbDevs = mmwaveHelper->InstallEnbDevice (se.getNode());
  NetDeviceContainer ueDevs = mmwaveHelper->InstallUeDevice (ue.getNode());

  // Install the IP stack on the UEs
  // Assign IP address to UEs, and install applications
  internet.Install (ue.getNode());
  //internet.Install (se.getNode());
  NetDeviceContainer internetDevices = p2ph.Install (ue.getNode().Get(0),se.getNode().Get(0));
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));

  //mmwaveHelper->AttachToClosestEnb (ueDevs, enbDevs);
  //mmwaveHelper->EnableTraces ();

TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
Ptr<Socket> recvSink = Socket::CreateSocket (ue.getNode().Get (0), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));
  recvSink->SetAllowBroadcast (true);
  recvSink->Connect (local);
 Ptr<Socket> source = Socket::CreateSocket (se.getNode().Get (0), tid);

  InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
  source->SetAllowBroadcast (true);
  source->Connect (remote);
  source->Bind (remote);
  source->SetRecvCallback (MakeCallback (&ReceivePacketSN));
 u8 SUCI[]={0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac};
 /* Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                  Seconds (1.0), &sendToUE,
                                  source, int(1000), int(1), Seconds(1),se);*/
                                    Simulator::ScheduleWithContext (recvSink->GetNode ()->GetId (),
                                  Seconds (1.0), &sendSUCI,
                                  recvSink, int(1000), int(1), Seconds(1),SUCI);

Simulator::Stop (Seconds (stopTime+10));
  AnimationInterface anim ("anim1.xml");
  Simulator::Run ();
  Simulator::Destroy ();
  
  
  return 0;// Set the default gateway for the UE
 /* Ptr<Node> ueNode = ueNodes.Get (0);
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  // Install and start applications on UEs and remote host
  uint16_t sinkPort = 20000;

  Address sinkAddress (InetSocketAddress (ueIpIface.GetAddress (0), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (ueNodes.Get (0));
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

  */
}


