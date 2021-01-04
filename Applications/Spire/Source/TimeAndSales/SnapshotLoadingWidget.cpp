#include "Spire/TimeAndSales/SnapshotLoadingWidget.hpp"
#include <QMovie>
#include <QResizeEvent>
#include <QScrollBar>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

SnapshotLoadingWidget::SnapshotLoadingWidget(QScrollArea* scroll_area,
    QWidget* parent)
    : QWidget(parent),
      m_scroll_area(scroll_area) {
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setFixedHeight(scale_height(44));
  m_logo_widget = new QLabel(this);
  auto logo = new QMovie(":/Icons/pre-loader.gif", QByteArray(),
    m_logo_widget);
  logo->setScaledSize(scale(16, 16));
  m_logo_widget->setMovie(logo);
  m_logo_widget->setStyleSheet(QString(R"(
    QLabel {
      background-color: white;
      padding-top: %1px;
    })").arg(scale_height(8)));
  m_logo_widget->setAlignment(Qt::AlignHCenter);
  m_logo_widget->movie()->start();
  m_scroll_area->installEventFilter(this);
  m_scroll_area->widget()->installEventFilter(this);
  connect(m_scroll_area->horizontalScrollBar(), &QScrollBar::valueChanged, this,
    [=] (auto i) { on_scroll_bar_value_changed(); });
}

bool SnapshotLoadingWidget::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_scroll_area) {
    if(event->type() == QEvent::Resize) {
      auto e = static_cast<QResizeEvent*>(event);
      auto rect = m_scroll_area->widget()->visibleRegion().boundingRect();
      update_logo_geometry(rect.x(), e->size().width());
    }
  } else if(watched == m_scroll_area->widget()) {
    if(event->type() == QEvent::Resize) {
      setFixedWidth(m_scroll_area->widget()->width());
    }
  }
  return QWidget::eventFilter(watched, event);
}

void SnapshotLoadingWidget::showEvent(QShowEvent* event) {
  auto rect = m_scroll_area->widget()->visibleRegion().boundingRect();
  if(m_scroll_area->verticalScrollBar()->isVisible()) {
    rect.setWidth(rect.width() + m_scroll_area->verticalScrollBar()->width());
  }
  update_logo_geometry(rect.x(), rect.width());
}

void SnapshotLoadingWidget::update_logo_geometry(int x_pos, int width) {
  m_logo_widget->setGeometry(x_pos, 0, width, height());
}

void SnapshotLoadingWidget::on_scroll_bar_value_changed() {
  auto rect = m_scroll_area->widget()->visibleRegion().boundingRect();
  update_logo_geometry(rect.x(), rect.width());
}
