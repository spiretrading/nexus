#ifndef SPIRE_CURRENCY_ENTRY_TO_CURRENCY_QUERY_MODEL_HPP
#define SPIRE_CURRENCY_ENTRY_TO_CURRENCY_QUERY_MODEL_HPP
#include "Nexus/Definitions/Currency.hpp"
#include "Spire/Spire/QueryModel.hpp"

namespace Spire {

  /** Represents a QueryModel for CurrencyDatabase entries. */
  using CurrencyEntryQueryModel = QueryModel<Nexus::CurrencyDatabase::Entry>;

  /**
   * Implements a QueryModel over CurrencyIds by adapting a
   * QueryModel<CurrencyDatabase::Entry>.
   */
  class CurrencyEntryToCurrencyQueryModel :
      public QueryModel<Nexus::CurrencyId> {
    public:

      /**
       * Constructs a CurrencyEntryToCurrencyQueryModel.
       * @param source The source QueryModel providing CurrencyDatabase entries.
       */
      explicit CurrencyEntryToCurrencyQueryModel(
        std::shared_ptr<CurrencyEntryQueryModel> source);

      /** Returns the source model. */
      const std::shared_ptr<CurrencyEntryQueryModel>& get_source() const;

      boost::optional<Nexus::CurrencyId> parse(const QString& query) override;
      QtPromise<std::vector<Nexus::CurrencyId>>
        submit(const QString& query) override;

    private:
      std::shared_ptr<CurrencyEntryQueryModel> m_source;
  };
}

#endif
