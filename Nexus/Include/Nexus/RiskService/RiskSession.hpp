#ifndef NEXUS_RISK_SESSION_HPP
#define NEXUS_RISK_SESSION_HPP
#include <unordered_set>
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/thread/mutex.hpp>
#include "Nexus/RiskService/RiskService.hpp"

namespace Nexus::RiskService {

  /** Stores session info for a RiskServlet client. */
  class RiskSession : public Beam::ServiceLocator::AuthenticatedSession {
    public:

      /** Constructs a RiskSession. */
      RiskSession();

      /** Copies a RiskSession. */
      RiskSession(const RiskSession& session);

      /**
       * Tests if a group's portfolio is being subscribed to by this session.
       * @param group The group to test for the subscription.
       */
      bool HasGroupPortfolioSubscription(
        const Beam::ServiceLocator::DirectoryEntry& group) const;

      /**
       * Adds a group as part of a session's risk portfolio subscription.
       * @param group The group to add.
       */
      void AddPortfolioGroup(const Beam::ServiceLocator::DirectoryEntry& group);

      /** Sets the session as being permissioned to view all portfolios. */
      void AddAllPortfolioGroups();

    private:
      mutable boost::mutex m_mutex;
      bool m_isViewingAllPortfolioGroups;
      std::unordered_set<Beam::ServiceLocator::DirectoryEntry>
        m_portfolioGroups;
  };

  inline RiskSession::RiskSession()
    : m_isViewingAllPortfolioGroups(false) {}

  inline RiskSession::RiskSession(const RiskSession& session) {
    auto lock = boost::lock_guard(session.m_mutex);
    m_isViewingAllPortfolioGroups = session.m_isViewingAllPortfolioGroups;
    m_portfolioGroups = session.m_portfolioGroups;
  }

  inline bool RiskSession::HasGroupPortfolioSubscription(
      const Beam::ServiceLocator::DirectoryEntry& group) const {
    auto lock = boost::lock_guard(m_mutex);
    if(m_isViewingAllPortfolioGroups) {
      return true;
    }
    return m_portfolioGroups.find(group) != m_portfolioGroups.end();
  }

  inline void RiskSession::AddPortfolioGroup(
      const Beam::ServiceLocator::DirectoryEntry& group) {
    auto lock = boost::lock_guard(m_mutex);
    m_portfolioGroups.insert(group);
  }

  inline void RiskSession::AddAllPortfolioGroups() {
    auto lock = boost::lock_guard(m_mutex);
    m_isViewingAllPortfolioGroups = true;
  }
}

#endif
