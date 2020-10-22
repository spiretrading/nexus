#ifndef SPIRE_SECURITY_INPUT_TEST_WIDGET_HPP
#define SPIRE_SECURITY_INPUT_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include "Spire/SecurityInput/LocalSecurityInputModel.hpp"
#include "Spire/SecurityInput/SecurityInputLineEdit.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

namespace Spire {

  //! Represents a widget for testing a SecurityInputLineEdit.
  class SecurityInputTestWidget : public QWidget {
    public:

      //! Constructs a SecurityInputTestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit SecurityInputTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      QLabel* m_status_label;
      LocalSecurityInputModel m_model;
      SecurityInputLineEdit* m_security_input;
      TextInputWidget* m_initial_text_input;
      CheckBox* m_icon_check_box;
      CustomVariantItemDelegate m_item_delegate;

      void on_reset_button();
  };
}

#endif
