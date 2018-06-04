#ifndef SPIRE_TOOLBAR_UI_TESTER
#define SPIRE_TOOLBAR_UI_TESTER
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QWidget>
#include "spire/toolbar/toolbar.hpp"
#include "spire/toolbar/recently_closed_model.hpp"

namespace spire {
  
  class toolbar_ui_tester : public QWidget {
    public:

      toolbar_ui_tester(toolbar_window* window,
        recently_closed_model& model,
        QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* receiver, QEvent* event);

    private:
      recently_closed_model* m_recently_closed_model;
      QPushButton* m_add_button;
      QLineEdit* m_text_line_edit;
      QRadioButton* m_book_view_radio;
      QRadioButton* m_time_sale_radio;

      void add_item();
      void remove_item(const recently_closed_model::entry& e);
  };
}

#endif