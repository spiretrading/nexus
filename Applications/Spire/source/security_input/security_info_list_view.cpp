#include "spire/security_input/security_info_list_view.hpp"
#include <QVBoxLayout>
#include "Nexus/Definitions/Country.hpp"
#include "spire/security_input/security_info_widget.hpp"
#include "spire/spire/dimensions.hpp"

using namespace Nexus;
using namespace spire;

security_info_list_view::security_info_list_view(QWidget* parent)
    : QScrollArea(parent) {
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  setFixedSize(scale(180, 200));
  setWidgetResizable(true);
  setObjectName("security_info_list_view_scrollbar");
  setFrameShape(QFrame::NoFrame);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setStyleSheet(QString(R"(
    #security_info_list_view_scrollbar {
      background-color: #FFFFFF;
      border-bottom: 1px solid #A0A0A0;
      border-left: 1px solid #A0A0A0;
      border-right: 1px solid #A0A0A0;
      border-top: none;
    }
    
    QScrollBar {
      background-color: #FFFFFF;
      border: none;
      width: %1px;
    }

    QScrollBar::handle:vertical {
      background-color: #EBEBEB;
    }

    QScrollBar::sub-line:vertical {
      border: none;
      background: none;
    }

    QScrollBar::add-line:vertical {
      border: none;
      background: none;
    })").arg(scale_height(12)));
  m_list_widget = new QWidget(this);
  auto layout = new QVBoxLayout(m_list_widget);
  layout->setMargin(0);
  layout->setSpacing(0);
  m_list_widget->setStyleSheet("background-color: #FFFFFF;");
  setWidget(m_list_widget);
}

void security_info_list_view::set_list(const std::vector<SecurityInfo>& list) {
  QLayoutItem* item;
  while((item = m_list_widget->layout()->takeAt(0)) != nullptr) {
    delete item->widget();
    delete item;
  }
  for(auto security : list) {
    QString icon_path;
    if(security.m_security.GetCountry() == DefaultCountries::CA()) {
      icon_path = ":/icons/canada.png";
    } else if(security.m_security.GetCountry() == DefaultCountries::US()) {
      icon_path = ":/icons/usa.png";
    }
    auto security_widget = new security_info_widget(
      QString::fromStdString(security.m_security.GetSymbol()) + "."
        + QString(security.m_security.GetMarket().GetData()),
      QString::fromStdString(security.m_name), icon_path, this);
    m_list_widget->layout()->addWidget(security_widget);
  }
}
