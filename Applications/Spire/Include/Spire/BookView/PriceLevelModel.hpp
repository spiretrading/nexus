#ifndef SPIRE_PRICE_LEVEL_MODEL_HPP
#define SPIRE_PRICE_LEVEL_MODEL_HPP
#include "Nexus/Definitions/Money.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /**
   * Transforms a list of prices into a list of levels corresponding to the
   * price level.
   */
  class PriceLevelModel : public ListModel<int> {
    public:

      /** The type of list model used for prices. */
      using PriceListModel = ListModel<Nexus::Money>;

      /** The type of model used to represent the max level. */
      using MaxLevelModel = ValueModel<int>;

      /**
       * Constructs the PriceLevelModel by transforming the given list of
       * prices into a list of levels.
       * @param prices The list of prices to transform.
       * @param max_level The maximum level to assign to a price.
       */
      PriceLevelModel(std::shared_ptr<PriceListModel> prices,
        std::shared_ptr<MaxLevelModel> max_level);

      int get_size() const override;

      const Type& get(int index) const;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      std::shared_ptr<PriceListModel> m_prices;
      std::shared_ptr<MaxLevelModel> m_max_level;
      ArrayListModel<int> m_levels;
      boost::signals2::scoped_connection m_price_connection;
      boost::signals2::scoped_connection m_max_level_connection;

      void on_price_operation(const PriceListModel::Operation& operation);
      void on_max_level(int max_level);
  };
}

#endif
