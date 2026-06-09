#include "Spire/TimeAndSales/PullIndicator.hpp"
#include <QLabel>
#include <QMovie>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

int PullIndicator::TABLE_BODY_BOTTOM_PADDING() {
  static auto height = scale_height(44);
  return height;
}

PullIndicator::PullIndicator(TableView& table_view)
    : QWidget(&table_view.get_body()),
      m_table_view(&table_view),
      m_timer(this),
      m_is_loading(false),
      m_is_full_reload(false),
      m_last_position(0) {
  m_spinner = new QMovie(":/Icons/spinner.gif");
  m_spinner->setScaledSize(scale(16, 16));
  auto spinner_widget = new QLabel();
  spinner_widget->setMovie(m_spinner);
  auto box = new Box(spinner_widget);
  enclose(*this, *box);
  proxy_style(*this, *box);
  update_style(*this, [] (auto& style) {
    style.get(Any()).
      set(BodyAlign(Qt::AlignHCenter)).
      set(horizontal_padding(scale_width(8))).
      set(PaddingBottom(scale_height(20))).
      set(PaddingTop(scale_height(8)));
  });
  auto& scroll_box = m_table_view->get_scroll_box();
  scroll_box.installEventFilter(this);
  m_loading_panel = new QWidget(&scroll_box);
  m_loading_panel->setAttribute(Qt::WA_TransparentForMouseEvents);
  auto loading_layout = make_vbox_layout(m_loading_panel);
  loading_layout->addStretch(66);
  auto loading_middle_layout = make_hbox_layout();
  loading_middle_layout->addStretch();
  m_loading_spinner =
    new QMovie(":/Icons/spinner.gif", QByteArray(), m_loading_panel);
  m_loading_spinner->setScaledSize(scale(44, 44));
  auto loading_spinner_widget = new QLabel();
  loading_spinner_widget->setMovie(m_loading_spinner);
  auto loading_spinner_container = new Box(loading_spinner_widget);
  update_style(*loading_spinner_container, [] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  loading_middle_layout->addWidget(loading_spinner_container);
  loading_middle_layout->addStretch();
  loading_layout->addLayout(loading_middle_layout);
  loading_layout->addStretch(130);
  m_loading_panel->hide();
  scroll_box.get_vertical_scroll_bar().connect_position_signal(
    std::bind_front(&PullIndicator::on_position, this));
  auto table = std::static_pointer_cast<TimeAndSalesTableModel>(
    m_table_view->get_table());
  table->connect_begin_loading_signal(
    std::bind_front(&PullIndicator::on_begin_loading, this));
  table->connect_end_loading_signal(
    std::bind_front(&PullIndicator::on_end_loading, this));
  m_table_view->get_body().installEventFilter(this);
  hide();
  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout,
    std::bind_front(&PullIndicator::on_timeout, this));
}

bool PullIndicator::eventFilter(QObject* watched, QEvent* event) {
  if(watched == &m_table_view->get_scroll_box()) {
    if(event->type() == QEvent::KeyPress) {
      auto& key_event = *static_cast<QKeyEvent*>(event);
      if(key_event.key() == Qt::Key_PageUp ||
          key_event.key() == Qt::Key_PageDown) {
        key_event.accept();
        return QCoreApplication::sendEvent(m_table_view, &key_event);
      }
    } else if(event->type() == QEvent::Resize) {
      update_loading_panel();
    }
  } else if(event->type() == QEvent::Resize && isVisible()) {
    auto& resize_event = *static_cast<QResizeEvent*>(event);
    update_position(resize_event.size());
  }
  return QObject::eventFilter(watched, event);
}

void PullIndicator::update_loading_panel() {
  auto& scroll_box = m_table_view->get_scroll_box();
  m_loading_panel->setGeometry(0, 0, scroll_box.width(), scroll_box.height());
  if(m_loading_panel->isVisible()) {
    m_loading_panel->raise();
  }
}

void PullIndicator::update_position(const QSize& size) {
  setGeometry(0, size.height() - TABLE_BODY_BOTTOM_PADDING(),
    size.width(), TABLE_BODY_BOTTOM_PADDING());
}

void PullIndicator::display() {
  auto& scroll_bar = m_table_view->get_scroll_box().get_vertical_scroll_bar();
  auto& body = m_table_view->get_body();
  if(!isVisible() && scroll_bar.get_position() >=
      body.sizeHint().height() - scroll_bar.get_page_size()) {
    update_position(body.sizeHint());
    show();
  }
}

void PullIndicator::on_position(int position) {
  auto& scroll_box = m_table_view->get_scroll_box();
  auto& scroll_bar = scroll_box.get_vertical_scroll_bar();
  if(m_is_loading) {
    if(m_spinner->state() == QMovie::Running) {
      display();
    }
  } else if(position > m_last_position &&
      scroll_bar.get_range().m_end - position <
        scroll_bar.get_page_size() / 2) {
    auto table = std::static_pointer_cast<TimeAndSalesTableModel>(
      m_table_view->get_table());
    table->load_history(scroll_box.height() /
      m_table_view->get_body().estimate_scroll_line_height());
  }
  m_last_position = position;
}

void PullIndicator::on_begin_loading() {
  if(m_is_loading) {
    return;
  }
  m_is_loading = true;
  m_is_full_reload = m_table_view->get_table()->get_row_size() == 0;
  const auto PULL_DELAY_TIMEOUT_MS = 1000;
  m_timer.start(PULL_DELAY_TIMEOUT_MS);
}

void PullIndicator::on_end_loading() {
  m_is_loading = false;
  m_spinner->stop();
  m_loading_spinner->stop();
  m_timer.stop();
  hide();
  m_loading_panel->hide();
}

void PullIndicator::on_timeout() {
  if(!m_is_loading) {
    return;
  }
  if(m_is_full_reload) {
    update_loading_panel();
    m_loading_spinner->start();
    m_loading_panel->raise();
    m_loading_panel->show();
  } else {
    m_spinner->start();
    display();
  }
}
