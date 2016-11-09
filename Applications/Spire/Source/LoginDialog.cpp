#include "Spire/LoginDialog.hpp"
#include "ui_LoginDialog.h"
#include <QDebug>
#include <QScreen>
#include <QMovie>
LoginDialog::LoginDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui_LoginDialog){
  QScreen *screen = qApp->screens().at(0);
  ui->setupUi(this);

  setStyleSheet("background-image: url(:/newPrefix/login-bg.png);");


  double physicalDotsPerInchY = screen->physicalDotsPerInchY();
  double physicalDotsPerInchX = screen->physicalDotsPerInchX();
  QFont f("Roboto", 10, QFont::Normal);

  //set LoginDialog Dimension
  double LoginDialogHeightInches = 3.6;
  double LoginDialogWidthInches = 4;
  this->setFixedHeight(3.6*physicalDotsPerInchY);
  this->setFixedWidth(4*physicalDotsPerInchX);
  
  ui->LeftSpacer->changeSize(0.425*physicalDotsPerInchX, 0, QSizePolicy::Fixed);
  ui->RightSpacer->changeSize(0.5*physicalDotsPerInchX, 0, QSizePolicy::Fixed);
  ui->BottomSpacer->changeSize(0, 0*physicalDotsPerInchY, QSizePolicy::Fixed);

  ui->widget->move(0, 0);
  ui->widget->setFixedHeight(this->height());
  ui->widget->setFixedWidth(this->width());
  ui->widget->setStyleSheet("background:transparent;");

  ui->UsernameInput->setFixedHeight(0.28*physicalDotsPerInchY);
  ui->UsernameInput->setFixedWidth(3* physicalDotsPerInchX);
  //ui->UsernameInput->setFont(f);
  //ui->UsernameInput->setStyleSheet("color:grey;");

  ui->PasswordInput->setFixedHeight(0.28*physicalDotsPerInchY);
  ui->PasswordInput->setFixedWidth(3 * physicalDotsPerInchX);
  //ui->PasswordInput->setFont(f);
  //ui->PasswordInput->setStyleSheet("color:grey");

  ui->VersionLabel->setFixedHeight(0.28*physicalDotsPerInchY);
  ui->VersionLabel->setFixedWidth(1* physicalDotsPerInchX);
  ui->VersionLabel->setStyleSheet("color:rgb(255,255,255)");
  ui->VersionLabel->setText("Version 1.0.0");
  ui->VersionLabel->setFont(f);
  ui->VersionLoginSpacer->changeSize(0.40*physicalDotsPerInchX, 0, QSizePolicy::Fixed);
  
  QMovie *movie = new QMovie(":/newPrefix/spire_desktop_loading_icon.gif");
  ui->LoadingLabel->setMovie(movie);
  ui->LoadingLabel->setFixedHeight(0.28*physicalDotsPerInchY);
  ui->LoadingLabel->setFixedWidth(1*physicalDotsPerInchX);
  ui->LoadingLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  movie->start();

  ui->LoginButton->setFixedHeight(0.28*physicalDotsPerInchY);
  ui->LoginButton->setFixedWidth(1 * physicalDotsPerInchX);
  ui->LoginButton->setFont(f);
  ui->LoginButton->setStyleSheet("background-color:rgb(104, 75, 199); color: white; border: 2px solid rgb(141,120,236);");

  ui->SpireLogoLabel->setFixedHeight(0.5*physicalDotsPerInchY);
  ui->SpireLogoLabel->setFixedWidth(1.44*physicalDotsPerInchX);
  ui->SpireLogoLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  ui->SpireLogoLabel->setStyleSheet("background - image: url(: / newPrefix / spire - white.png);");

  ui->MainLayout->setAlignment(Qt::AlignCenter);

  ui->LogoTopSpacer->changeSize(0, 0.4*physicalDotsPerInchY, QSizePolicy::Fixed);
  ui->LogoSpacer->changeSize(0, 0.25*physicalDotsPerInchY, QSizePolicy::Fixed);
  ui->UsernamePasswordSpacer->changeSize(0, 0.125*physicalDotsPerInchY, QSizePolicy::Fixed);
  ui->PasswordLoginSpacer->changeSize(0, 0.25*physicalDotsPerInchY, QSizePolicy::Fixed);

  //ui->InvalidLabel->hide();
  ui->InvalidLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  ui->InvalidLabel->setFont(f);
  ui->InvalidLabel->setStyleSheet("color : rgb(219, 213, 44)");

}

/*LoginDialog::~LoginDialog(){
  delete ui;
}*/