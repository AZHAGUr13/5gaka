
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

    NS_LOG_COMPONENT_DEFINE("MyHeader");

    TypeId
MyHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyHeader")
    .SetParent<Header> ()
    ;
  return tid;
}
TypeId MyHeader::GetInstanceTypeId (void) const
{
  return MyHeader::GetTypeId ();
}
void 
MyHeader::SetData (uint16_t data)
{
  m_data = data;
}
uint16_t
MyHeader::GetData (void)
{
  return m_data;
}
uint32_t 
MyHeader::GetSerializedSize (void) const
{
  // two bytes of data to store
  return 2;
}
void 
MyHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteHtonU16 (m_data);
}
uint32_t 
MyHeader::Deserialize (Buffer::Iterator start)
{
  m_data = start.ReadNtohU16 ();
  return 2;
}
void 
MyHeader::Print (std::ostream &os) const
{
  os << m_data;
}
}
