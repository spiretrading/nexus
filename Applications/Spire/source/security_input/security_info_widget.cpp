#include "spire/security_input/security_info_widget.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

security_info_widget::security_info_widget(const SecurityInfo& security_info,
    const QString& icon_path, QWidget* parent)
    : QWidget(parent),
      m_security(security_info.m_security) {
  setFixedSize(scale(166, 40));
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(scale_width(8), scale_height(6), scale_width(8),
    scale_height(6));
  layout->setSpacing(scale_height(3));
  setStyleSheet(":hover { background-color: #F2F2FF; }");
  auto top_line_layout = new QHBoxLayout();
  top_line_layout->setMargin(0);
  top_line_layout->setSpacing(0);
  m_security_name_label = new QLabel(QString::fromStdString(
    m_security.GetSymbol()) + "."
      + QString(m_security.GetMarket().GetData()), this);
  m_security_name_label->setStyleSheet(QString(R"(
    background-color: transparent;
    color: #333333;
    font-family: Roboto;
    font-size: %1px;
  )").arg(scale_height(12)));
  top_line_layout->addWidget(m_security_name_label);
  m_icon_label = new QLabel(this);
  m_icon_label->setFixedSize(scale(14, 14));
  m_icon_label->setStyleSheet("background-color: transparent;");
  m_icon_label->setPixmap(QPixmap::fromImage(
    QImage(icon_path).scaled(scale(14, 8), Qt::KeepAspectRatio,
      Qt::SmoothTransformation)));
  top_line_layout->addWidget(m_icon_label);
  layout->addLayout(top_line_layout);
  m_company_name_label = new QLabel(
    QString::fromStdString(security_info.m_name), this);
  m_company_name_label->setFixedWidth(scale_width(136));
  m_company_name_label->setStyleSheet(QString(R"(
    background-color: transparent;
    color: #8C8C8C;
    font-family: Roboto;
    font-size: %1px;
  )").arg(scale_height(10)));
  //QFontMetrics metrics(m_company_name_label->font());
  //auto shortened_text = metrics.elidedText(company_name,
  //  Qt::ElideRight, m_company_name_label->width());
  //m_company_name_label->setText(shortened_text);
  layout->addWidget(m_company_name_label);
}

void security_info_widget::mouseReleaseEvent(QMouseEvent* event) {
  m_clicked_signal(m_security);
}

connection security_info_widget::connect_clicked_signal(
    const clicked_signal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}
