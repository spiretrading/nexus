#ifndef SPIRE_BOOK_DEPTH_HPP
#define SPIRE_BOOK_DEPTH_HPP
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/BookView/CurrentUserOrderModel.hpp"

namespace Spire {
  class TableView;

  /**
   * Displays a widget that contains the BBO panel along with tables for bids
   * and asks.
   */
  class BookDepth : public QWidget {
    public:

      /**
       * Constructs a BookDepth.
       * @param model The book view model.
       * @param properties The properties applied to the table view.
       * @param parent The parent widget.
       */
      BookDepth(std::shared_ptr<BookViewModel> model,
        std::shared_ptr<BookViewPropertiesModel> properties,
        QWidget* parent = nullptr);

      const std::shared_ptr<CurrentUserOrderModel>& get_current() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      std::shared_ptr<BookViewModel> m_model;
      std::shared_ptr<ValueModel<QFont>> m_font_property;
      QFont m_font;
      TableView* m_bid_table_view;
      TableView* m_ask_table_view;
      std::shared_ptr<CurrentUserOrderModel> m_current;
      boost::signals2::scoped_connection m_bid_position_connection;
      boost::signals2::scoped_connection m_ask_position_connection;
      boost::signals2::scoped_connection m_font_property_connection;

      void on_bid_position(int position);
      void on_ask_position(int position);
      void on_font_property_update(const QFont& font);
  };
}

#endif
