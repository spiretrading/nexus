#ifndef SPIRE_SCROLL_AREA_TEST_WIDGET_HPP
#define SPIRE_SCROLL_AREA_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/ScrollArea.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

namespace Spire {

  class ScrollAreaTestWidget : public QWidget {
    public:

      ScrollAreaTestWidget(QWidget* parent = nullptr);

    private:
      QGridLayout* m_layout;
      TextInputWidget* m_width_input;
      TextInputWidget* m_height_input;
      CheckBox* m_dynamic_check_box;
      ScrollArea* m_scroll_area;
      QLabel* m_widget;

      bool set_widget_size();
      void on_reset_button();
      void on_resize_button();
  };
}

#endif
