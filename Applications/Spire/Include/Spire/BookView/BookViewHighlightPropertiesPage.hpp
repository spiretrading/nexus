#ifndef SPIRE_BOOK_VIEW_HIGHLIGHT_PROPERTIES_PAGE_HPP
#define SPIRE_BOOK_VIEW_HIGHLIGHT_PROPERTIES_PAGE_HPP
#include <QWidget>
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {

  /** A ValueModel over a BookViewHighlightProperties. */
  using HighlightPropertiesModel = ValueModel<BookViewHighlightProperties>;

  /** A LocalValueModel over a BookViewHighlightProperties. */
  using LocalHighlightPropertiesModel =
    LocalValueModel<BookViewHighlightProperties>;

  /** Implements a widget for the highlight property settings. */
  class BookViewHighlightPropertiesPage : public QWidget {
    public:

      /**
       * Constructs a BookViewHighlightPropertiesPage.
       * @param current The initial current properties.
       * @param markets The market database to use.
       * @param parent The parent widget.
       */
      BookViewHighlightPropertiesPage(
        std::shared_ptr<HighlightPropertiesModel> current,
        Nexus::MarketDatabase markets, QWidget* parent = nullptr);

      /** Returns the current properties. */
      const std::shared_ptr<HighlightPropertiesModel>& get_current() const;

    private:
      std::shared_ptr<HighlightPropertiesModel> m_current;
      std::array<HighlightBox*,
        BookViewHighlightProperties::ORDER_HIGHLIGHT_STATE_COUNT>
          m_order_highlight_boxes;

      void on_visibility_update(
        BookViewHighlightProperties::OrderVisibility visibility);
  };
}

#endif
