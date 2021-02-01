#ifndef SPIRE_CHECKBOX_HPP
#define SPIRE_CHECKBOX_HPP
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
      void paintEvent(QPaintEvent* event) override;
      bool hitButton(const QPoint& pos) const override;

    private:
      bool m_is_read_only;
  };
}

#endif
