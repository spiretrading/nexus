#ifndef SPIRE_SECURITY_INPUT_DIALOG_HPP
#define SPIRE_SECURITY_INPUT_DIALOG_HPP
#include <QDialog>
#include <QPoint>
#include "Nexus/Definitions/Security.hpp"
#include "spire/security_input/security_input.hpp"
#include "spire/security_input/security_input_box.hpp"

namespace spire {

  //! \brief Displays a security input box within a dialog window.
  class security_input_dialog : public QDialog {
    public:

      //! Constructs a security input dialog.
      /*!
        \param model The model to query for securities.
        \param parent The parent widget.
      */
      security_input_dialog(security_input_model& model,
        QWidget* parent = nullptr);

      //! Returns the security that was input.
      const Nexus::Security& get_security() const noexcept;

    private:
      void closeEvent(QCloseEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      Nexus::Security m_security;
      QWidget* m_dialog;
      security_input_box* m_security_input_box;
      bool m_is_dragging;
      QPoint m_last_mouse_pos;

      void set_security(const Nexus::Security& security);
  };
}

#endif
