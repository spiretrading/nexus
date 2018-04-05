#include "spire/security_input/security_info_widget.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

security_info_widget::security_info_widget(SecurityInfo info, QWidget* parent)
    : QWidget(parent),
      m_info(std::move(info)),
      m_is_highlighted(false) {
  setFixedHeight(scale_height(40));
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setFocusPolicy(Qt::NoFocus);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(scale_width(8), scale_height(6), scale_width(8),
    scale_height(6));
  layout->setSpacing(scale_height(3));
  auto top_line_layout = new QHBoxLayout();
  top_line_layout->setMargin(0);
  top_line_layout->setSpacing(0);
  m_security_name_label = new QLabel(QString::fromStdString(
    Nexus::ToString(m_info.m_security)), this);
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
  auto icon_path = QString(":/icons/%1.png").arg(
    m_info.m_security.GetCountry());
  m_icon_label->setPixmap(QPixmap::fromImage(
    QImage(icon_path).scaled(scale(14, 8), Qt::KeepAspectRatio,
    Qt::SmoothTransformation)));
  top_line_layout->addWidget(m_icon_label);
  layout->addLayout(top_line_layout);
  m_company_name_label = new QLabel(QString::fromStdString(m_info.m_name),
    this);
  m_company_name_label->setStyleSheet(QString(R"(
    background-color: transparent;
    color: #8C8C8C;
    font-family: Roboto;
    font-size: %1px;
  )").arg(scale_height(10)));
  display_company_name();
  layout->addWidget(m_company_name_label);
  setMouseTracking(true);
}

const SecurityInfo& security_info_widget::get_info() const {
  return m_info;
}

void security_info_widget::set_highlighted() {
  if(m_is_highlighted) {
    return;
  }
  setStyleSheet("background-color: #F2F2FF;");
  m_is_highlighted = true;
  m_highlighted_signal(true);
}

void security_info_widget::remove_highlight() {
  if(!m_is_highlighted) {
    return;
  }
  setStyleSheet("background-color: transparent;");
  m_is_highlighted = false;
  m_highlighted_signal(false);
}

connection security_info_widget::connect_highlighted_signal(
    const highlighted_signal::slot_type& slot) const {
  return m_highlighted_signal.connect(slot);
}

connection security_info_widget::connect_commit_signal(
    const commit_signal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

void security_info_widget::enterEvent(QEvent* event) {
  set_highlighted();
}

void security_info_widget::leaveEvent(QEvent* event) {
  remove_highlight();
}

void security_info_widget::mouseReleaseEvent(QMouseEvent* event) {
  m_commit_signal();
}

void security_info_widget::resizeEvent(QResizeEvent* event) {
  display_company_name();
}

void security_info_widget::display_company_name() {
  QFontMetrics metrics(m_company_name_label->font());
  auto shortened_text = metrics.elidedText(
    QString::fromStdString(m_info.m_name), Qt::ElideRight,
    m_company_name_label->width());
  m_company_name_label->setText(shortened_text);
}
