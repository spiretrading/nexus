#include "spire/time_and_sales/snapshot_loading_widget.hpp"
#include <QMovie>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"

using namespace spire;

snapshot_loading_widget::snapshot_loading_widget(QWidget* parent)
    : QWidget(parent) {
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setFixedHeight(scale_height(32));
  m_logo_widget = new QLabel(this);
  auto logo = new QMovie(":/icons/pre-loader.gif", QByteArray(),
    m_logo_widget);
  logo->setScaledSize(scale(16, 16));
  m_logo_widget->setMovie(logo);
  m_logo_widget->setAlignment(Qt::AlignCenter);
  m_logo_widget->movie()->start();
}

void snapshot_loading_widget::paintEvent(QPaintEvent* event) {
  auto rect = visibleRegion().boundingRect();
  m_logo_widget->setGeometry(rect.x(), rect.y(), rect.width(), height());
  QWidget::paintEvent(event);
}

void snapshot_loading_widget::resizeEvent(QResizeEvent* event) {
  setFixedWidth(static_cast<QWidget*>(parent())->width());
}
