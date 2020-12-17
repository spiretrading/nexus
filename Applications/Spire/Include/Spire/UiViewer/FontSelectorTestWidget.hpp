#ifndef SPIRE_FONT_SELECTOR_TEST_WIDGET_HPP
#define SPIRE_FONT_SELECTOR_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/Ui/FontSelectorWidget.hpp"
#include "Spire/Ui/IntegerSpinBox.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Represents a widget for testing a FontSelectorWidget.
  class FontSelectorTestWidget : public QWidget {
    public:

      //! Constructs a FontSelectorTestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit FontSelectorTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      FontSelectorWidget* m_font_selector;
      QLabel* m_selected_label;
      QLabel* m_preview_label;
      StaticDropDownMenu* m_family_list;
      CheckBox* m_bold_check_box;
      CheckBox* m_italic_check_box;
      CheckBox* m_underline_check_box;
      IntegerSpinBox* m_size_spin_box;
      FlatButton* m_set_button;
      FlatButton* m_reset_button;

      QFont get_font() const;
      void update_labels();
      void on_font_set();
      void on_widget_reset();
  };
}

#endif
