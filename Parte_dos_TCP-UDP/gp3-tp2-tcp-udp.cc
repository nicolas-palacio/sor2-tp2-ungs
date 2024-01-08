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
#include <string>
#include <fstream>
#include <stddef.h>
#include <iomanip>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/packet-sink.h"

 
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SOR2-Grupo 3-TCP/UDP");
static bool firstCwnd = true;
static bool firstSshThr = true;
static Ptr<OutputStreamWrapper> cWndStream;
static Ptr<OutputStreamWrapper> ssThreshStream;
static uint32_t cWndValue;
static uint32_t ssThreshValue;


static void
CwndTracer (uint32_t oldval, uint32_t newval)
{
  if (firstCwnd)
    {
      *cWndStream->GetStream () << "0.0 " << oldval << std::endl;
      firstCwnd = false;
    }
  *cWndStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
  cWndValue = newval;

  if (!firstSshThr)
    {
      *ssThreshStream->GetStream () << Simulator::Now ().GetSeconds () << " " << ssThreshValue << std::endl;
    }
}

static void
SsThreshTracer (uint32_t oldval, uint32_t newval)
{
  if (firstSshThr)
    {
      *ssThreshStream->GetStream () << "0.0 " << oldval << std::endl;
      firstSshThr = false;
    }
  *ssThreshStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
  ssThreshValue = newval;

  if (!firstCwnd)
    {
      *cWndStream->GetStream () << Simulator::Now ().GetSeconds () << " " << cWndValue << std::endl;
    }
}

// //NodeList/Dispositivo/$ns3::TcpL4Protocol/SocketList/Interfaz/CongestionWindow
static void
TraceCwnd (std::string cwnd_tr_file_name)
{
  AsciiTraceHelper ascii;
  cWndStream = ascii.CreateFileStream (cwnd_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&CwndTracer));
}

static void
TraceSsThresh (std::string ssthresh_tr_file_name)
{
  AsciiTraceHelper ascii;
  ssThreshStream = ascii.CreateFileStream (ssthresh_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/SlowStartThreshold", MakeCallback (&SsThreshTracer));
}


int main (int argc, char *argv[])
{
  std::string prefix_file_name="gp3-tp2";
  std::string cwndName="cwnd";
  uint16_t port=400;
  bool tracing=true;
  uint64_t data_bytes=400;

  //Define que el algoritmo a usar es TCP New Reno;
  Config::SetDefault("ns3::TcpL4Protocol::SocketType",TypeIdValue(TcpNewReno::GetTypeId()));
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
   

  //Genero la conexion punto a punto 
  PointToPointHelper bottleneck;
  bottleneck.SetDeviceAttribute ("DataRate", StringValue ("50KBps")); 
  bottleneck.SetChannelAttribute ("Delay", StringValue ("100ms"));
  bottleneck.SetQueue("ns3::DropTailQueue","MaxSize", StringValue ("10p"));

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("50MBps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("10ms"));
  pointToPoint.SetQueue("ns3::DropTailQueue","MaxSize", StringValue ("10p"));

  PointToPointDumbbellHelper dumbbellNetwork(3,pointToPoint,3,pointToPoint,bottleneck);

  //Se instala la pila de protocolos internet
  InternetStackHelper stack;
  dumbbellNetwork.InstallStack(stack);

  //Defino las IPs bases para los nodos
  Ipv4AddressHelper leftIP("10.1.1.0", "255.255.255.0");
  Ipv4AddressHelper rightIP("10.2.1.0", "255.255.255.0");
  Ipv4AddressHelper routersIP("10.3.1.0", "255.255.255.0");

  dumbbellNetwork.AssignIpv4Addresses(leftIP,rightIP,routersIP);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  uint32_t nodo0=0;
  uint32_t nodo1=1;
  uint32_t nodo2=2;

//Se crean las aplicaciones OnOff TCP
//-------------------------------------------
  OnOffHelper clientHelper("ns3::TcpSocketFactory",InetSocketAddress (dumbbellNetwork.GetRightIpv4Address (nodo1),port));
  clientHelper.SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1000.0]"));
  clientHelper.SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper.SetAttribute("MaxBytes", UintegerValue(data_bytes*10000));
 
  clientApps.Add(clientHelper.Install(dumbbellNetwork.GetLeft(nodo0)));

  PacketSinkHelper server("ns3::TcpSocketFactory", InetSocketAddress(dumbbellNetwork.GetRightIpv4Address(nodo1),port));
  serverApps.Add(server.Install(dumbbellNetwork.GetRight(nodo1)));

//-------------------------------------------
  OnOffHelper clientHelper2("ns3::TcpSocketFactory",InetSocketAddress (dumbbellNetwork.GetRightIpv4Address (nodo2),port));
  clientHelper2.SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1000.0]"));
  clientHelper2.SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper2.SetAttribute("MaxBytes", UintegerValue(data_bytes*10000));

  clientApps.Add(clientHelper2.Install(dumbbellNetwork.GetLeft(1)));

  PacketSinkHelper server2("ns3::TcpSocketFactory", InetSocketAddress(dumbbellNetwork.GetRightIpv4Address(nodo2),port));
  serverApps.Add(server2.Install(dumbbellNetwork.GetRight(nodo2)));

//Se crea la aplicacion OnOff UDP
//-------------------------------------------
  OnOffHelper clientHelperUDP("ns3::UdpSocketFactory",InetSocketAddress (dumbbellNetwork.GetRightIpv4Address (nodo0),port));
  clientHelperUDP.SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1000.0]"));
  clientHelperUDP.SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelperUDP.SetAttribute("MaxBytes", UintegerValue(data_bytes*10000));

  clientApps.Add(clientHelperUDP.Install(dumbbellNetwork.GetLeft(2)));

  PacketSinkHelper serverUDP("ns3::TcpSocketFactory", InetSocketAddress(dumbbellNetwork.GetRightIpv4Address(nodo0),port));
  serverApps.Add(serverUDP.Install(dumbbellNetwork.GetRight(nodo0)));

//-------------------------------------------
  serverApps.Start (Seconds (0.0));
  serverApps.Stop (Seconds (100.0));
  clientApps.Start (Seconds (1.0));
  clientApps.Stop (Seconds (30.0));

  if(tracing){
    pointToPoint.EnablePcapAll ("gp3-tp2-tcp-udp",true);

     std::ofstream ascii;
     Ptr<OutputStreamWrapper> ascii_wrap;
      ascii.open ((prefix_file_name + "-ascii").c_str ());
      ascii_wrap = new OutputStreamWrapper ((prefix_file_name + "-ascii").c_str (),std::ios::out);
      stack.EnableAsciiIpv4All (ascii_wrap);

      //Definimos que el tracing comience a partir del segundo 2 ya que, si comenzaba antes, todos los .data se generaban vacios;
      Simulator::Schedule (Seconds (2.000001), &TraceCwnd, cwndName + "-cwnd.data");
      Simulator::Schedule (Seconds (2.000001), &TraceSsThresh, prefix_file_name + "-ssth.data");
  }

  NS_LOG_INFO("Simulation started.");
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
