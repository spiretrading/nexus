#include "Spire/SecurityInput/SecurityInfoWidget.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SecurityInfoWidget::SecurityInfoWidget(SecurityInfo info, QWidget* parent)
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
  top_line_layout->setContentsMargins({});
  top_line_layout->setSpacing(0);
  CustomVariantItemDelegate item_delegate;
  m_security_name_label = new QLabel(item_delegate.displayText(
    QVariant::fromValue(m_info.m_security), QLocale()), this);
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
  auto icon_path = QString(":/Icons/%1.png").arg(
    static_cast<std::uint16_t>(m_info.m_security.GetCountry()));
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

const SecurityInfo& SecurityInfoWidget::get_info() const {
  return m_info;
}

void SecurityInfoWidget::set_highlighted() {
  if(m_is_highlighted) {
    return;
  }
  setStyleSheet("background-color: #F2F2FF;");
  m_is_highlighted = true;
  m_highlighted_signal(true);
}

void SecurityInfoWidget::remove_highlight() {
  if(!m_is_highlighted) {
    return;
  }
  setStyleSheet("background-color: transparent;");
  m_is_highlighted = false;
  m_highlighted_signal(false);
}

connection SecurityInfoWidget::connect_highlighted_signal(
    const HighlightedSignal::slot_type& slot) const {
  return m_highlighted_signal.connect(slot);
}

connection SecurityInfoWidget::connect_commit_signal(
    const CommitSignal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

void SecurityInfoWidget::enterEvent(QEvent* event) {
  set_highlighted();
}

void SecurityInfoWidget::leaveEvent(QEvent* event) {
  remove_highlight();
}

void SecurityInfoWidget::mouseReleaseEvent(QMouseEvent* event) {
  m_commit_signal();
}

void SecurityInfoWidget::resizeEvent(QResizeEvent* event) {
  display_company_name();
}

void SecurityInfoWidget::display_company_name() {
  QFontMetrics metrics(m_company_name_label->font());
  auto shortened_text = metrics.elidedText(
    QString::fromStdString(m_info.m_name), Qt::ElideRight,
    m_company_name_label->width());
  m_company_name_label->setText(shortened_text);
}
