/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
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
 *
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */
// ndn-simple.cc
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

using namespace ns3;

/**
 * This scenario simulates a very simple network topology:
 *
 *
 *      +----------+     1Mbps      +--------+     1Mbps      +----------+
 *      | consumer | <------------> | router | <------------> | producer |
 *      +----------+         10ms   +--------+          10ms  +----------+
 *
 *
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-simple
 */

int 
main (int argc, char *argv[])
{
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("1Mbps"));
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("10ms"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("20"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);

  AnnotatedTopologyReader topologyReader ("", 25);
  topologyReader.SetFileName ("topology.txt");
  topologyReader.Read ();


  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes (true);
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  ndnHelper.SetContentStore ("ns3::ndn::cs::Lru",
                              "MaxSize", "10000");
  ndnHelper.InstallAll ();

  // Installing applications

  // Consumer
  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfPrefixMandelbrot");
  // Consumer will request /prefix0, /prefix1, ...
  consumerHelper.SetPrefix ("/producer");
  consumerHelper.SetAttribute ("Frequency", StringValue ("10")); // 10 interests a second
  consumerHelper.SetAttribute("Randomize",    StringValue ("uniform"));

NodeContainer consumerNodes;
int nodeIndex = 1;
std::string consumernodeNamePrefix("consumer");
Ptr<Node> consumerNode = Names::Find<Node>(consumernodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
while(consumerNode != NULL)
{
  consumerNodes.Add (consumerNode);
  consumerNode = Names::Find<Node>(consumernodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
}
consumerHelper.Install(consumerNodes);



int nodeIndex = 1;
std::string producernodeNamePrefix("producer");
Ptr<Node> producerNode = Names::Find<Node>(producernodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
while(producerNode != NULL)
{
  NodeContainer producerNodes;
  producerNodes.Add (producerNode);

  // Producer
  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  // Producer will reply to all requests starting with /prefix
  producerHelper.SetPrefix ("/producer"+  boost::lexical_cast<std::string>(nodeIndex));
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  producerHelper.Install(producerNodes);

  producerNode = Names::Find<Node>(producernodeNamePrefix +  boost::lexical_cast<std::string>(nodeIndex++));
}




  Simulator::Stop (Seconds (20.0));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
