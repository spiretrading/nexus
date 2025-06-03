#ifndef SPIRE_PULL_INDICATOR_HPP
#define SPIRE_PULL_INDICATOR_HPP
#include <QTimer>
#include <QWidget>
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /** Displays an animation while waiting for data to load in a TableView. */
  class PullIndicator : public QWidget {
    public:

      /**
       * Returns the amount of padding needed at the bottom of the TableBody
       * needed to display a PullIndicator.
       */
      static int TABLE_BODY_BOTTOM_PADDING();

      /**
       * Constructs a PullIndicator.
       * @param table_view The TableView to show the loading animation in.
       */
      explicit PullIndicator(TableView& table_view);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      TableView* m_table_view;
      QMovie* m_spinner;
      QTimer m_timer;
      bool m_is_loading;
      int m_last_position;

      void update_position(const QSize& size);
      void display();
      void on_position(int position);
      void on_begin_loading();
      void on_end_loading();
      void on_timeout();
  };
}

#endif
