#ifndef SPIRE_LOGINDIALOG_HPP
#define SPIRE_LOGINDIALOG_HPP
#include <memory>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationException.hpp>
#include <Beam/ServiceLocator/ServiceLocator.hpp>
#include <Beam/ServiceLocator/SessionEncryption.hpp>
#include <QDialog>
#include <QPoint>

class Ui_LoginDialog;

namespace Spire {
  class LoginDialog : public QDialog {
    public:
      explicit LoginDialog(QWidget* parent = nullptr);
      virtual ~LoginDialog();

    protected:
      void mousePressEvent(QMouseEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;

    private:
      std::unique_ptr<Ui_LoginDialog> m_ui;
      QPoint m_mousePoint;
  };
}

#endif
