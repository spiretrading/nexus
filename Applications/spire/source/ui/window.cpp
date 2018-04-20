#include "spire/ui/window.hpp"
#include <QEvent>
#include <QResizeEvent>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/drop_shadow.hpp"
#include "spire/ui/title_bar.hpp"

using namespace spire;

window::window(QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body) {
  this->::QWidget::window()->setWindowFlags(
    this->::QWidget::window()->windowFlags() | Qt::Window |
    Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  this->::QWidget::window()->setAttribute(Qt::WA_TranslucentBackground);
  m_shadow = std::make_unique<drop_shadow>(this);
  resize(m_body->width() + scale_width(25),
    m_body->height() + scale_height(25));
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_border = new QWidget(this);
  m_border->setObjectName("window_border");
  m_border->resize(m_body->size() + scale(1, 1));
  set_border_stylesheet("#A0A0A0");
  layout->addWidget(m_border);
  auto border_layout = new QVBoxLayout(m_border);
  border_layout->setMargin(scale_width(1));
  border_layout->setSpacing(0);
  m_title_bar = new title_bar(m_border);
  border_layout->addWidget(m_title_bar);
  border_layout->addWidget(m_body);
  this->::QWidget::window()->installEventFilter(this);
}

void window::set_icon(const QImage& icon) {
  m_title_bar->set_icon(icon);
}

void window::set_icon(const QImage& icon, const QImage& unfocused_icon) {
  m_title_bar->set_icon(icon, unfocused_icon);
}

bool window::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::WindowActivate) {
    set_border_stylesheet("#A0A0A0");
  } else if(event->type() == QEvent::WindowDeactivate) {
    set_border_stylesheet("#C8C8C8");
  } else if(event->type() == QEvent::Resize) {
    auto e = static_cast<QResizeEvent*>(event);
    if(e->size().height() > height()) {
      this->::QWidget::window()->resize(size());
    }
  }
  return QWidget::eventFilter(watched, event);
}

void window::set_border_stylesheet(const QColor& color) {
  m_border->setStyleSheet(QString(R"(
    #window_border {
      border: %1px solid %3 %2px solid %3;
    })").arg(scale_height(1)).arg(scale_width(1)).arg(color.name()));
}
