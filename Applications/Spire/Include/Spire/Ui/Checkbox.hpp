#ifndef SPIRE_CHECK_BOX_HPP
#define SPIRE_CHECK_BOX_HPP
#include <QCheckBox>

namespace Spire {

  //! Represents a Spire-styled checkbox.
  class Checkbox : public QCheckBox {
    public:

      //! Constructs a Checkbox without a label.
      /*!
        \param parent The parent widget.
      */
      explicit Checkbox(QWidget* parent = nullptr);
  
      //! Constructs a Checkbox with a label.
      /*!
        \param label The text to display next to the check box.
        \param parent The parent widget.
      */
      explicit Checkbox(const QString& label, QWidget* parent = nullptr);
  
      //! Sets the read-only state of the Checkbox.
      /*!
        \param is_read_only Sets the Checkbox to read-only iff is_read_only is
                true.
      */
      void set_read_only(bool is_read_only);

    protected:
      bool event(QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      bool m_is_read_only;
      QImage m_check_icon;

      QColor get_box_color() const;
      QPoint get_box_position() const;
      QColor get_check_color() const;
      QPoint get_check_position() const;
      QColor get_inner_box_color() const;
      QRect get_inner_box_rect() const;
      QPoint get_text_position() const;
  };
}

#endif
