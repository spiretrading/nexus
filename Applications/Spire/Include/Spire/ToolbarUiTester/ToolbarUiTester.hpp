#ifndef SPIRE_TOOLBAR_UI_TESTER
#define SPIRE_TOOLBAR_UI_TESTER
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QWidget>
#include "Spire/Toolbar/Toolbar.hpp"
#include "Spire/Toolbar/RecentlyClosedModel.hpp"

namespace Spire {
  
  class ToolbarUiTester : public QWidget {
    public:

      ToolbarUiTester(ToolbarWindow* window,
        RecentlyClosedModel& model,
        QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* receiver, QEvent* event);

    private:
      RecentlyClosedModel* m_recently_closed_model;
      QPushButton* m_add_button;
      QLineEdit* m_text_line_edit;
      QRadioButton* m_book_view_radio;
      QRadioButton* m_time_sale_radio;

      void add_item();
      void remove_item(const RecentlyClosedModel::Entry& e);
  };
}

#endif