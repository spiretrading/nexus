#include "Spire/Ui/TransitionView.hpp"
#include <QMovie>
#include <QTimer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

TransitionView::TransitionView(QWidget* component, QWidget* parent)
    : QWidget(parent),
      m_component(component),
      m_timer(new QTimer(this)),
      m_status(Status::NONE) {
  make_vbox_layout(this);
  make_pending_widget();
  m_timer->setSingleShot(true);
  connect(m_timer, &QTimer::timeout,
    std::bind_front(&TransitionView::on_timer_expired, this));
}

void TransitionView::set_status(Status status) {
  if(m_status == status) {
    return;
  }
  m_status = status;
  if(m_status == Status::NONE) {
    m_timer->stop();
    clear_layout();
    stop_playing_spinner();
  } else if(m_status == Status::READY) {
    m_timer->stop();
    clear_layout();
    stop_playing_spinner();
    add_widget_to_layout(*m_component);
  } else if(m_status == Status::LOADING) {
    m_timer->start(2000);
  }
}

void TransitionView::add_widget_to_layout(QWidget& widget) {
  layout()->addWidget(&widget);
  widget.show();
}

void TransitionView::clear_layout() {
  auto item = layout()->takeAt(0);
  if(item) {
    item->widget()->hide();
  }
}

void TransitionView::make_pending_widget() {
  m_pending_widget = new QWidget(this);
  auto pending_layout = make_vbox_layout(m_pending_widget);
  pending_layout->addStretch(73);
  auto pending_middle_layout = make_hbox_layout(m_pending_widget);
  pending_middle_layout->addStretch();
  m_spinner = new QMovie(":/Icons/spinner.gif", QByteArray());
  m_spinner->setScaledSize(scale(44, 44));
  auto spinner_widget = new QLabel();
  spinner_widget->setMovie(m_spinner);
  auto spinner_container = new Box(spinner_widget);
  update_style(*spinner_container, [&] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  pending_middle_layout->addWidget(spinner_container);
  pending_middle_layout->addStretch();
  pending_layout->addLayout(pending_middle_layout);
  pending_layout->addStretch(145);
  m_pending_widget->hide();
}

void TransitionView::on_timer_expired() {
  clear_layout();
  m_spinner->start();
  add_widget_to_layout(*m_pending_widget);
}

void TransitionView::stop_playing_spinner() {
  if(m_spinner->state() != QMovie::NotRunning) {
    m_spinner->stop();
  }
}
