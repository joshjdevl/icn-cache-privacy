/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Tsinghua University, P.R.China
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
 * Author: Xiaoke Jiang <shock.jiang@gmail.com>
 */

#include "ndn-consumer-zipf-prefix-mandelbrot.h"

#include "ns3/ndn-app-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"

#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"

#include <math.h>
#include <boost/lexical_cast.hpp>

NS_LOG_COMPONENT_DEFINE ("ndn.ConsumerZipfPrefixMandelbrot");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (ConsumerZipfPrefixMandelbrot);

TypeId
ConsumerZipfPrefixMandelbrot::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::ConsumerZipfPrefixMandelbrot")
    .SetGroupName ("Ndn")
    .SetParent<ConsumerCbr> ()
    .AddConstructor<ConsumerZipfPrefixMandelbrot> ()

    .AddAttribute ("NumberOfContents", "Number of the Contents in total",
                   StringValue ("100"),
                   MakeUintegerAccessor (&ConsumerZipfPrefixMandelbrot::SetNumberOfContents, &ConsumerZipfPrefixMandelbrot::GetNumberOfContents),
                   MakeUintegerChecker<uint32_t> ())

    .AddAttribute ("q", "parameter of improve rank",
                   StringValue ("0.7"),
                   MakeDoubleAccessor (&ConsumerZipfPrefixMandelbrot::SetQ, &ConsumerZipfPrefixMandelbrot::GetQ),
                   MakeDoubleChecker<double> ())
    
    .AddAttribute ("s", "parameter of power",
                   StringValue ("0.7"),
                   MakeDoubleAccessor (&ConsumerZipfPrefixMandelbrot::SetS, &ConsumerZipfPrefixMandelbrot::GetS),
                   MakeDoubleChecker<double> ())

    .AddAttribute ("producerprefix", "prefix of the producers",
		   StringValue("/producer"),
                   MakeStringAccessor (&ConsumerZipfPrefixMandelbrot::m_producerPrefix),
		   MakeStringChecker ())
    ;

  return tid;
}


ConsumerZipfPrefixMandelbrot::ConsumerZipfPrefixMandelbrot()
  : m_N (100) // needed here to make sure when SetQ/SetS are called, there is a valid value of N
  , m_q (0.7)
  , m_s (0.7)
  , m_SeqRng (0.0, 1.0)
{
  // SetNumberOfContents is called by NS-3 object system during the initialization
}

ConsumerZipfPrefixMandelbrot::~ConsumerZipfPrefixMandelbrot()
{
}

void
ConsumerZipfPrefixMandelbrot::SetNumberOfContents (uint32_t numOfContents)
{
  m_N = numOfContents;

  NS_LOG_DEBUG (m_q << " and " << m_s << " and " << m_N);

  m_Pcum = std::vector<double> (m_N + 1);

  m_Pcum[0] = 0.0;
  for (uint32_t i=1; i<=m_N; i++)
    {
      m_Pcum[i] = m_Pcum[i-1] + 1.0 / std::pow(i+m_q, m_s);
    }

  for (uint32_t i=1; i<=m_N; i++)
    {
      m_Pcum[i] = m_Pcum[i] / m_Pcum[m_N];
      NS_LOG_LOGIC ("Cumulative probability [" << i << "]=" << m_Pcum[i]);
  }
}

uint32_t
ConsumerZipfPrefixMandelbrot::GetNumberOfContents () const
{
  return m_N;
}

void
ConsumerZipfPrefixMandelbrot::SetQ (double q)
{
  m_q = q;
  SetNumberOfContents (m_N);
}

double
ConsumerZipfPrefixMandelbrot::GetQ () const
{
  return m_q;
}

void
ConsumerZipfPrefixMandelbrot::SetS (double s)
{
  m_s = s;
  SetNumberOfContents (m_N);
}

double
ConsumerZipfPrefixMandelbrot::GetS () const
{
  return m_s;
}

