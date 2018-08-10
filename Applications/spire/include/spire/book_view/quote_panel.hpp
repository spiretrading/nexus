#ifndef SPIRE_QUOTE_PANEL_HPP
#define SPIRE_QUOTE_PANEL_HPP
#include <QLabel>
#include <QWidget>
#include "Nexus/Definitions/Side.hpp"
#include "spire/book_view/book_view.hpp"

namespace Spire {

  //! Displays one side of a BBO quote, with a a colored widget
  //! indicating the increase or decrease of the quote.
  class QuotePanel : public QWidget {
    public:

      //! Constructs a QuotePanel.
      /*
        \param model The model that supplies the BBO quote data.
        \param side Which side of a BBO quote this widget represents.
        \param parent The parent widget.
      */
      QuotePanel(const std::shared_ptr<BookViewModel>& model, Nexus::Side side,
        QWidget* parent = nullptr);

    private:
      std::shared_ptr<BookViewModel> m_model;
      QWidget* m_indicator_widget;
      QLabel* m_price_label;
      QLabel* m_size_label;

      void set_indicator_color(const QColor& color);
  };
}

#endif
