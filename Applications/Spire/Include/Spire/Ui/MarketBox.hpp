#ifndef SPIRE_MARKET_BOX_HPP
#define SPIRE_MARKET_BOX_HPP
#include "Nexus/Definitions/Market.hpp"
#include "Spire/Ui/EnumBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Defines a model over a Market. */
  using MarketModel = ValueModel<Nexus::MarketCode>;

  /** Defines a local model for a Market. */
  using LocalMarketModel = LocalValueModel<Nexus::MarketCode>;

  /** An EnumBox specialized for a Nexus::Market. */
  using MarketBox = EnumBox<Nexus::MarketCode>;

  /**
   * Returns a new MarketBox using a LocalMarketModel.
   * @param parent The parent widget.
   */
  MarketBox* make_market_box(QWidget* parent = nullptr);

  /**
   * Returns a new MarketBox using a LocalMarketModel and initial current value.
   * @param current The initial current value.
   * @param parent The parent widget.
   */
  MarketBox* make_market_box(
    Nexus::MarketCode current, QWidget* parent = nullptr);

  /**
   * Returns a MarketBox.
   * @param current The current value.
   * @param parent The parent widget.
   */
   MarketBox* make_market_box(
    std::shared_ptr<MarketModel> current, QWidget* parent = nullptr);
}

#endif
