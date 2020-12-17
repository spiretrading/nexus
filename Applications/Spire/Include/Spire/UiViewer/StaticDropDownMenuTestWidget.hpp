#ifndef SPIRE_STATIC_DROP_DOWN_MENU_TEST_WIDGET_HPP
#define SPIRE_STATIC_DROP_DOWN_MENU_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QWidget>
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

namespace Spire {

  //! Represents a widget for testing a StatiDropDownMenuTestWidget.
  class StaticDropDownMenuTestWidget : public QWidget {
    public:

      //! Constructs a StaticDropDownMenuTestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit StaticDropDownMenuTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      StaticDropDownMenu* m_menu;
      QLabel* m_status_label;
      QPlainTextEdit* m_item_input;
      TextInputWidget* m_label_input;
      FlatButton* m_set_button;
      FlatButton* m_reset_button;
      TextInputWidget* m_insert_item_input;
      FlatButton* m_insert_item_button;
      TextInputWidget* m_index_input;
      FlatButton* m_remove_item_button;
      CheckBox* m_next_item_check_box;

      void on_insert_button();
      void on_set_button();
      void on_remove_button();
      void on_reset_button();
  };
}

#endif
