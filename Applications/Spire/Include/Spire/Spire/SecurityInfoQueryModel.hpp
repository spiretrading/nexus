#ifndef SPIRE_SECURITY_INFO_QUERY_MODEL_HPP
#define SPIRE_SECURITY_INFO_QUERY_MODEL_HPP
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/ComboBox.hpp"

namespace Spire {

  /**
   * Extends the interface of a QueryModel specifically for querying
   * SecurityInfos by name, description, or ticker symbol.
   */
  class SecurityInfoQueryModel : public ComboBox::QueryModel {
    public:

      /** Returns the database of markets used to parse ticker symbols. */
      virtual const Nexus::MarketDatabase& get_markets() const = 0;

      virtual Nexus::SecurityInfo parse_security(const QString& query) = 0;

      virtual QtPromise<std::vector<Nexus::SecurityInfo>> submit_security(
        const QString& query) = 0;

    private:
      std::any parse(const QString& query) override final;
      QtPromise<std::vector<std::any>> submit(
        const QString& query) override final;
  };
}

#endif
