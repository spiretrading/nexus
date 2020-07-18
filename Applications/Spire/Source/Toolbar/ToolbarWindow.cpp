#include "Spire/Toolbar/ToolbarWindow.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Toolbar/ToolbarMenu.hpp"
#include "Spire/Ui/IconButton.hpp"
#include "Spire/Ui/Window.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  auto ICON_SIZE() {
    static auto icon_size = scale(10, 10);
    return icon_size;
  }

  auto ICON_RECT() {
    static auto icon_rect = QRect(QPoint(0, 0), scale(10, 10));
    return icon_rect;
  }
}

ToolbarWindow::ToolbarWindow(Ref<RecentlyClosedModel> model,
    const DirectoryEntry& account, QWidget* parent)
    : Window(parent),
      m_model(model.Get()) {
  set_fixed_body_size(scale(308, 72));
  set_svg_icon(":/Icons/spire-icon-black.svg",
    ":/Icons/spire-icon-grey.svg");
  setWindowIcon(QIcon(":/Icons/spire-icon-256x256.png"));
  setWindowTitle(tr("Spire - Signed in as ") +
    QString::fromStdString(account.m_name));
  auto body = new QWidget(this);
  body->setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QVBoxLayout(body);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  layout->addStretch(8);
  auto combo_box_layout = new QHBoxLayout();
  combo_box_layout->setContentsMargins({});
  combo_box_layout->setSpacing(0);
  layout->addLayout(combo_box_layout);
  layout->setStretchFactor(combo_box_layout, 26);
  layout->addStretch(10);
  combo_box_layout->addStretch(8);
  m_window_manager_button = new StaticDropDownMenu({tr("Minimize All"),
    tr("Restore All"), tr("Import/Export Settings")}, tr("Window Manager"),
    body);
  m_window_manager_button->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  combo_box_layout->addWidget(m_window_manager_button);
  combo_box_layout->setStretchFactor(m_window_manager_button, 138);
  combo_box_layout->addStretch(16);
  m_recently_closed_button = new ToolbarMenu(tr("Recently Closed"), body);
  m_recently_closed_button->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_recently_closed_button->connect_item_selected_signal(
    [=] (auto index) { on_item_selected(index); });
  combo_box_layout->addWidget(m_recently_closed_button);
  combo_box_layout->setStretchFactor(m_recently_closed_button, 138);
  combo_box_layout->addStretch(8);
  auto button_layout = new QHBoxLayout();
  button_layout->setContentsMargins({});
  button_layout->setSpacing(0);
  auto window_button_size = scale(20, 20);
  m_account_button = new IconButton(
    imageFromSvg(":/Icons/account-light-purple.svg", window_button_size),
    imageFromSvg(":/Icons/account-purple.svg", window_button_size), body);
  m_account_button->setToolTip(tr("Account"));
  button_layout->addWidget(m_account_button);
  m_key_bindings_button = new IconButton(
    imageFromSvg(":/Icons/key-bindings-light-purple.svg", window_button_size),
    imageFromSvg(":/Icons/key-bindings-purple.svg", window_button_size), body);
  m_key_bindings_button->setToolTip(tr("Key Bindings"));
  button_layout->addWidget(m_key_bindings_button);
  m_canvas_button = new IconButton(
    imageFromSvg(":/Icons/canvas-light-purple.svg", window_button_size),
    imageFromSvg(":/Icons/canvas-purple.svg", window_button_size), body);
  m_canvas_button->setToolTip(tr("Canvas"));
  button_layout->addWidget(m_canvas_button);
  m_book_view_button = new IconButton(
    imageFromSvg(":/Icons/bookview-light-purple.svg", window_button_size),
    imageFromSvg(":/Icons/bookview-purple.svg", window_button_size), body);
  m_book_view_button->setToolTip(tr("Book View"));
  m_book_view_button->connect_clicked_signal(
    [=] { on_open_window(RecentlyClosedModel::Type::BOOK_VIEW); });
  button_layout->addWidget(m_book_view_button);
  m_time_and_sales_button = new IconButton(
    imageFromSvg(":/Icons/time-sale-light-purple.svg", window_button_size),
    imageFromSvg(":/Icons/time-sale-purple.svg", window_button_size), body);
  m_time_and_sales_button->setToolTip(tr("Time and Sales"));
  m_time_and_sales_button->connect_clicked_signal(
    [=] { on_open_window(RecentlyClosedModel::Type::TIME_AND_SALE); });
  button_layout->addWidget(m_time_and_sales_button);
  m_chart_button = new IconButton(
    imageFromSvg(":/Icons/chart-light-purple.svg", window_button_size),
    imageFromSvg(":/Icons/chart-purple.svg", window_button_size), body);
  m_chart_button->setToolTip(tr("Chart"));
  button_layout->addWidget(m_chart_button);
  m_dashboard_button = new IconButton(
    imageFromSvg(":/Icons/dashboard-light-purple.svg", window_button_size),
    imageFromSvg(":/Icons/dashboard-purple.svg", window_button_size), body);
  m_dashboard_button->setToolTip(tr("Dashboard"));
  button_layout->addWidget(m_dashboard_button);
  m_order_imbalances_button = new IconButton(
    imageFromSvg(":/Icons/order-imbalances-light-purple.svg",
      window_button_size),
    imageFromSvg(":/Icons/order-imbalances-purple.svg", window_button_size),
    body);
  m_order_imbalances_button->setToolTip(tr("Order Imbalances"));
  button_layout->addWidget(m_order_imbalances_button);
  m_blotter_button = new IconButton(
    imageFromSvg(":/Icons/blotter-light-purple.svg", window_button_size),
    imageFromSvg(":/Icons/blotter-purple.svg", window_button_size), body);
  m_blotter_button->setToolTip(tr("Blotter"));
  button_layout->addWidget(m_blotter_button);
  layout->addLayout(button_layout);
  layout->setStretchFactor(button_layout, 20);
  layout->addStretch(8);
  Window::layout()->addWidget(body);
  m_entry_added_connection = m_model->connect_entry_added_signal(
    [=] (auto& e) {entry_added(e);});
  m_entry_removed_connection = m_model->connect_entry_removed_signal(
    [=] (auto e) {entry_removed(e);});
}

