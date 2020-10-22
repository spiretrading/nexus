#ifndef SPIRE_COMBO_BOX_TEST_WIDGET_HPP
#define SPIRE_COMBO_BOX_TEST_WIDGET_HPP
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/UiViewer/ComboBoxAdapter.hpp"

namespace Spire {

  //! Represents a widget for testing Spire combo boxes.
  class ComboBoxTestWidget : public QWidget {
    public:

      //! Constructs a SpinBoxTestWidget.
      /*!
        \param combo_box The combo box to test.
        \param parent The parent widget.
      */
      explicit ComboBoxTestWidget(ComboBoxAdapter* combo_box,
        QWidget* parent = nullptr);

    private:
      boost::signals2::scoped_connection m_selection_connection;
      CustomVariantItemDelegate m_item_delegate;
  };
}

#endif
