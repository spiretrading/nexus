#ifndef SPIRE_FONT_SELECTOR_WIDGET_HPP
#define SPIRE_FONT_SELECTOR_WIDGET_HPP
#include <QWidget>
#include "Spire/Ui/DropDownMenu.hpp"
#include "Spire/Ui/FlatButton.hpp"

namespace Spire {

  class FontSelectorWidget : public QWidget {
    public:

      FontSelectorWidget(const QFont& font, QWidget* parent = nullptr);

    private:
      DropDownMenu* m_font_list;
      DropDownMenu* m_style_list;
      DropDownMenu* m_size_list;
      FlatButton* m_bold_button;
      FlatButton* m_italics_button;
      FlatButton* m_underline_button;
      FlatButton* m_strikethrough_button;
  };
}

#endif
