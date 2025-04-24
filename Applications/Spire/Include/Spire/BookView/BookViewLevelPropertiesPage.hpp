#ifndef SPIRE_BOOK_VIEW_LEVEL_PROPERTIES_PAGE_HPP
#define SPIRE_BOOK_VIEW_LEVEL_PROPERTIES_PAGE_HPP
#include <QWidget>
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {

  /** A ValueModel over a BookViewLevelProperties. */
  using LevelPropertiesModel = ValueModel<BookViewLevelProperties>;

  /** A LocalValueModel over a BookViewLevelProperties. */
  using LocalLevelPropertiesModel = LocalValueModel<BookViewLevelProperties>;

  /** Implements a widget for the price level property settings. */
  class BookViewLevelPropertiesPage : public QWidget {
    public:

      /**
       * Constructs a BookViewLevelPropertiesPage.
       * @param current The initial current properties.
       * @param parent The parent widget.
       */
      explicit BookViewLevelPropertiesPage(
        std::shared_ptr<LevelPropertiesModel> current,
        QWidget* parent = nullptr);

      /** Returns the current properties. */
      const std::shared_ptr<LevelPropertiesModel>& get_current() const;

    private:
      struct PriceLevelWidget;
      std::shared_ptr<LevelPropertiesModel> m_current;
      PriceLevelWidget* m_price_level_widget;
      QFont m_font;
      boost::signals2::scoped_connection m_font_connection;

      void on_font(const QFont& font);
  };
}

#endif
