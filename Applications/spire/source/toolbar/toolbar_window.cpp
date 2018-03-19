#include "spire/toolbar/toolbar_window.hpp"
#include <QAction>
#include <QEvent>
#include <QGraphicsDropshadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QString>
#include <QStyledItemDelegate>
#include <QtSvg/QSvgRenderer>
#include <QVBoxlayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/icon_button.hpp"
#include "spire/ui/window.hpp"

using std::vector;
using namespace boost;
using namespace boost::signals2;
using namespace spire;

toolbar_window::toolbar_window(recently_closed_model& model, QWidget* parent)
    : QWidget(parent),
      m_model(&model),
      m_is_dragging(false) {
  m_model->connect_entry_added_signal([&] (auto& e) { entry_added(e); });
  m_model->connect_entry_removed_signal([&] (auto e) { entry_removed(e); });
  setContentsMargins(0, 0, 0, 0);
  setFixedSize(scale(308, 98));
  setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);
  auto title_bar_layout = new QHBoxLayout();
  title_bar_layout->setMargin(0);
  title_bar_layout->setSpacing(0);
  layout->addLayout(title_bar_layout);
  auto renderer = new QSvgRenderer(QString(":/icons/spire-icon.svg"), this);
  auto spire_icon = QImage(scale(26, 26), QImage::Format_ARGB32);
  spire_icon.fill(QColor(0, 0, 0, 0));
  QPainter p1(&spire_icon);
  auto icon_rect = QRectF(scale_width(8), scale_height(8), scale_width(10),
    scale_height(10));
  renderer->render(&p1, icon_rect);
  auto title_bar_logo = new QLabel(this);
  title_bar_logo->setFixedSize(scale(26, 26));
  title_bar_logo->setPixmap(QPixmap::fromImage(spire_icon));
  title_bar_layout->addWidget(title_bar_logo);
  auto username_label = new QLabel(tr("Spire - Signed in as Username"));
  username_label->setStyleSheet(QString(
    R"(font-family: Roboto;
       font-size: %1px;)").arg(scale_height(12)));
  username_label->setFixedSize(scale(218, 26));
  title_bar_layout->addWidget(username_label);
  m_minimize_button = new icon_button(":/icons/minimize-grey.svg",
    ":/icons/minimize-black.svg", scale_width(32), scale_height(26),
    QRectF(scale_width(11), scale_height(12),
           scale_width(10), scale_height(2)),
    this);
  m_minimize_button->setStyleSheet(":hover { background-color: #EBEBEB; }");
  m_minimize_button->connect_clicked_signal(
    [&] { window()->showMinimized(); });
  title_bar_layout->addWidget(m_minimize_button);
  m_close_button = new icon_button(":/icons/close-grey.svg",
    ":/icons/close-red.svg", scale_width(32), scale_height(26),
    QRectF(scale_width(11), scale_height(8),
           scale_width(10),scale_height(10)),
    this);
  m_close_button->setStyleSheet(":hover { background-color: #EBEBEB; }");
  m_close_button->connect_clicked_signal([&] { window()->close(); });
  title_bar_layout->addWidget(m_close_button);
  auto input_layout = new QVBoxLayout();
  input_layout->setMargin(0);
  input_layout->setSpacing(0);
  layout->addLayout(input_layout);
  auto combobox_layout = new QHBoxLayout();
  combobox_layout->setContentsMargins(scale_width(8), scale_height(8),
    scale_width(8), scale_height(5));
  combobox_layout->setSpacing(0);
  input_layout->addLayout(combobox_layout);
  m_window_manager_button = new toolbar_menu(tr("Window Manager"), this);
  m_window_manager_button->setFixedSize(scale(138, 26));
  m_window_manager_button->add(tr("Minimize All"));
  m_window_manager_button->add(tr("Restore All"));
  m_window_manager_button->add(tr("Import/Export Settings"));
  combobox_layout->addWidget(m_window_manager_button);
  combobox_layout->addStretch(1);
  m_recently_closed_button = new toolbar_menu(tr("Recently Closed"), this);
  m_recently_closed_button->connect_item_selected_signal(
    [&] (auto index) { on_item_selected(index); });
  m_recently_closed_button->setFixedSize(scale(138, 26));
  combobox_layout->addWidget(m_recently_closed_button);
  auto button_layout = new QHBoxLayout();
  button_layout->setContentsMargins(scale_width(8), scale_height(5),
    scale_width(8), scale_height(8));
  button_layout->setSpacing(scale_width(14));
  input_layout->addLayout(button_layout);
  m_account_button = new icon_button(":/icons/account-light-purple.svg",
    ":/icons/account-purple.svg", scale_width(20), scale_height(20), this);
  m_account_button->set_focusable(true);
  m_account_button->setToolTip(tr("Account"));
  button_layout->addWidget(m_account_button);
  m_key_bindings_button = new icon_button(
    ":/icons/key-bindings-light-purple.svg",
    ":/icons/key-bindings-purple.svg", scale_width(20),
    scale_height(20), this);
  m_key_bindings_button->set_focusable(true);
  m_key_bindings_button->setToolTip(tr("Key Bindings"));
  button_layout->addWidget(m_key_bindings_button);
  m_canvas_button = new icon_button(":/icons/canvas-light-purple.svg",
    ":/icons/canvas-purple.svg", scale_width(20), scale_height(20), this);
  m_canvas_button->set_focusable(true);
  m_canvas_button->setToolTip(tr("Canvas"));
  button_layout->addWidget(m_canvas_button);
  m_book_view_button = new icon_button(":/icons/bookview-light-purple.svg",
    ":/icons/bookview-purple.svg", scale_width(20), scale_height(20), this);
  m_book_view_button->set_focusable(true);
  m_book_view_button->setToolTip(tr("Bookview"));
  button_layout->addWidget(m_book_view_button);
  m_time_sale_button = new icon_button(":/icons/time-sale-light-purple.svg",
    ":/icons/time-sale-purple.svg", scale_width(20), scale_height(20), this);
  m_time_sale_button->set_focusable(true);
  m_time_sale_button->setToolTip(tr("Time and Sale"));
  button_layout->addWidget(m_time_sale_button);
  m_chart_button = new icon_button(":/icons/chart-light-purple.svg",
    ":/icons/chart-purple.svg", scale_width(20), scale_height(20), this);
  m_chart_button->set_focusable(true);
  m_chart_button->setToolTip(tr("Chart"));
  button_layout->addWidget(m_chart_button);
  m_dashboard_button = new icon_button(":/icons/dashboard-light-purple.svg",
    ":/icons/dashboard-purple.svg", scale_width(20), scale_height(20), this);
  m_dashboard_button->set_focusable(true);
  m_dashboard_button->setToolTip(tr("Dashboard"));
  button_layout->addWidget(m_dashboard_button);
  m_order_imbalances_button = new icon_button(
    ":/icons/order-imbalances-light-purple.svg",
    ":/icons/order-imbalances-purple.svg", scale_width(20), scale_height(20),
    this);
  m_order_imbalances_button->set_focusable(true);
  m_order_imbalances_button->setToolTip(tr("Order Imbalances"));
  button_layout->addWidget(m_order_imbalances_button);
  m_blotter_button = new icon_button(":/icons/blotter-light-purple.svg",
    ":/icons/blotter-purple.svg", scale_width(20), scale_height(20), this);
  m_blotter_button->set_focusable(true);
  m_blotter_button->setToolTip(tr("Blotter"));
  button_layout->addWidget(m_blotter_button);
}

