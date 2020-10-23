#ifndef SPIRE_SECURITY_WIDGET_TEST_WIDGET_HPP
#define SPIRE_SECURITY_WIDGET_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/SecurityWidget.hpp"

namespace Spire {

  //! Represents a widget for testing a SecurityWidget.
  class SecurityWidgetTestWidget : public QWidget {
    public:

      //! Constructs a SecurityWidgetTestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit SecurityWidgetTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      CheckBox* m_dark_theme_check_box;
      QLabel* m_status_label;
      SecurityWidget* m_security_widget;
      CustomVariantItemDelegate m_item_delegate;

      void on_reset_button();
      void on_security_selected(const Nexus::Security& security);
  };
}

#endif
