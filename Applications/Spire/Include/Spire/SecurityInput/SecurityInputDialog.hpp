#ifndef SPIRE_SECURITY_INPUT_DIALOG_HPP
#define SPIRE_SECURITY_INPUT_DIALOG_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QDialog>
#include <QPoint>
#include <QVBoxLayout>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/SecurityInput/SecurityInput.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a security input box within a dialog window.
  class SecurityInputDialog : public QDialog {
    public:

      //! Constructs a security input dialog.
      /*!
        \param model The model to query for securities.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      explicit SecurityInputDialog(Beam::Ref<SecurityInputModel> model,
        QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      //! Constructs a security input dialog with an initial text value.
      /*!
        \param model The model to query for securities.
        \param initial_text The initial text to input into the input line edit.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      SecurityInputDialog(Beam::Ref<SecurityInputModel> model,
        const QString& text, QWidget* parent = nullptr,
        Qt::WindowFlags flags = 0);

      ~SecurityInputDialog();

      //! Returns the security that was input.
      const Nexus::Security& get_security() const noexcept;

    protected:
      void closeEvent(QCloseEvent* event) override;
      bool event(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      Nexus::Security m_security;
      SecurityInputModel* m_model;
      DropShadow* m_shadow;
      QVBoxLayout* m_layout;
      SecurityInputLineEdit* m_security_line_edit;
      QString m_initial_text;
      bool m_is_dragging;
      QPoint m_last_mouse_pos;

      void set_security(const Nexus::Security& security);
  };
}

#endif