connection ToolbarWindow::connect_open_signal(
    const OpenSignal::slot_type& slot) const {
  return m_open_signal.connect(slot);
}

connection ToolbarWindow::connect_reopen_signal(
    const ReopenSignal::slot_type& slot) const {
  return m_reopen_signal.connect(slot);
}

void ToolbarWindow::entry_added(const RecentlyClosedModel::Entry& e) {
  m_entries.push_back(e);
  switch(e.m_type) {
    case RecentlyClosedModel::Type::BOOK_VIEW: {
      m_recently_closed_button->add(QString::fromStdString(e.m_identifier),
        imageFromSvg(QString(":/Icons/bookview-black.svg"), ICON_SIZE(),
          ICON_RECT()));
      break;
    }
    case RecentlyClosedModel::Type::TIME_AND_SALE: {
      m_recently_closed_button->add(QString::fromStdString(e.m_identifier),
        imageFromSvg(QString(":/Icons/time-sale-black.svg"), ICON_SIZE(),
          ICON_RECT()));
      break;
    }
  }
}

void ToolbarWindow::entry_removed(const RecentlyClosedModel::Entry& e) {
  for(auto i = 0; i < static_cast<int>(m_entries.size()); ++i) {
    if(m_entries[i].m_id == e.m_id) {
      m_entries.erase(m_entries.begin() + i);
      m_recently_closed_button->remove(i);
    }
  }
}

void ToolbarWindow::on_item_selected(int index) {
  m_reopen_signal(m_entries[index]);
}

void ToolbarWindow::on_open_window(RecentlyClosedModel::Type w) {
  m_open_signal(w);
}
