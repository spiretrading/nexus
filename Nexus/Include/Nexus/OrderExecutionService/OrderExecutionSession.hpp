#ifndef NEXUS_ORDER_EXECUTION_SESSION_HPP
#define NEXUS_ORDER_EXECUTION_SESSION_HPP
#include <unordered_set>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

  /** Stores session info for an OrderExecutionServlet client. */
  class OrderExecutionSession
      : public Beam::ServiceLocator::AuthenticatedSession {
    public:

      /** Constructs an OrderExecutionSession. */
      OrderExecutionSession();

      /**
       * Returns <code>true</code> iff this session is authorized to execute
       * Orders for a specified account.
       * @param account The account to test for authorization.
       * @return <code>true</code> iff this session is authorized to execute
       *         Orders for the specified <i>account</i>.
       */
      bool HasOrderExecutionPermission(
        const Beam::ServiceLocator::DirectoryEntry& account) const;

      /**
       * Grants Order execution permission to an account.
       * @param account The account to grant Order execution permissions to.
       */
      void GrantOrderExecutionPermission(
        const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Returns <code>true</code> iff this session belongs to an administrator.
       */
      bool IsAdministrator() const;

      /**
       * Grants administrative permissions to an account.
       * @param value Whether this session belongs to an administrator.
       */
      void SetAdministrator(bool value);

      /**
       * Returns <code>true</code> iff this session is subscribed to all
       * execution reports.
       */
      bool IsSubscribedToAllExecutionReports() const;

      /** Sets whether this session is subscribed to all execution reports. */
      void SetSubscribedToAllExecutionReports(bool value);

    private:
      bool m_isAdministrator;
      std::unordered_set<Beam::ServiceLocator::DirectoryEntry>
        m_orderExecutionAccounts;
      bool m_isSubscribedToAllExecutionReports;
  };

  inline OrderExecutionSession::OrderExecutionSession()
    : m_isAdministrator(false),
      m_isSubscribedToAllExecutionReports(false) {}

  inline bool OrderExecutionSession::HasOrderExecutionPermission(
      const Beam::ServiceLocator::DirectoryEntry& account) const {
    return m_isAdministrator || m_orderExecutionAccounts.find(account) !=
      m_orderExecutionAccounts.end();
  }

  inline void OrderExecutionSession::GrantOrderExecutionPermission(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    m_orderExecutionAccounts.insert(account);
  }

  inline bool OrderExecutionSession::IsAdministrator() const {
    return m_isAdministrator;
  }

  inline void OrderExecutionSession::SetAdministrator(bool value) {
    m_isAdministrator = value;
  }

  inline bool OrderExecutionSession::IsSubscribedToAllExecutionReports() const {
    return m_isSubscribedToAllExecutionReports;
  }

  inline void OrderExecutionSession::SetSubscribedToAllExecutionReports(
      bool value) {
    m_isSubscribedToAllExecutionReports = value;
  }
}

#endif
