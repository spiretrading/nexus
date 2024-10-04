#include "Spire/Ui/TransitionView.hpp"
#include <QLabel>
#include <QMovie>
#include <QTimer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;
using namespace Spire::Styles;

TransitionView::TransitionView(QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body),
      m_pending_widget(nullptr),
      m_spinner(nullptr),
      m_timer(new QTimer(this)),
      m_status(Status::NONE) {
  m_body->setParent(this);
  m_body->hide();
  make_vbox_layout(this);
  m_timer->setSingleShot(true);
  connect(m_timer, &QTimer::timeout,
    std::bind_front(&TransitionView::on_timer_expired, this));
}

TransitionView::Status TransitionView::get_status() const {
  return m_status;
}

void TransitionView::set_status(Status status) {
  if(m_status == status) {
    return;
  }
  m_status = status;
  if(m_status == Status::LOADING) {
    m_timer->start(2000);
  } else {
    m_timer->stop();
    if(m_spinner && m_spinner->state() != QMovie::NotRunning) {
      m_spinner->stop();
    }
    clear_layout();
    if(m_status == Status::READY) {
      add_widget_to_layout(*m_body);
    }
  }
}

void TransitionView::set_body(QWidget& body) {
  auto old_body = m_body;
  m_body = &body;
  if(auto item =
      layout()->replaceWidget(old_body, m_body, Qt::FindDirectChildrenOnly)) {
    delete item;
  }
  delete old_body;
}

void TransitionView::add_widget_to_layout(QWidget& widget) {
  layout()->addWidget(&widget);
  widget.show();
}

void TransitionView::clear_layout() {
  if(auto item = layout()->takeAt(0)) {
    item->widget()->hide();
    delete item;
  }
}

void TransitionView::make_pending_widget() {
  if(m_pending_widget) {
    return;
  }
  m_pending_widget = new QWidget(this);
  auto pending_layout = make_vbox_layout(m_pending_widget);
  pending_layout->addStretch(66);
  auto pending_middle_layout = make_hbox_layout();
  pending_middle_layout->addStretch();
  m_spinner = new QMovie(":/Icons/spinner.gif", QByteArray());
  m_spinner->setScaledSize(scale(44, 44));
  auto spinner_widget = new QLabel();
  spinner_widget->setMovie(m_spinner);
  auto spinner_container = new Box(spinner_widget);
  update_style(*spinner_container, [] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  pending_middle_layout->addWidget(spinner_container);
  pending_middle_layout->addStretch();
  pending_layout->addLayout(pending_middle_layout);
  pending_layout->addStretch(130);
}

void TransitionView::on_timer_expired() {
  clear_layout();
  make_pending_widget();
  m_spinner->start();
  add_widget_to_layout(*m_pending_widget);
}
