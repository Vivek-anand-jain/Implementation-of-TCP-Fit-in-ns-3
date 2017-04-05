#include "tcp-fit.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpFit");
NS_OBJECT_ENSURE_REGISTERED (TcpFit);

TypeId
TcpFit::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpFit")
    .SetParent<TcpNewReno> ()
    .AddConstructor<TcpFit> ()
    .SetGroupName ("Internet")
    .AddAttribute ("Alpha", "Alpha in TCP Fit",
                   DoubleValue (0.125),
                   MakeDoubleAccessor (&TcpFit::m_alpha),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

TcpFit::TcpFit (void)
  : TcpNewReno (),
    m_cntRtt(0),
    m_N(1),
    m_alpha(0.125),
    m_Q (0),
    m_minRtt (0),
    m_avgRtt (0)
{
  NS_LOG_FUNCTION (this);
}

TcpFit::TcpFit (const TcpFit& sock)
  : TcpNewReno (sock),
    m_cntRtt(sock.m_cntRtt),
    m_N(sock.m_N),
    m_alpha(sock.m_alpha),
    m_Q (sock.m_Q),
    m_minRtt (sock.m_minRtt),
    m_avgRtt (sock.m_avgRtt)
{
  NS_LOG_FUNCTION (this);
}

TcpFit::~TcpFit (void)
{
  NS_LOG_FUNCTION (this);
}

Ptr<TcpCongestionOps>
TcpFit::Fork (void)
{
  return CopyObject<TcpFit> (this);
}


void
TcpFit::CongestionStateSet (Ptr<TcpSocketState> tcb,
                              const TcpSocketState::TcpCongState_t newState)
{
  NS_LOG_FUNCTION (this << tcb << newState);
  if (newState == TcpSocketState::CA_LOSS || newState == TcpSocketState::CA_RECOVERY)
    {
      if (m_Q < m_alpha / m_N)
        {
          m_N = m_N + 1;
        }
      else if (m_Q > m_alpha / m_N)
        {
          int oldN = m_N - 1;
          m_N = std::max(1, oldN);
        }
    }
}

void
TcpFit::CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);
  if (segmentsAcked > 0)
    {
      double adder = static_cast<double> (tcb->m_segmentSize * tcb->m_segmentSize) / tcb->m_cWnd.Get () * m_N;
      adder = std::max (1.0, adder);
      tcb->m_cWnd += static_cast<uint32_t> (adder);
    }
}

void
TcpFit::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t packetsAcked,
                        const Time& rtt)
{
  NS_LOG_FUNCTION (this << tcb << packetsAcked << rtt);

  if (rtt.IsZero ())
    {
      NS_LOG_WARN ("RTT measured is zero!");
      return;
    }

  if (m_minRtt.IsZero ())
    {
      m_minRtt = rtt;
    }
  else
    {
      if (rtt < m_minRtt)
        {
          m_minRtt = rtt;
        }
    }

  m_avgRtt = m_avgRtt * m_cntRtt;
  m_cntRtt++;
  double temp = rtt.GetMilliSeconds () / 1000.0;
  m_avgRtt = (temp + m_avgRtt) * 1.0 / m_cntRtt;

  CalculateQ ();

}

void
TcpFit::CalculateQ ()
{
  NS_LOG_FUNCTION (this);
  double temp = m_minRtt.GetMilliSeconds () / 1000.0;
  m_Q = (m_avgRtt - temp) * 1.0 / m_avgRtt;
}

std::string
TcpFit::GetName () const
{
  return "TcpFit";
}

uint32_t
TcpFit::GetSsThresh (Ptr<const TcpSocketState> tcb,
                          uint32_t bytesInFlight)
{
  NS_LOG_FUNCTION (this << tcb << bytesInFlight);

  uint32_t segCwnd = bytesInFlight / tcb->m_segmentSize;

  double b = 1.0 - 2 / (3 * m_N + 1);
  uint32_t ssThresh = std::max (2.0, segCwnd * b);

  NS_LOG_DEBUG ("Calculated b(w) = " << b << " resulting (in segment) ssThresh=" << ssThresh);

  return ssThresh * tcb->m_segmentSize;
}

} // namespace ns3