void
ConsumerZipfPrefixMandelbrot::SendPacket() {
  if (!m_active) return;

  NS_LOG_FUNCTION_NOARGS ();

  uint32_t seq=std::numeric_limits<uint32_t>::max (); //invalid

  // std::cout << Simulator::Now ().ToDouble (Time::S) << "s max -> " << m_seqMax << "\n";

  while (m_retxSeqs.size ())
    {
      seq = *m_retxSeqs.begin ();
      m_retxSeqs.erase (m_retxSeqs.begin ());

      // NS_ASSERT (m_seqLifetimes.find (seq) != m_seqLifetimes.end ());
      // if (m_seqLifetimes.find (seq)->time <= Simulator::Now ())
      //   {

      //     NS_LOG_DEBUG ("Expire " << seq);
      //     m_seqLifetimes.erase (seq); // lifetime expired. Trying to find another unexpired sequence number
      //     continue;
      //   }
      NS_LOG_DEBUG("=interest seq "<<seq<<" from m_retxSeqs");
      break;
    }

  if (seq == std::numeric_limits<uint32_t>::max ()) //no retransmission
    {
      if (m_seqMax != std::numeric_limits<uint32_t>::max ())
        {
          if (m_seq >= m_seqMax)
            {
              return; // we are totally done
            }
        }

      seq = ConsumerZipfPrefixMandelbrot::GetNextSeq();
      m_seq ++;
    }

  // std::cout << Simulator::Now ().ToDouble (Time::S) << "s -> " << seq << "\n";

  //
  NS_LOG_INFO("seq="<<seq);
  Ptr<Name> nameWithSequence = Create<Name> (m_producerPrefix+boost::lexical_cast<std::string>(seq));
  //nameWithSequence->appendSeqNum (seq);
  //

  Ptr<Interest> interest = Create<Interest> ();
  interest->SetNonce (m_rand.GetValue ());
  interest->SetName  (nameWithSequence);

  // NS_LOG_INFO ("Requesting Interest: \n" << *interest);
  NS_LOG_INFO ("> Interest for " << seq<<", Total: "<<m_seq<<", face: "<<m_face->GetId());
  NS_LOG_DEBUG ("Trying to add " << seq << " with " << Simulator::Now () << ". already " << m_seqTimeouts.size () << " items");

  m_seqTimeouts.insert (SeqTimeout (seq, Simulator::Now ()));
  m_seqFullDelay.insert (SeqTimeout (seq, Simulator::Now ()));

  m_seqLastDelay.erase (seq);
  m_seqLastDelay.insert (SeqTimeout (seq, Simulator::Now ()));

  m_seqRetxCounts[seq] ++;

  m_rtt->SentSeq (SequenceNumber32 (seq), 1);

  FwHopCountTag hopCountTag;
  interest->GetPayload ()->AddPacketTag (hopCountTag);

  m_transmittedInterests (interest, this, m_face);
  m_face->ReceiveInterest (interest);

  ConsumerZipfPrefixMandelbrot::ScheduleNextPacket ();
}


uint32_t
ConsumerZipfPrefixMandelbrot::GetNextSeq()
{
  uint32_t content_index = 1; //[1, m_N]
  double p_sum = 0;

  double p_random = m_SeqRng.GetValue();
  while (p_random == 0)
    {
      p_random = m_SeqRng.GetValue();
    }
  //if (p_random == 0)
  NS_LOG_LOGIC("p_random="<<p_random);
  for (uint32_t i=1; i<=m_N; i++)
    {
      p_sum = m_Pcum[i];   //m_Pcum[i] = m_Pcum[i-1] + p[i], p[0] = 0;   e.g.: p_cum[1] = p[1], p_cum[2] = p[1] + p[2]
      if (p_random <= p_sum)
        {
          content_index = i;
          break;
        } //if
    } //for
  //content_index = 1;
  NS_LOG_DEBUG("RandomNumber="<<content_index);
  return content_index;
}

void
ConsumerZipfPrefixMandelbrot::ScheduleNextPacket() {

  if (m_firstTime)
    {
      m_sendEvent = Simulator::Schedule (Seconds (0.0),
                                         &ConsumerZipfPrefixMandelbrot::SendPacket, this);
      m_firstTime = false;
    }
  else if (!m_sendEvent.IsRunning ())
    m_sendEvent = Simulator::Schedule (
                                       (m_random == 0) ?
                                       Seconds(1.0 / m_frequency)
                                       :
                                       Seconds(m_random->GetValue ()),
                                       &ConsumerZipfPrefixMandelbrot::SendPacket, this);
}

} /* namespace ndn */
} /* namespace ns3 */

