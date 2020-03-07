/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "five-g-aka.h"
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
#include "ns3/custom-data-tag.h"
#include "myheader.h"
#include<string>
#include<map>
#include<iostream>
#include "ns3/log.h"
#include "ns3/simulator.h"
namespace ns3 {

/* ... */
NS_LOG_COMPONENT_DEFINE("fiveGAka");
  NS_OBJECT_ENSURE_REGISTERED(fiveGAka);
/*

fiveGAka::fiveGAka()
{
        indexMap.insert(std::make_pair('0',0));
        indexMap.insert(std::make_pair('1',1));
        indexMap.insert(std::make_pair('2',2));
        indexMap.insert(std::make_pair('3',3));
        indexMap.insert(std::make_pair('4',4));
        indexMap.insert(std::make_pair('5',5));
        indexMap.insert(std::make_pair('6',6));
        indexMap.insert(std::make_pair('7',7));
        indexMap.insert(std::make_pair('8',8));   
        indexMap.insert(std::make_pair('9',9));
        indexMap.insert(std::make_pair('A',10));   
        indexMap.insert(std::make_pair('B',11));
        indexMap.insert(std::make_pair('C',12));
        indexMap.insert(std::make_pair('D',13));
        indexMap.insert(std::make_pair('E',14));
        indexMap.insert(std::make_pair('F',15));
}
fiveGAka::~fiveGAka()
{

}
char fiveGAka::findKey(int t)
{
        for(auto itr= indexMap.begin(); itr!= indexMap.end();itr++)
                {
                        if(itr->second == t)
                         return    itr->first;
                }
        return '0';
}
void
fiveGAka::StartApplication(){
        NS_LOG_FUNCTION (this);
         Ptr<Node> n = GetNode ();
          Ptr<NetDevice> dev= n->GetDevice (0);
          Ptr<NetDevice> dev1= n->GetDevice (1);
          std::cout<<dev->GetInstanceTypeId ();
          std::cout<<dev1->GetInstanceTypeId ();
        std::cout<<"I m here";
         Simulator::Schedule (Seconds(5), &fiveGAka::tttt, this,7);
}
void fiveGAka::tttt(int a){
        std::cout<<"Called\n"<<a;
}

   
TypeId fiveGAka::GetTypeId()
{
    static TypeId tid = TypeId("ns3::fiveGAka")
                .SetParent <Application> ()
                .AddConstructor<fiveGAka> ()
                 ;
    return tid;
}

TypeId fiveGAka::GetInstanceTypeId() const
{
    return fiveGAka::GetTypeId();
}
//Encrytion algo 
std::string fiveGAka::aes(std::string publicKey,std::string supi,std::string freshKey,std::string snid)
{
        std::string ans="";
        for( unsigned int i=0; i<publicKey.size();i++)
        {
                int t= indexMap[publicKey[i]]^indexMap[supi[i]]^indexMap[freshKey[i]]^indexMap[snid[i]];                
                ans +=  findKey(t%16); 
        }
        return ans;
} 
std::string fiveGAka::comple(std::string r1)
{
        
        std::string ans="";
        for( unsigned int i=0; i<r1.size(); i++)
         {
               int t = 15 - indexMap[r1[i]];
               ans +=  findKey(t);              
         }
	return ans;
}    
std::string fiveGAka::add(std::string r1,std::string r2)
{
        if(r1.size()!=r2.size())
                return NULL;   
        std::string ans="";
        for( unsigned int i=0; i<r1.size(); i++)
         {
               int t = (indexMap[r1[i]]+indexMap[r2[i]])%16;
               ans +=  findKey(t); 
        }             
        
	return ans;
}
std::string fiveGAka::rotr(std::string r, int sumDigit)
{
        std::string ans="";
        for( unsigned int i=sumDigit; i<r.size(); i++)
                ans+=r[i];
        for( int i=0; i<sumDigit; i++)
                ans+=r[i];
	
	return ans;
}
std::string fiveGAka::freshkey(std::string r1, std::string r2)
{
        int sumDigit = 0;
        for(unsigned int i=0; i<r1.size(); i++)
        {
                sumDigit += indexMap[r1[i]];
                std::cout<<sumDigit<<" "<<r1[i]<<"  "<<i<<std::endl;
        }
        std::cout<<"\n Individual Addition:"<<sumDigit;
        sumDigit = sumDigit % r2.size();
        std::cout<<"\n Mod:"<<sumDigit;
        std::string rotate = rotr(r2, sumDigit );
	std::cout<<"\nRotation :"<<rotate;
        std::string addition = add(r1,rotate);
        std::cout<<"\n  Addition:"<<addition;
        std::string com = comple(addition);
        std::cout<<"\n  Compl:"<<com;
        return com;
}
*/
fiveGAka::~fiveGAka ()
{
  m_socket = 0;
}

void
fiveGAka::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
fiveGAka::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
fiveGAka::StopApplication (void)
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
fiveGAka::SendPacket (void)
{
  static int send_num = 1;
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  //Ptr<MyHeader> header = CreateObject<MyHeader>();
  MyHeader h1;
  
  h1.SetData (10);
// Fill out udpHeader fields appropriately
packet->AddHeader (h1);
  CustomDataTag tag;
    tag.SetNodeId (5) ;
    //tag.SetPosition ( GetNode()->GetObject<MobilityModel>()->GetPosition());
    //timestamp is set in the default constructor of the CustomDataTag class as Simulator::Now()

    //attach the tag to the packet
    packet->AddPacketTag (tag);
uint32_t ab= m_socket->Send (packet);

  MyHeader header;
uint32_t kk=packet->PeekHeader (header);
std::cout<<header.GetData ()<<"  "<<ab<<"  "<<kk<<std::endl;
std::cout<<"Packet send";
  NS_LOG_DEBUG ("Sending:    " << send_num++ << "\t" << Simulator::Now ().GetSeconds ());

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}




void
fiveGAka::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &fiveGAka::SendPacket, this);
    }
}

fiveGAka::fiveGAka ()
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

