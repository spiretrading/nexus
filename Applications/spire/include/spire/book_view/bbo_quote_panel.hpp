#ifndef SPIRE_BBO_QUOTE_PANEL_HPP
#define SPIRE_BBO_QUOTE_PANEL_HPP
#include <QWidget>
#include "Nexus/Definitions/Side.hpp"
#include "spire/book_view/book_view.hpp"

namespace Spire {

  //! Displays both sides of a BBO quote.
  class BboQuotePanel : public QWidget {
    public:

      //! Constructs a BboQuotePanel.
      /*
        \param model The model that supplies the BBO quote data.
        \param parent The parent widget.
      */
      explicit BboQuotePanel(const BookViewModel& model,
        QWidget* parent = nullptr);

      //! Sets the model that supplies the data to this widget.
      void set_model(const BookViewModel& model);

    private:
      QuotePanel* m_bid_quote_panel;
      QuotePanel* m_ask_quote_panel;
  };
}

#endif
