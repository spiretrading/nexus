#ifndef SPIRE_SERVICE_ACCOUNT_QUERY_MODEL_HPP
#define SPIRE_SERVICE_ACCOUNT_QUERY_MODEL_HPP
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Spire/Spire/QueryModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/AccountListItem.hpp"

namespace Spire {

  /**
   * Implements a QueryModel by submitting queries for accounts to an
   * AdministrationClient.
   */
  class ServiceAccountQueryModel : public QueryModel<AccountListItem::Account> {
    public:

      /**
       * Constructs a ServiceAccountQueryModel.
       * @param administration_client The AdministrationClient to submit queries
       *        to.
       */
      explicit ServiceAccountQueryModel(
        Nexus::AdministrationClient administration_client);

      boost::optional<AccountListItem::Account> parse(
        const QString& query) override;
      QtPromise<std::vector<AccountListItem::Account>> submit(
        const QString& query) override;

    private:
      Nexus::AdministrationClient m_administration_client;
  };
}

#endif