connection toolbar_window::connect_closed_signal(
    const closed_signal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

connection toolbar_window::connect_reopen_signal(
    const reopen_signal::slot_type& slot) const {
  return m_reopen_signal.connect(slot);
}

void toolbar_window::closeEvent(QCloseEvent* event) {
  m_closed_signal();
}

void toolbar_window::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    if(m_window_manager_button->hasFocus()) {
      m_window_manager_button->showMenu();
    } else if(m_recently_closed_button->hasFocus()) {
      m_recently_closed_button->showMenu();
      m_recently_closed_button->menu()->setFocus();
    }
  } else if(event->key() == Qt::Key_Left) {
    focusPreviousChild();
  } else if(event->key() == Qt::Key_Right) {
    focusNextChild();
  }
}

void toolbar_window::mouseMoveEvent(QMouseEvent* event) {
  if(!m_is_dragging) {
    return;
  }
  auto delta = event->globalPos();
  delta -= m_last_pos;
  auto window_pos = window()->pos();
  window_pos += delta;
  m_last_pos = event->globalPos();
  window()->move(window_pos);
}

void toolbar_window::mousePressEvent(QMouseEvent* event) {
  if(m_is_dragging || event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = true;
  m_last_pos = event->globalPos();
  setFocus();
}

void toolbar_window::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = false;
}

void toolbar_window::entry_added(const recently_closed_model::entry& e) {
  m_entries.push_back(e);
  switch(e.m_type) {
    case recently_closed_model::type::BOOK_VIEW: {
      m_recently_closed_button->add(e.m_identifier.c_str(),
        ":/icons/bookview-black.svg");
      break;
    }
    case recently_closed_model::type::TIME_AND_SALE: {
      m_recently_closed_button->add(e.m_identifier.c_str(),
        ":/icons/time-sale-black.svg");
      break;
    }
  }
}

void toolbar_window::entry_removed(const recently_closed_model::entry& e) {
  for(auto i = 0; i < static_cast<int>(m_entries.size()); ++i) {
    if(m_entries[i].m_id == e.m_id) {
      m_entries.erase(m_entries.begin() + i);
      m_recently_closed_button->remove(i);
    }
  }
}

void toolbar_window::on_item_selected(int index) {
  m_reopen_signal(m_entries[index]);
}
