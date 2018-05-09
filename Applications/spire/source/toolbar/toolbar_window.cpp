#include "spire/toolbar/toolbar_window.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/icon_button.hpp"
#include "spire/ui/window.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::signals2;
using namespace spire;

toolbar_window::toolbar_window(recently_closed_model& model,
    const DirectoryEntry& account, QWidget* parent)
    : QWidget(parent),
      m_model(&model) {
  setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint |
    Qt::WindowCloseButtonHint);
  setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
  m_body = new QWidget(this);
  m_body->setFixedSize(scale(308, 72));
  auto window_layout = new QHBoxLayout(this);
  window_layout->setContentsMargins({});
  m_window = new spire::window(m_body, this);
  m_window->set_icon(imageFromSvg(":/icons/spire-icon-black.svg", scale(26, 26),
    QRect(translate(8, 8), scale(10, 10))),
    imageFromSvg(":/icons/spire-icon-grey.svg", scale(26, 26),
    QRect(translate(8, 8), scale(10, 10))));
  window_layout->addWidget(m_window);
  m_body->setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QVBoxLayout(m_body);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  layout->addStretch(8);
  window()->setWindowTitle(tr("Spire - Signed in as ") +
    QString::fromStdString(account.m_name));
  auto combo_box_layout = new QHBoxLayout();
  combo_box_layout->setContentsMargins({});
  combo_box_layout->setSpacing(0);
  layout->addLayout(combo_box_layout);
  layout->setStretchFactor(combo_box_layout, 26);
  layout->addStretch(10);
  combo_box_layout->addStretch(8);
  m_window_manager_button = new toolbar_menu(tr("Window Manager"), m_body);
  m_window_manager_button->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_window_manager_button->add(tr("Minimize All"));
  m_window_manager_button->add(tr("Restore All"));
  m_window_manager_button->add(tr("Import/Export Settings"));
  combo_box_layout->addWidget(m_window_manager_button);
  combo_box_layout->setStretchFactor(m_window_manager_button, 138);
  combo_box_layout->addStretch(16);
  m_recently_closed_button = new toolbar_menu(tr("Recently Closed"), m_body);
  m_recently_closed_button->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_recently_closed_button->connect_item_selected_signal(
    [=] (auto index) {on_item_selected(index);});
  combo_box_layout->addWidget(m_recently_closed_button);
  combo_box_layout->setStretchFactor(m_recently_closed_button, 138);
  combo_box_layout->addStretch(8);
  auto button_layout = new QHBoxLayout();
  button_layout->setContentsMargins({});
  button_layout->setSpacing(0);
  auto window_button_size = scale(20, 20);
  m_account_button = new icon_button(
    imageFromSvg(":/icons/account-light-purple.svg", window_button_size),
    imageFromSvg(":/icons/account-purple.svg", window_button_size), m_body);
  m_account_button->setToolTip(tr("Account"));
  button_layout->addWidget(m_account_button);
  m_key_bindings_button = new icon_button(
    imageFromSvg(":/icons/key-bindings-light-purple.svg", window_button_size),
    imageFromSvg(":/icons/key-bindings-purple.svg", window_button_size),
    m_body);
  m_key_bindings_button->setToolTip(tr("Key Bindings"));
  button_layout->addWidget(m_key_bindings_button);
  m_canvas_button = new icon_button(
    imageFromSvg(":/icons/canvas-light-purple.svg", window_button_size),
    imageFromSvg(":/icons/canvas-purple.svg", window_button_size), m_body);
  m_canvas_button->setToolTip(tr("Canvas"));
  button_layout->addWidget(m_canvas_button);
  m_book_view_button = new icon_button(
    imageFromSvg(":/icons/bookview-light-purple.svg", window_button_size),
    imageFromSvg(":/icons/bookview-purple.svg", window_button_size), m_body);
  m_book_view_button->setToolTip(tr("Book View"));
  button_layout->addWidget(m_book_view_button);
  m_time_and_sales_button = new icon_button(
    imageFromSvg(":/icons/time-sale-light-purple.svg", window_button_size),
    imageFromSvg(":/icons/time-sale-purple.svg", window_button_size), m_body);
  m_time_and_sales_button->setToolTip(tr("Time and Sales"));
  m_time_and_sales_button->connect_clicked_signal(
    [=] { on_open_window(recently_closed_model::type::TIME_AND_SALE); });
  button_layout->addWidget(m_time_and_sales_button);
  m_chart_button = new icon_button(
    imageFromSvg(":/icons/chart-light-purple.svg", window_button_size),
    imageFromSvg(":/icons/chart-purple.svg", window_button_size), m_body);
  m_chart_button->setToolTip(tr("Chart"));
  button_layout->addWidget(m_chart_button);
  m_dashboard_button = new icon_button(
    imageFromSvg(":/icons/dashboard-light-purple.svg", window_button_size),
    imageFromSvg(":/icons/dashboard-purple.svg", window_button_size), m_body);
  m_dashboard_button->setToolTip(tr("Dashboard"));
  button_layout->addWidget(m_dashboard_button);
  m_order_imbalances_button = new icon_button(
    imageFromSvg(":/icons/order-imbalances-light-purple.svg",
      window_button_size),
    imageFromSvg(":/icons/order-imbalances-purple.svg", window_button_size),
    m_body);
  m_order_imbalances_button->setToolTip(tr("Order Imbalances"));
  button_layout->addWidget(m_order_imbalances_button);
  m_blotter_button = new icon_button(
    imageFromSvg(":/icons/blotter-light-purple.svg", window_button_size),
    imageFromSvg(":/icons/blotter-purple.svg", window_button_size), m_body);
  m_blotter_button->setToolTip(tr("Blotter"));
  button_layout->addWidget(m_blotter_button);
  layout->addLayout(button_layout);
  layout->setStretchFactor(button_layout, 20);
  layout->addStretch(8);
  m_model->connect_entry_added_signal([=] (auto& e) {entry_added(e);});
  m_model->connect_entry_removed_signal([=] (auto e) {entry_removed(e);});
}

connection toolbar_window::connect_open_signal(
    const open_signal::slot_type& slot) const {
  return m_open_signal.connect(slot);
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
    event->accept();
  } else {
    QWidget::keyPressEvent(event);
  }
}

void toolbar_window::entry_added(const recently_closed_model::entry& e) {
  auto icon_size = scale(26, 20);
  auto icon_rect = QRect(translate(8, 5), scale(10, 10));
  m_entries.push_back(e);
  switch(e.m_type) {
    case recently_closed_model::type::BOOK_VIEW: {
      m_recently_closed_button->add(e.m_identifier.c_str(),
        imageFromSvg(QString(":/icons/bookview-black.svg"), icon_size,
          icon_rect));
      break;
    }
    case recently_closed_model::type::TIME_AND_SALE: {
      m_recently_closed_button->add(e.m_identifier.c_str(),
        imageFromSvg(QString(":/icons/time-sale-black.svg"), icon_size,
          icon_rect));
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

void toolbar_window::on_open_window(recently_closed_model::type w) {
  m_open_signal(w);
}
