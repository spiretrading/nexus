#ifndef SPIRE_FONT_SELECTOR_WIDGET_HPP
#define SPIRE_FONT_SELECTOR_WIDGET_HPP
#include <QFont>
#include <QWidget>
#include "Spire/Ui/DropDownMenu.hpp"
#include "Spire/Ui/FlatButton.hpp"

namespace Spire {

  class FontSelectorWidget : public QWidget {
    public:

      FontSelectorWidget(const QFont& font, QWidget* parent = nullptr);

    private:
      QFont m_current_font;
      DropDownMenu* m_font_list;
      DropDownMenu* m_size_list;
      FlatButton* m_bold_button;
      FlatButton* m_italics_button;
      FlatButton* m_underline_button;

      void on_font_selected(const QString& family);
      void on_size_selected(const QString& size);
      void on_style_selected(const QString& style);
  };
}

#endif
