#ifndef SPIRE_TEXT_BOX_HPP
#define SPIRE_TEXT_BOX_HPP
#include <array>
#include <QLineEdit>
#include <QTimeLine>
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a one-line text box.
  class TextBox : public QLineEdit {
    public:

      //! Represents the box style with font properties.
      struct Style : public Box::Style {

        //! The font.
        boost::optional<QFont> m_font;

        //! The text color.
        boost::optional<QColor> m_text_color;
      };

      //! Represents a collection of styles for the text box.
      struct Styles {

        //! The default style.
        Style m_style;

        //! The style of the text box when hovered.
        Style m_hover_style;

        //! The style of the text box when focused with the keyboard.
        Style m_focus_style;

        //! The style of the text box when it's disabled.
        Style m_disabled_style;

        //! The style of the text box when it's read-only.
        Style m_read_only_style;

        //! The style of the text box when it's read-only and hovered.
        Style m_read_only_hover_style;

        //! The style of the text box when it's read-only and focused.
        Style m_read_only_focus_style;

        //! The style of the text box when it's read-only and disabled.
        Style m_read_only_disabled_style;

        //! The style of the text box when the input warning indicator is playing.
        Style m_warning_style;
      };

      //! Signals that the current text changed.
      using CurrentSignal = Signal<void (const QString& text)>;

      //! Signals that the text is submitted.
      using SubmitSignal = Signal<void (const QString& text)>;

      //! Constructs a text box without a default text.
      /*!
        \param parent The parent widget.
      */
      explicit TextBox(QWidget* parent = nullptr);

      //! Constructs a text box with a default text.
      /*!
        \param text The text to display.
        \param parent The parent widget.
      */
      explicit TextBox(const QString& text, QWidget* parent = nullptr);

      //! Gets the text.
      const QString& get_text() const;

      //! Sets the text.
      void set_text(const QString& text);

      //! Gets a selection of styles.
      const Styles& get_styles() const;

      //! Sets styles to the text box.
      void set_styles(const Styles& styles);

      //! Plays the input warning indicator.
      void play_warning();

      //! Connects a slot to the current signal.
      boost::signals2::connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const;

      //! Connects a slot to the submit signal.
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      void changeEvent(QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      bool event(QEvent* event) override;
      QSize sizeHint() const override;

    private:
      mutable CurrentSignal m_current_signal;
      mutable SubmitSignal m_submit_signal;
      bool m_is_hover;
      QString m_text;
      QString m_submitted_text;
      QString m_style_sheet;
      Styles m_styles;
      QTimeLine m_warning_time_line;
      std::array<int, 3> m_warning_background_color_step;
      std::array<int, 3> m_warning_border_color_step;

      QString text() const = delete;
      void setText(const QString&) = delete;
      void on_editing_finished();
      void on_selection_changed();
      void on_text_edited(const QString& text);
      void on_warning_timeout();
      void on_warning_fade_out(int frame);
      void on_warning_finished();
      void elide_text();
      void update_font(const Style& style);
      void update_font();
  };
}

#endif
