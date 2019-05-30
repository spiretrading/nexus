#ifndef SPIRE_TREND_LINE_STYLE_DROPDOWN_MENU_HPP
#define SPIRE_TREND_LINE_STYLE_DROPDOWN_MENU_HPP
#include <QWidget>
#include "spire/charting/charting.hpp"
#include "spire/charting/trend_line.hpp"

namespace Spire {

  //! Displays a Spire-styled QComboBox.
  class TrendLineStyleDropdownMenu : public QWidget {
    public:

      //! Constructs a TrnedLineStyleDropdownMenu.
      /*
        \param parent The parent widget to the DropdownMenu.
      */
      TrendLineStyleDropdownMenu(QWidget* parent = nullptr);

      //! Returns the selected item.
      TrendLineStyle get_style() const;

    protected:
      bool eventFilter(QObject* object, QEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      TrendLineStyle m_current_style;
      QImage m_dropdown_image;
      StyleDropdownMenuList* m_menu_list;

      void move_menu_list();
      void on_clicked();
      void on_item_selected(TrendLineStyle style);
  };
}

#endif
