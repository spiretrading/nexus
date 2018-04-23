#ifndef SPIRE_SECURITY_INPUT_DIALOG_HPP
#define SPIRE_SECURITY_INPUT_DIALOG_HPP
#include <QDialog>
#include <QPoint>
#include "Nexus/Definitions/Security.hpp"
#include "spire/security_input/security_input.hpp"
#include "spire/security_input/security_input_box.hpp"
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays a security input box within a dialog window.
  class security_input_dialog : public QDialog {
    public:

      //! Constructs a security input dialog.
      /*!
        \param model The model to query for securities.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      security_input_dialog(security_input_model& model,
        QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      //! Constructs a security input dialog with an initial text value.
      /*!
        \param model The model to query for securities.
        \param initial_text The initial text to input into the input line edit.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      security_input_dialog(security_input_model& model, const QString& text,
        QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      ~security_input_dialog();

      //! Returns the security that was input.
      const Nexus::Security& get_security() const noexcept;

    private:
      void closeEvent(QCloseEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      Nexus::Security m_security;
      std::unique_ptr<drop_shadow> m_shadow;
      security_input_box* m_security_input_box;
      bool m_is_dragging;
      QPoint m_last_mouse_pos;

      void set_security(const Nexus::Security& security);
  };
}

#endif
