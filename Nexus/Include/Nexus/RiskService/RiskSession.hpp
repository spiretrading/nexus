#ifndef NEXUS_RISK_SESSION_HPP
#define NEXUS_RISK_SESSION_HPP
#include <mutex>
#include <unordered_set>
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/thread/mutex.hpp>

namespace Nexus {

  /** Stores session info for a RiskServlet client. */
  class RiskSession : public Beam::ServiceLocator::AuthenticatedSession {
    public:

      /** Constructs a RiskSession. */
      RiskSession() noexcept;

      /** Copies a RiskSession. */
      RiskSession(const RiskSession& session);

      /**
       * Tests if a group's portfolio is being subscribed to by this session.
       * @param group The group to test for the subscription.
       */
      bool has_subscription(
        const Beam::ServiceLocator::DirectoryEntry& group) const;

      /**
       * Adds a group as part of a session's risk portfolio subscription.
       * @param group The group to add.
       */
      void add(const Beam::ServiceLocator::DirectoryEntry& group);

      /** Sets the session as being permissioned to view all portfolios. */
      void add_all();

    private:
      mutable boost::mutex m_mutex;
      bool m_has_all_subscriptions;
      std::unordered_set<Beam::ServiceLocator::DirectoryEntry> m_subscriptions;
  };

  inline RiskSession::RiskSession() noexcept
    : m_has_all_subscriptions(false) {}

  inline RiskSession::RiskSession(const RiskSession& session) {
    auto lock = boost::lock_guard(session.m_mutex);
    m_has_all_subscriptions = session.m_has_all_subscriptions;
    m_subscriptions = session.m_subscriptions;
  }

  inline bool RiskSession::has_subscription(
      const Beam::ServiceLocator::DirectoryEntry& group) const {
    auto lock = std::lock_guard(m_mutex);
    return m_has_all_subscriptions || m_subscriptions.contains(group);
  }

  inline void RiskSession::add(
      const Beam::ServiceLocator::DirectoryEntry& group) {
    auto lock = boost::lock_guard(m_mutex);
    m_subscriptions.insert(group);
  }

  inline void RiskSession::add_all() {
    auto lock = boost::lock_guard(m_mutex);
    m_has_all_subscriptions = true;
  }
}

#endif
