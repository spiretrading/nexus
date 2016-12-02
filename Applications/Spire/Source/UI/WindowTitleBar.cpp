#include "Spire/UI/WindowTitleBar.hpp"
#include <QBoxLayout>
#include <QLabel>
#include "ui_WindowTitleBar.h"
#include <QIcon>
#include <QFont>
#include <QScreen>
using namespace Spire;
//using namespace Spire::UI;

WindowTitleBar::WindowTitleBar(QWidget* parent)
  :QWidget{parent}, m_ui{ std::make_unique<Ui_WindowTitleBar>() }
{
  m_ui->setupUi(this);
  //setStyleSheet("background-color: red");
  auto screen = qApp->screens().at(0);
  auto dpiY = screen->physicalDotsPerInchY();
  auto dpiX = screen->physicalDotsPerInchX();
  const auto TOOLBAR_HEIGHT_INCHES = 0.25;
  //const auto TOOLBAR_WIDTH_INCHES = 3.25;
  setFixedHeight(TOOLBAR_HEIGHT_INCHES * dpiY);
  //setFixedWidth(TOOLBAR_WIDTH_INCHES * dpiX);
  //setContentsMargins(0, 0, 0, 0);
  m_ui->m_minButton->setFlat(true);
  m_ui->m_minButton->setAutoFillBackground(true);
  m_ui->m_minButton->setStyleSheet("QPushButton{ border-image: url(:/newPrefix/minimize_normal.png); }");
  m_ui->m_minButton->setFixedHeight(0.125*dpiY);
  m_ui->m_minButton->setFixedWidth(0.125*dpiX);
  m_ui->m_minMaxSpacer->changeSize(0.0625 * dpiX, 0,
    QSizePolicy::Fixed);
  m_ui->m_maxButton->setFlat(true);
  m_ui->m_maxButton->setAutoFillBackground(true);
  m_ui->m_maxButton->setStyleSheet("QPushButton{ border-image: url(:/newPrefix/maximize_normal.png); }");
  m_ui->m_maxButton->setFixedHeight(0.125*dpiY);
  m_ui->m_maxButton->setFixedWidth(0.125*dpiX);
  m_ui->m_maxCloseSpacer->changeSize(0.0625 * dpiX, 0,
    QSizePolicy::Fixed);
  m_ui->m_closeButton->setFlat(true);
  m_ui->m_closeButton->setAutoFillBackground(true);
  m_ui->m_closeButton->setStyleSheet("QPushButton{ border-image: url(:/newPrefix/close_normal.png); }");
  m_ui->m_closeButton->setFixedHeight(0.125*dpiY);
  m_ui->m_closeButton->setFixedWidth(0.125*dpiX);


  QFont f{ "Arial", 10, QFont::Normal };
  m_ui->m_windowTitleLabel->setFont(f);
  m_ui->m_windowTitleLabel->setStyleSheet("color: rgb(185, 180, 236);");
}
