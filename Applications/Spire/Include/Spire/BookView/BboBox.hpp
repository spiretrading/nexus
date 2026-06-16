#ifndef SPIRE_BBO_BOX_HPP
#define SPIRE_BBO_BOX_HPP
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QWidget>
#include "Nexus/Definitions/Quote.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {
  class TextBox;

  /** A ValueModel over a Quote. */
  using QuoteModel = ValueModel<Nexus::Quote>;

  /** A LocalValueModel over a Quote. */
  using LocalQuoteModel = LocalValueModel<Nexus::Quote>;

  /** Displays the BBO quote. */
  class BboBox : public QWidget {
    public:

      /**
       * Constructs a BboBox.
       * @param quote The BBO quote to display.
       * @param parent The parent widget.
       */
      explicit BboBox(
        std::shared_ptr<QuoteModel> quote, QWidget* parent = nullptr);

      /** Returns the quote value model. */
      const std::shared_ptr<QuoteModel>& get_quote() const;

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      std::shared_ptr<QuoteModel> m_quote;
      Nexus::Money m_previous_price;
      TextBox* m_money_label;
      QHBoxLayout* m_body_layout;
      QSpacerItem* m_gap1;
      QSpacerItem* m_gap2;
      int m_font_size;
      boost::signals2::scoped_connection m_quote_connection;
      boost::signals2::scoped_connection m_style_connection;

      void update_gap_width();
      void on_quote(const Nexus::Quote& quote);
      void on_style();
  };
}

#endif
