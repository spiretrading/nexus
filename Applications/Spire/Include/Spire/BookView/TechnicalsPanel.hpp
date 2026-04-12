#ifndef SPIRE_TECHNICALS_PANEL_HPP
#define SPIRE_TECHNICALS_PANEL_HPP
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/Ui/QuantityBox.hpp"

namespace Spire {
  class AdaptiveBox;

  /** Displays the technical details on the adaptive panel. */
  class TechnicalsPanel : public QWidget {
    public:

      /**
       * Constructs a TechnicalsPanel.
       * @param candlestick The session candlestick to display.
       * @param default_bid_quantity The default bid quantity to display.
       * @param default_bid_quantity The default ask quantity to display.
       * @param parent The parent widget.
       */
      TechnicalsPanel(std::shared_ptr<SessionCandlestickModel> candlestick,
        std::shared_ptr<QuantityModel> default_bid_quantity,
        std::shared_ptr<QuantityModel> default_ask_quantity,
        QWidget* parent = nullptr);

      /** Returns the session candlestick value model. */
      const std::shared_ptr<SessionCandlestickModel>&
        get_session_candlestick() const;

      /** Returns the default bid quantity value model. */
      const std::shared_ptr<QuantityModel>& get_default_bid_quantity() const;

      /** Returns the default ask quantity value model. */
      const std::shared_ptr<QuantityModel>& get_default_ask_quantity() const;

      QSize minimumSizeHint() const override;

    private:
      std::shared_ptr<SessionCandlestickModel> m_candlestick;
      std::shared_ptr<QuantityModel> m_bid_quantity;
      std::shared_ptr<QuantityModel> m_ask_quantity;
      AdaptiveBox* m_adaptive_box;
      TextBox* m_default_field;
      QLayout* m_extra_small_layout;
      BoxGeometry m_geometry;
      mutable boost::optional<QSize> m_minimum_size_hint;
      boost::signals2::scoped_connection m_bid_quantity_connection;
      boost::signals2::scoped_connection m_ask_quantity_connection;
      boost::signals2::scoped_connection m_style_connection;

      void on_bid_quantity_update(Nexus::Quantity quantity);
      void on_ask_quantity_update(Nexus::Quantity quantity);
      void on_style();
  };
}

#endif
