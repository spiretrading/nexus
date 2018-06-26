#include "spire/time_and_sales/snapshot_loading_widget.hpp"
#include <QMovie>
#include <QScrollBar>
#include "spire/spire/dimensions.hpp"

using namespace spire;

snapshot_loading_widget::snapshot_loading_widget(QScrollArea* scroll_area,
    QWidget* parent)
    : QWidget(parent),
      m_scroll_area(scroll_area) {
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setFixedHeight(scale_height(32));
  m_logo_widget = new QLabel(this);
  auto logo = new QMovie(":/icons/pre-loader.gif", QByteArray(),
    m_logo_widget);
  logo->setScaledSize(scale(16, 16));
  m_logo_widget->setMovie(logo);
  m_logo_widget->setAlignment(Qt::AlignCenter);
  m_logo_widget->movie()->start();
  m_scroll_area->installEventFilter(this);
  m_scroll_area->widget()->installEventFilter(this);
  connect(m_scroll_area->horizontalScrollBar(), &QScrollBar::valueChanged, this,
    [=] (auto i) { update_logo_geometry(); });
}

bool snapshot_loading_widget::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_scroll_area) {
    if(event->type() == QEvent::Resize) {
      update_logo_geometry();
    }
  } else if(watched == m_scroll_area->widget()) {
    if(event->type() == QEvent::Resize) {
      setFixedWidth(m_scroll_area->widget()->width());
    }
  }
  return QWidget::eventFilter(watched, event);
}

void snapshot_loading_widget::showEvent(QShowEvent* event) {
  update_logo_geometry();
}

void snapshot_loading_widget::update_logo_geometry() {
  auto rect = m_scroll_area->widget()->visibleRegion().boundingRect();
  m_logo_widget->setGeometry(rect.x(), 0, rect.width(), height());
}
