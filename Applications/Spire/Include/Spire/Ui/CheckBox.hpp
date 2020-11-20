#ifndef SPIRE_CHECK_BOX_HPP
#define SPIRE_CHECK_BOX_HPP
#include <QCheckBox>

namespace Spire {

  //! A check box with a custom focus style
  class CheckBox : public QCheckBox {
    public:

      //! Constructs a CheckBox
      /*!
        \param text The text to display next to the check box.
        \param parent The parent to this widget.
      */
      explicit CheckBox(const QString& text, QWidget* parent = nullptr);

      //! Sets the CheckBox's stylesheet. Note that these styles are
      //! wrapped in the appropriate CSS selectors, so only the properties
      //! need to be specified.
      /*!
        \param base_style The text style.
        \param indicator_style The default box style.
        \param checked_style The box style when the CheckBox is selected.
        \param hover_style The style when the box or text is hovered.
        \param focused_style The style when the CheckBox has keyboard focus.
      */
      void set_stylesheet(const QString& text_style,
        const QString& indicator_style, const QString& checked_style,
        const QString& hover_style, const QString& focused_style);

    protected:
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      QString m_text_style;
      QString m_indicator_style;
      QString m_checked_style;
      QString m_hover_style;
      QString m_focused_style;
      Qt::FocusReason m_last_focus_reason;

      void set_hover_stylesheet();
      void set_focused_stylesheet();
  };

  //! Constructs a Spire-style CheckBox.
  /*
    \param label The CheckBox's text label.
    \param parent The parent widget.
  */
  CheckBox* make_check_box(const QString& label, QWidget* parent);
}

#endif
