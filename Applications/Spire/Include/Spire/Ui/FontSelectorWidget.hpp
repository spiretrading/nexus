#ifndef SPIRE_FONT_SELECTOR_WIDGET_HPP
#define SPIRE_FONT_SELECTOR_WIDGET_HPP
#include <QFont>
#include <QWidget>
#include "Spire/Ui/DropDownMenu.hpp"
#include "Spire/Ui/FontSelectorButton.hpp"

namespace Spire {

  //! Represents a widget for selecting system fonts.
  class FontSelectorWidget : public QWidget {
    public:

      //! Signal type for font selection.
      /*!
        \param font The selected font.
      */
      using FontSignal = Signal<void (const QFont& font)>;

      //! Constructs a FontSelectorWidget.
      /*!
        \param font The initial font to display.
        \param parent The parent widget.
      */
      explicit FontSelectorWidget(const QFont& font,
        QWidget* parent = nullptr);

      //! Returns the font represented by this widget.
      const QFont& get_font() const;

      //! Connects a slot to the font selection signal.
      boost::signals2::connection connect_font_signal(
        const FontSignal::slot_type& slot) const;

    private:
      mutable FontSignal m_font_signal;
      QFont m_current_font;
      DropDownMenu* m_font_list;
      DropDownMenu* m_size_list;
      FontSelectorButton* m_bold_button;
      FontSelectorButton* m_italics_button;
      FontSelectorButton* m_underline_button;

      void on_bold_button_clicked();
      void on_italics_button_clicked();
      void on_underline_button_clicked();
      void on_font_selected(const QString& family);
      void on_size_selected(const QString& size);
  };
}

#endif
