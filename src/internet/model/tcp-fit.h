#ifndef TCPFIT_H
#define TCPFIT_H

#include "ns3/tcp-congestion-ops.h"

namespace ns3 {

/**
 * \ingroup congestionOps
 *
 * \brief An implementation of TCP Fit
 *
 */

class TcpFit : public TcpNewReno
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Create an unbound tcp socket.
   */
  TcpFit (void);

  /**
   * \brief Copy constructor
   * \param sock the object to copy
   */
  TcpFit (const TcpFit& sock);
  virtual ~TcpFit (void);

  virtual std::string GetName () const;

  /**
   * \brief Get slow start threshold following Scalable principle (Equation 2)
   *
   * \param tcb internal congestion state
   * \param bytesInFlight bytes in flight
   *
   * \return the slow start threshold value
   */
  virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb,
                                uint32_t bytesInFlight);

  virtual void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t packetsAcked,
                          const Time& rtt);

  virtual Ptr<TcpCongestionOps> Fork ();

  virtual void CongestionStateSet (Ptr<TcpSocketState> tcb, const TcpSocketState::TcpCongState_t newState);

protected:
  /**
   * \brief Congestion avoidance of TcpFit (Equation 1)
   *
   * \param tcb internal congestion state
   * \param segmentsAcked count of segments acked
   */
  virtual void CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);

  virtual void CalculateQ();

private:
  uint32_t               m_cntRtt;
  uint32_t               m_N;
  double                 m_alpha;
  double                 m_Q;
  Time                   m_minRtt;
  double                 m_avgRtt;
};

} // namespace ns3

#endif // TCPFIT_H
