#ifndef SPIRE_LOCAL_SECURITY_QUERY_MODEL_HPP
#define SPIRE_LOCAL_SECURITY_QUERY_MODEL_HPP
#include <Beam/Collections/Trie.hpp>
#include "Spire/Spire/SecurityInfoQueryModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Implements an in-memory SecurityInfoQueryModel that will match against
   * either the ticker symbol or the full name.
   */
  class LocalSecurityInfoQueryModel : public SecurityInfoQueryModel {
    public:

      /**
       * Constructs an empty model.
       * @param markets The database of markets used to parse securities.
       */
      explicit LocalSecurityInfoQueryModel(Nexus::MarketDatabase markets);

      /**
       * Adds a security to the model.
       * @param security The security to add.
       */
      void add(const Nexus::SecurityInfo& security);

      const Nexus::MarketDatabase& get_markets() const;

      Nexus::SecurityInfo parse_security(const QString& query) override;

      QtPromise<std::vector<Nexus::SecurityInfo>>
        submit_security(const QString& query) override;

    private:
      Nexus::MarketDatabase m_markets;
      rtv::Trie<QChar, Nexus::SecurityInfo> m_values;
  };
}

#endif
