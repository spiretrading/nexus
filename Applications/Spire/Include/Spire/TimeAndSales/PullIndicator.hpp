#ifndef SPIRE_PULL_INDICATOR_HPP
#define SPIRE_PULL_INDICATOR_HPP
#include <QTimer>
#include <QWidget>
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
      enum class State {
        NONE,
        LOADING,
        RELOADING
      };
      TableView* m_table_view;
      QMovie* m_spinner;
      QWidget* m_loading_panel;
      QMovie* m_loading_spinner;
      QTimer m_timer;
      State m_state;
      int m_load_index;
      int m_last_position;

      void update_position(const QSize& size);
      void update_loading_panel();
      void display();
      void load_page();
      void fill();
      void on_horizontal_position(int position);
      void on_vertical_position(int position);
      void on_begin_loading();
      void on_end_loading();
      void on_operation(const TableModel::Operation& operation);
      void on_timeout();
  };
}

#endif
