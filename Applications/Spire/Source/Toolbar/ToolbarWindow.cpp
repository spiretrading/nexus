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
  auto BUTTON_SIZE() {
    static auto size = scale(26, 26);
    return size;
  }

  auto DROP_DOWN_SIZE() {
    static auto size = scale(138, 26);
    return size;
  }

  auto ICON_SIZE() {
    static auto icon_size = scale(10, 10);
    return icon_size;
  }

  auto ICON_RECT() {
    static auto icon_rect = QRect(QPoint(0, 0), scale(10, 10));
    return icon_rect;
  }

  auto create_button(const QString& icon, const QString& tooltip,
      QWidget* parent) {
    auto style = IconButton::Style();
    style.m_default_color = QColor("#7F5EEC");
    style.m_blur_color = QColor("#7F5EEC");
    auto button = new IconButton(imageFromSvg(icon, BUTTON_SIZE()), style,
      parent);
    button->setFixedSize(BUTTON_SIZE());
    button->setToolTip(tooltip);
    return button;
  }
}

ToolbarWindow::ToolbarWindow(Ref<RecentlyClosedModel> model,
    const DirectoryEntry& account, QWidget* parent)
    : Window(parent),
      m_model(model.Get()) {
  set_svg_icon(":/Icons/spire.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/spire.png"));
  setWindowTitle(tr("Spire - Signed in as ") +
    QString::fromStdString(account.m_name));
  auto body = new QWidget(this);
  body->setObjectName("toolbar_window_body");
  body->setStyleSheet("#toolbar_window_body { background-color: #F5F5F5; }");
  body->setFixedSize(scale(308, 76));
  auto layout = new QVBoxLayout(body);
  layout->setContentsMargins(scale_width(8), scale_height(8), scale_width(8),
    scale_height(8));
  layout->setSpacing(scale_height(10));
  auto combo_box_layout = new QHBoxLayout();
  combo_box_layout->setContentsMargins({});
  combo_box_layout->setSpacing(scale_width(16));
  layout->addLayout(combo_box_layout);
  m_window_manager_button = new StaticDropDownMenu({tr("Minimize All"),
    tr("Restore All"), tr("Import/Export Settings")}, tr("Window Manager"),
    body);
  m_window_manager_button->setFixedSize(DROP_DOWN_SIZE());
  m_window_manager_button->set_next_activated(false);
  combo_box_layout->addWidget(m_window_manager_button);
  m_recently_closed_button = new ToolbarMenu(tr("Recently Closed"), body);
  m_recently_closed_button->setFixedSize(DROP_DOWN_SIZE());
  m_recently_closed_button->connect_index_selected_signal(
    [=] (auto index) { on_item_selected(index); });
  combo_box_layout->addWidget(m_recently_closed_button);
  auto button_layout = new QHBoxLayout();
  button_layout->setContentsMargins({});
  button_layout->setSpacing(scale_width(7));
  m_account_button = create_button(":/Icons/toolbar_icons/account.svg",
    tr("Account"), body);
  button_layout->addWidget(m_account_button);
  m_key_bindings_button = create_button(
    ":/Icons/toolbar_icons/key-bindings.svg", tr("Key Bindings"), body);
  button_layout->addWidget(m_key_bindings_button);
  m_canvas_button = create_button(":/Icons/toolbar_icons/canvas.svg",
    tr("Canvas"), body);
  button_layout->addWidget(m_canvas_button);
  m_book_view_button = create_button(":/Icons/toolbar_icons/bookview.svg",
    tr("Book View"), body);
  connect(m_book_view_button, &IconButton::clicked,
    [=] { on_open_window(RecentlyClosedModel::Type::BOOK_VIEW); });
  button_layout->addWidget(m_book_view_button);
  m_time_and_sales_button = create_button(
    ":/Icons/toolbar_icons/time-sales.svg", tr("Time and Sales"), body);
  connect(m_time_and_sales_button, &IconButton::clicked,
    [=] { on_open_window(RecentlyClosedModel::Type::TIME_AND_SALE); });
  button_layout->addWidget(m_time_and_sales_button);
  m_chart_button = create_button(":/Icons/toolbar_icons/chart.svg",
    tr("Chart"), body);
  button_layout->addWidget(m_chart_button);
  m_dashboard_button = create_button(":/Icons/toolbar_icons/dashboard.svg",
    tr("Dashboard"), body);
  button_layout->addWidget(m_dashboard_button);
  m_order_imbalances_button = create_button(
    ":/Icons/toolbar_icons/imbalance-indicator.svg", tr("Order Imbalances"),
    body);
  button_layout->addSpacing(scale_width(1));
  button_layout->addWidget(m_order_imbalances_button);
  m_blotter_button = create_button(":/Icons/toolbar_icons/blotter.svg",
    tr("Blotter"), body);
  button_layout->addSpacing(scale_width(1));
  button_layout->addWidget(m_blotter_button);
  layout->addLayout(button_layout);
  set_body(body);
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
        imageFromSvg(QString(":/Icons/bookview.svg"), ICON_SIZE(),
          ICON_RECT()));
      break;
    }
    case RecentlyClosedModel::Type::TIME_AND_SALE: {
      m_recently_closed_button->add(QString::fromStdString(e.m_identifier),
        imageFromSvg(QString(":/Icons/time-sales.svg"), ICON_SIZE(),
          ICON_RECT()));
      break;
    }
  }
}

void ToolbarWindow::entry_removed(const RecentlyClosedModel::Entry& e) {
  for(auto i = 0; i < static_cast<int>(m_entries.size()); ++i) {
    if(m_entries[i].m_id == e.m_id) {
      m_entries.erase(m_entries.begin() + i);
      m_recently_closed_button->remove_item(i);
    }
  }
}

void ToolbarWindow::on_item_selected(int index) {
  m_reopen_signal(m_entries[index]);
}

void ToolbarWindow::on_open_window(RecentlyClosedModel::Type w) {
  m_open_signal(w);
}
