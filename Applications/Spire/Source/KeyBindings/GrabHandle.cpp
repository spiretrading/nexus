#include "Spire/KeyBindings/GrabHandle.hpp"
#include <QEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;
using namespace Spire::Styles;

GrabHandle::GrabHandle(QWidget* parent)
    : QWidget(parent),
      m_hover_observer(*this) {
  auto icon = new Icon(
    imageFromSvg(":/Icons/dot-grid.svg", scale(6, 10)), this);
  icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  auto body = new QWidget();
  body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  enclose(*body, *icon, Qt::AlignCenter);
  auto box = new Box(body);
  box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  enclose(*this, *box);
  proxy_style(*this, *box);
  update_style(*this, [] (auto& style) {
    style.get(Any() > is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(Fill(QColor(0x535353)));
  });
  m_hover_observer.connect_state_signal(
    std::bind_front(&GrabHandle::on_hover, this));
}

void GrabHandle::mousePressEvent(QMouseEvent* event) {
  setCursor(Qt::ClosedHandCursor);
  QWidget::mousePressEvent(event);
}

void GrabHandle::mouseReleaseEvent(QMouseEvent* event) {
  setCursor(Qt::OpenHandCursor);
  QWidget::mousePressEvent(event);
}

void GrabHandle::on_hover(HoverObserver::State state) {
  if(state == HoverObserver::State::NONE) {
    unsetCursor();
  } else {
    setCursor(Qt::OpenHandCursor);
  }
}
