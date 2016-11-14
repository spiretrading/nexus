#ifndef SPIRE_LOGINDIALOG_HPP
#define SPIRE_LOGINDIALOG_HPP
#include <memory>
#include <QMouseEvent>
#include <QDialog>
#include <QPoint>
class Ui_LoginDialog;

namespace Spire {
  class LoginDialog : public QDialog {
    public:
      explicit LoginDialog(QWidget* parent = nullptr);
      virtual ~LoginDialog();

    private:
      std::unique_ptr<Ui_LoginDialog> m_ui;
      void mousePressEvent(QMouseEvent* event);
      void mouseMoveEvent(QMouseEvent* event);
      QPoint mousePoint;
  };
}

#endif
