#ifndef SPIRE_QUOTE_PANEL_HPP
#define SPIRE_QUOTE_PANEL_HPP
#include <QLabel>
#include <QWidget>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/Spire/Signal.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  //! Displays one side of a BBO quote, with a a colored widget
  //! indicating the increase or decrease of the quote.
  class QuotePanel : public QWidget {
    public:

      //! Constructs a QuotePanel.
      /*!
        \param model The model that supplies the BBO quote data.
        \param side Which side of a BBO quote this widget represents.
        \param parent The parent widget.
      */
      QuotePanel(const BookViewModel& model, Nexus::Side side,
        QWidget* parent = nullptr);

      //! Sets the model that supplies the data to this widget.
      void set_model(const BookViewModel& model);

    private:
      Nexus::Side m_side;
      Nexus::BboQuote m_current_bbo;
      QuotePanelIndicatorWidget* m_indicator_widget;
      QLabel* m_price_label;
      QLabel* m_size_label;
      boost::signals2::scoped_connection m_bbo_connection;
      CustomVariantItemDelegate* m_item_delegate;

      void set_quote_text(const Nexus::Money& price,
        const Nexus::Quantity& size);
      void on_bbo_quote(const Nexus::BboQuote& bbo);
  };
}

#endif
