#ifndef NEXUS_ORDER_EXECUTION_SESSION_HPP
#define NEXUS_ORDER_EXECUTION_SESSION_HPP
#include <unordered_set>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>

namespace Nexus {

  /** Stores session info for an OrderExecutionServlet client. */
  class OrderExecutionSession : public Beam::AuthenticatedSession {
    public:

      /** Constructs an OrderExecutionSession. */
      OrderExecutionSession() noexcept;

      /**
       * Returns <code>true</code> iff this session is authorized to execute
       * Orders for a specified account.
       * @param account The account to test for authorization.
       * @return <code>true</code> iff this session is authorized to execute
       *         Orders for the specified <i>account</i>.
       */
      bool has_permission(const Beam::DirectoryEntry& account) const;

      /**
       * Grants Order execution permission to an account.
       * @param account The account to grant Order execution permissions to.
       */
      void grant_permission(const Beam::DirectoryEntry& account);

      /**
       * Returns <code>true</code> iff this session belongs to an administrator.
       */
      bool is_administrator() const;

      /**
       * Grants administrative permissions to an account.
       * @param is_administrator Whether this session belongs to an
       *        administrator.
       */
      void set_administrator(bool is_administrator);

      /**
       * Returns <code>true</code> iff this session is subscribed to all
       * execution reports.
       */
      bool is_globally_subscribed() const;

      /** Sets whether this session is subscribed to all execution reports. */
      void set_globally_subscribed(bool is_globally_subscribed);

    private:
      bool m_is_administrator;
      bool m_is_globally_subscribed;
      std::unordered_set<Beam::DirectoryEntry> m_accounts;
  };

  inline OrderExecutionSession::OrderExecutionSession() noexcept
    : m_is_administrator(false),
      m_is_globally_subscribed(false) {}

  inline bool OrderExecutionSession::has_permission(
      const Beam::DirectoryEntry& account) const {
    return m_is_administrator || m_accounts.find(account) != m_accounts.end();
  }

  inline void OrderExecutionSession::grant_permission(
      const Beam::DirectoryEntry& account) {
    m_accounts.insert(account);
  }

  inline bool OrderExecutionSession::is_administrator() const {
    return m_is_administrator;
  }

  inline void OrderExecutionSession::set_administrator(bool is_administrator) {
    m_is_administrator = is_administrator;
  }

  inline bool OrderExecutionSession::is_globally_subscribed() const {
    return m_is_globally_subscribed;
  }

  inline void OrderExecutionSession::set_globally_subscribed(
      bool is_globally_subscribed) {
    m_is_globally_subscribed = is_globally_subscribed;
  }
}

#endif
