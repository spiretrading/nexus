#include "spire/security_input/security_info_widget.hpp"
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"






#include <QDebug>






using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

security_info_widget::security_info_widget(const SecurityInfo& security_info,
    const QString& icon_path, QWidget* parent)
    : QWidget(parent),
      m_security(security_info.m_security) {
  setFixedHeight(scale_height(40));
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setFocusPolicy(Qt::StrongFocus);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(scale_width(8), scale_height(6), scale_width(8),
    scale_height(6));
  layout->setSpacing(scale_height(3));
  //setStyleSheet(":hover { background-color: #F2F2FF; }");
  auto top_line_layout = new QHBoxLayout();
  top_line_layout->setMargin(0);
  top_line_layout->setSpacing(0);
  m_security_name_label = new QLabel(QString::fromStdString(
    Nexus::ToString(m_security)), this);
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
  QFontMetrics metrics(m_company_name_label->font());
  auto shortened_text = metrics.elidedText(
    QString::fromStdString(security_info.m_name), Qt::ElideRight,
    m_company_name_label->width());
  m_company_name_label->setText(shortened_text);
  layout->addWidget(m_company_name_label);
  setMouseTracking(true);
}

void security_info_widget::mouseReleaseEvent(QMouseEvent* event) {
  // focusInEvent is fired when the mouse is clicked, so it might work
  // to use both up/down keys and mouse clicks to set the current text in
  // the line edit
  m_commit_signal(m_security);
}

void security_info_widget::enterEvent(QEvent* event) {
  setStyleSheet("background-color: #F2F2FF;");
  m_hovered_signal(this);
  qDebug() << "Enter";
  repaint();
  update();
}
//
void security_info_widget::leaveEvent(QEvent* event) {
  setStyleSheet("background-color: transparent;");
  qDebug() << "Leave";
  repaint();
  update();
}

void security_info_widget::focusInEvent(QFocusEvent* event) {
  setStyleSheet("background-color: #F2F2FF;");
  // The program crashes if I call this
  //m_commit_signal(m_security);
}

void security_info_widget::focusOutEvent(QFocusEvent* event) {
  setStyleSheet("background-color: transparent;");
}

connection security_info_widget::connect_commit_signal(
    const commit_signal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

connection security_info_widget::connect_hovered_signal(
    const hovered_signal::slot_type& slot) const {
  return m_hovered_signal.connect(slot);
}

Security security_info_widget::get_security() {
  return m_security;
}
