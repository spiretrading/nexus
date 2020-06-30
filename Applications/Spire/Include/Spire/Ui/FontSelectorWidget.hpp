#ifndef SPIRE_FONT_SELECTOR_WIDGET_HPP
#define SPIRE_FONT_SELECTOR_WIDGET_HPP
#include <QFont>
#include <QSpinBox>
#include <QWidget>
#include "Spire/Ui/DropDownMenu.hpp"
#include "Spire/Ui/FontSelectorButton.hpp"

namespace Spire {

  //! Represents a widget for selecting system fonts.
  class FontSelectorWidget : public QWidget {
    public:

      using FontPreviewSignal = Signal<void (const QFont& font)>;

      //! Signal type for font selection.
      /*!
        \param font The selected font.
      */
      using FontSelectedSignal = Signal<void (const QFont& font)>;

      //! Constructs a FontSelectorWidget.
      /*!
        \param font The initial font to display.
        \param parent The parent widget.
      */
      explicit FontSelectorWidget(const QFont& font,
        QWidget* parent = nullptr);

      //! Returns the font represented by this widget.
      const QFont& get_font() const;

      boost::signals2::connection connect_font_preview_signal(
        const FontSelectedSignal::slot_type& slot) const;

      //! Connects a slot to the font selection signal.
      boost::signals2::connection connect_font_selected_signal(
        const FontSelectedSignal::slot_type& slot) const;

    private:
      mutable FontPreviewSignal m_font_preview_signal;
      mutable FontSelectedSignal m_font_selected_signal;
      QFont m_current_font;
      DropDownMenu* m_font_list;
      QSpinBox* m_size_spin_box;
      FontSelectorButton* m_bold_button;
      FontSelectorButton* m_italics_button;
      FontSelectorButton* m_underline_button;

      void on_bold_button_clicked();
      void on_italics_button_clicked();
      void on_underline_button_clicked();
      void on_font_list_closed();
      void on_font_preview(const QString& family);
      void on_font_selected(const QString& family);
      void on_size_selected(int size);
  };
}

#endif
