#ifndef SPIRE_CHECK_BOX_HPP
#define SPIRE_CHECK_BOX_HPP
#include <QCheckBox>

namespace spire {

  //! A check box with a custom focus style
  class check_box : public QCheckBox {
    public:

      //! Constructs a check_box
      check_box(const QString& text, QWidget* parent = nullptr);

      //! Sets the check_box's stylesheet. Note that these styles are appended
      //! verbatim, so they should be wrapped in the appropriate CSS
      //! selectors before being passed in.
      /*
        \param default_style The text and box style when the check_box isn't
               hovered or focused, but is either checked or unchecked.
        \param hover_style The box style when the check_box is hovered.
        \param focused_style The box style when the check_box is focused.
      */
      void set_stylesheet(const QString& default_style,
        const QString& hover_style, const QString& focused_style);

    protected:
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;

    private:
      QString m_default_style;
      QString m_hover_style;
      QString m_focused_style;

      void set_hover_stylesheet();
      void set_focused_stylesheet();
  };
}

#endif
