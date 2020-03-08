/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "serving-network-aka.h"
#include "ns3/point-to-point-module.h"
#include "ns3/user-equipment-aka.h"
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
#include "ns3/custom-data-tag.h"
#include "ns3/net-device.h"
#include "ns3/myheader.h"
#include<string>
#include<map>
#include<iostream>
#include "ns3/log.h"
#include "ns3/simulator.h"
namespace ns3 {

/* ... */
NS_LOG_COMPONENT_DEFINE("SeingNetworkAka");
  NS_OBJECT_ENSURE_REGISTERED(ServingNetworkAka);

typedef unsigned char u8;
ServingNetworkAka::~ServingNetworkAka ()
{
}

static void Rx (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &from)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << packet->GetSize () << std::endl;
  CustomDataTag tag;
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

void
ServingNetworkAka::StartApplication (void)
{
    n = GetNode ();
    uint16_t sinkPort = 20000;

    for(unsigned int i=0;i<16;i++)
        R1[i] = 0x63;
    // send to UE
    
    ue.setString(R1,suci);
    
    

     for(unsigned int i=0;i<8;i++)
    {
      std::cout<<int(suci[i])<<" ";
    }
    std::cout<<"\nSUCI in SN\n";

  //send to HN
  // Will send xmac,hres,ek
  u8 xmac[8],hres[32],ek[32],resUE[32];
  hn.setString(R1,suci,xmac,hres,ek);

  // UE Calculation only x amc is passed returns true or false
  //Also assign res value 
  bool macVerification = ue.calculation(xmac,resUE);
  bool resVerification = true; 
  std::cout<<"\n Verification at UE at SN :"<<macVerification<<std::endl;
  std::cout<<"\nRES Token Verification At SN:\n";
  for(unsigned int i=0;i<8;i++)
  {
    std::cout<<int(resUE[i])<<" "<<(int(hres[i]))<<std::endl;
    if((int(resUE[i])) != (int(hres[i])) )
        resVerification = false;
    }
  std::cout<<"\n Verification at UE at SN :"<<resVerification<<std::endl;

  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer m_peer = packetSinkHelper.Install (n);
  //ApplicationContainer sinkApps1 = packetSinkHelper.Install (enbNodes.Get (0));
  m_peer.Start (Seconds (0.));
  m_peer.Stop (Seconds (10));
    dev = n->GetDevice (0);
    AsciiTraceHelper asciiTraceHelper;
    Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream ("mmWave-tcp-data.txt");
    
  m_peer.Get (0)->TraceConnectWithoutContext ("Rx",MakeBoundCallback (&Rx, stream2));
 //Simulator::Schedule (Seconds(0.2), &ServingNetworkAka::SendPacket, this);
 // m_socket->Bind ();
  //m_socket->Connect (m_peer);
  //SendPacket ();
    
}
void ServingNetworkAka::ReceivePacket (Ptr<NetDevice>, Ptr<const Packet>,uint16_t, const Address)
{
  std::cout<<"Received called\n";
}
void
ServingNetworkAka::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
ServingNetworkAka::SendPacket (void)
{
  static int send_num = 1;
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  NS_LOG_FUNCTION (this);
    //Setup transmission parameters
    /*MyHeader h1;
 
  h1.SetData (10);
// Fill out udpHeader fields appropriately
packet->AddHeader (h1);*/
  CustomDataTag tag;
    tag.SetNodeId (5) ;
    //tag.SetPosition ( GetNode()->GetObject<MobilityModel>()->GetPosition());
    //timestamp is set in the default constructor of the CustomDataTag class as Simulator::Now()

    //attach the tag to the packet
    packet->AddPacketTag (tag);
    Ptr<Socket> m_socket = Socket::CreateSocket (n,TcpSocketFactory::GetTypeId ());
    uint32_t ab=m_socket->SendTo (packet,0,m_peer);
//m_socket->Send (packet);
std::cout<<"Packet send"<<ab<<std::endl;
 
  NS_LOG_DEBUG ("Sending:    " << send_num++ << "\t" << Simulator::Now ().GetSeconds ());
AsciiTraceHelper asciiTraceHelper;
    Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream ("mmWave-tcp-data.txt");
 
 //  m_peer.Get(0)->TraceConnectWithoutContext ("Rx",MakeBoundCallback (&Rx, stream2));
 
}


ServingNetworkAka::ServingNetworkAka ()
{
  nodes.Create(1);
  enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector (0.0, 0.0, 0.0));
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.SetPositionAllocator (enbPositionAlloc);
  enbmobility.Install (nodes);
  for(unsigned int i=0;i<16;i++)
      R1[i] = 0x63;
    // send to UE
}
void ServingNetworkAka::getR1(u8 R[])
{
 for(unsigned int i=0;i<16;i++)
      R[i] = R1[i];
}
NodeContainer ServingNetworkAka::getNode()
{
  return nodes;
}
/*def singleKeyGenerate( rand1, rand):
	sumDigit=0
	for i in rand1:
		sumDigit += indexMap[i]
	print("addition:",sumDigit)	
	sumDigit = sumDigit % len(rand)
	print("mod:",sumDigit)
	rotate = rotr(rand, sumDigit )
	print("Rotation :",rotate)
	addition = add(rand1, rotate)
	complement = comple(addition)
	print("complem :",complement)
	return complement,rand1*/
		
}
