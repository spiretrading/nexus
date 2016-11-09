#include <QDialog>

class Ui_LoginDialog;

class LoginDialog : public QDialog{

public:
  explicit LoginDialog(QWidget *parent = nullptr);
  //~LoginDialog();

private:
  Ui_LoginDialog *ui;
};