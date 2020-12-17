#ifndef SPIRE_FILTERED_DROP_DOWN_MENU_TEST_WIDGET_HPP
#define SPIRE_FILTERED_DROP_DOWN_MENU_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QWidget>
#include "Spire/Ui/FilteredDropDownMenu.hpp"
#include "Spire/Ui/FlatButton.hpp"

namespace Spire {

  //! Represents a widget for testing a FilteredDropDownMenuTestWidget.
  class FilteredDropDownMenuTestWidget : public QWidget {
    public:

      //! Constructs a FilteredDropDownMenuTestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit FilteredDropDownMenuTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      FilteredDropDownMenu* m_menu;
      QLabel* m_status_label;
      QPlainTextEdit* m_item_input;
      FlatButton* m_set_button;
      FlatButton* m_reset_button;

      void on_set_button();
      void on_reset_button();
  };
}

#endif
