#include "Spire/Toolbar/Toolbar.hpp"
#include <QResizeEvent>
#include "ui_Toolbar.h"
#include <QScreen>
//using namespace Beam;
using namespace Spire;
using namespace std;

Toolbar::Toolbar(QWidget* parent, Qt::WindowFlags flags) : m_ui{ std::make_unique<Ui_Toolbar>() }
{
  m_ui->setupUi(this);
  setContentsMargins(0, 0, 0, 0);
  setWindowFlags(Qt::FramelessWindowHint);
  setStyleSheet("background-color: rgb(4,1,8)");
  auto screen = qApp->screens().at(0);
  auto dpiY = screen->physicalDotsPerInchY();
  auto dpiX = screen->physicalDotsPerInchX();
  const auto TOOLBAR_HEIGHT_INCHES = 1.025;
  const auto TOOLBAR_WIDTH_INCHES = 3.375;
  setFixedHeight(TOOLBAR_HEIGHT_INCHES * dpiY);
  setFixedWidth(TOOLBAR_WIDTH_INCHES * dpiX);
  m_ui->m_comboBoxSpacer->changeSize(0.0625 * dpiX, 0,
    QSizePolicy::Fixed);
  m_ui->m_comboBox_1->setFixedHeight(0.2*dpiY);
  m_ui->m_comboBox_2->setFixedHeight(0.2*dpiY);
  
  m_ui->m_canvasButton->setStyleSheet("border-image:url(:/newPrefix/canvas_normal.png)");
  m_ui->m_canvasButton->setFixedSize(0.2*dpiX, 0.2*dpiY);
  m_ui->m_accountButton->setStyleSheet("border-image:url(:/newPrefix/account_normal.png)");
  m_ui->m_accountButton->setFixedSize(0.2*dpiX, 0.2*dpiY);
  m_ui->m_keybindButton->setStyleSheet("border-image:url(:/newPrefix/key_bindings_normal.png)");
  m_ui->m_keybindButton->setFixedSize(0.2*dpiX, 0.2*dpiY);
  m_ui->m_riskButton->setStyleSheet("border-image:url(:/newPrefix/risk_manager_normal.png)");
  m_ui->m_riskButton->setFixedSize(0.2*dpiX, 0.2*dpiY);

  m_ui->m_dashboardButton->setStyleSheet("border-image:url(:/newPrefix/dashboard_normal.png)");
  m_ui->m_dashboardButton->setFixedSize(0.2*dpiX, 0.2*dpiY);

  m_ui->m_bookviewButton->setStyleSheet("border-image:url(:/newPrefix/bookview_normal.png)");
  m_ui->m_bookviewButton->setFixedSize(0.2*dpiX, 0.2*dpiY);

  m_ui->m_timesaleButton->setStyleSheet("border-image:url(:/newPrefix/time_and_sale_normal.png)");
  m_ui->m_timesaleButton->setFixedSize(0.2*dpiX, 0.2*dpiY);

  m_ui->m_chartButton->setStyleSheet("border-image:url(:/newPrefix/chart_normal.png)");
  m_ui->m_chartButton->setFixedSize(0.2*dpiX, 0.2*dpiY);

  m_ui->m_orderimbaButton->setStyleSheet("border-image:url(:/newPrefix/order_imbalances_normal.png)");
  m_ui->m_orderimbaButton->setFixedSize(0.2*dpiX, 0.2*dpiY);

  m_ui->m_blotterButton->setStyleSheet("border-image:url(:/newPrefix/blotter_normal.png)");
  m_ui->m_blotterButton->setFixedSize(0.2*dpiX, 0.2*dpiY);
}

Toolbar::~Toolbar() {}
