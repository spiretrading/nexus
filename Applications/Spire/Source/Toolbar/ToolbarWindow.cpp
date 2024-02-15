#include "Spire/Toolbar/ToolbarWindow.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MenuButton.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

ToolbarWindow::ToolbarWindow(QString user_name, bool is_manager,
    std::shared_ptr<ListModel<WindowInfo>> recent_windows,
    std::shared_ptr<ListModel<QString>> pinned_blotters, QWidget* parent)
    : Window(parent),
      m_recent_windows(std::move(recent_windows)),
      m_pinned_blotters(std::move(pinned_blotters)) {
  setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
  set_svg_icon(":/Icons/spire.svg");
  setWindowIcon(QIcon(":/Icons/spire-icon-48x48.png"));
  setWindowTitle(QString("Spire - Signed in as %1").arg(user_name));
  auto top_layout = make_hbox_layout();
  top_layout->addWidget(make_window_manager_button());
  top_layout->addStretch();
  top_layout->addWidget(make_recently_closed_button());
  auto bottom_layout = make_hbox_layout();
  bottom_layout->setSpacing(scale_width(4));
  bottom_layout->addWidget(make_icon_tool_button(
    WindowType::CANVAS, ":/Icons/toolbar_icons/canvas.svg"));
  bottom_layout->addWidget(make_icon_tool_button(
    WindowType::BOOK_VIEW, ":/Icons/toolbar_icons/bookview.svg"));
  bottom_layout->addWidget(make_icon_tool_button(
    WindowType::TIME_AND_SALES, ":/Icons/toolbar_icons/time-sales.svg"));
  bottom_layout->addWidget(make_icon_tool_button(
    WindowType::CHART, ":/Icons/toolbar_icons/chart.svg"));
  bottom_layout->addWidget(make_icon_tool_button(
    WindowType::WATCHLIST, ":/Icons/toolbar_icons/watchlist.svg"));
  bottom_layout->addWidget(make_icon_tool_button(
    WindowType::IMBALANCE_INDICATOR,
    ":/Icons/toolbar_icons/imbalance-indicator.svg"));
  bottom_layout->addWidget(make_blotter_button());
  if(is_manager) {
    bottom_layout->addWidget(make_icon_tool_button(
      WindowType::PORTFOLIO_VIEWER, ":/Icons/toolbar_icons/portfolio.svg"));
  }
  bottom_layout->addSpacing(scale_width(4));
  auto separator = new Box();
  separator->setFixedSize(scale(1, 16));
  update_style(*separator, [] (auto& styles) {
    styles.get(Any()).
      set(BackgroundColor(QColor(0xC8C8C8)));
  });
  bottom_layout->addWidget(separator);
  bottom_layout->addSpacing(scale_width(4));
  bottom_layout->addWidget(make_icon_tool_button(WindowType::KEY_BINDINGS,
    ":/Icons/toolbar_icons/key-bindings.svg"));
  bottom_layout->addWidget(make_icon_tool_button(WindowType::PROFILE,
    ":/Icons/toolbar_icons/profile.svg", tr("Spire Web Portal")));
  auto body = new QWidget();
  auto body_layout = make_vbox_layout(body);
  body_layout->addLayout(top_layout);
  body_layout->addSpacing(scale_height(8));
  body_layout->addLayout(bottom_layout);
  auto content = new Box(body);
  content->setFixedHeight(scale_height(72));
  update_style(*content, [] (auto& styles) {
    styles.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(horizontal_padding(scale_width(4))).
      set(PaddingTop(scale_width(8))).
      set(PaddingBottom(scale_width(4)));
  });
  set_body(content);
}

const std::shared_ptr<ListModel<ToolbarWindow::WindowInfo>>&
    ToolbarWindow::get_recent_windows() const {
  return m_recent_windows;
}

const std::shared_ptr<ListModel<QString>>&
    ToolbarWindow::get_pinned_blotters() const {
  return m_pinned_blotters;
}

connection ToolbarWindow::connect_open_signal(
    const OpenSignal::slot_type& slot) const {
  return m_open_signal.connect(slot);
}

connection ToolbarWindow::connect_minimize_all_signal(
    const MinimizeAllSignal::slot_type& slot) const {
  return m_minimize_all_signal.connect(slot);
}

connection ToolbarWindow::connect_restore_all_signal(
    const RestoreAllSignal::slot_type& slot) const {
  return m_restore_all_signal.connect(slot);
}

connection ToolbarWindow::connect_sign_out_signal(
    const SignOutSignal::slot_type& slot) const {
  return m_sign_out_signal.connect(slot);
}

void ToolbarWindow::closeEvent(QCloseEvent* event) {
  m_sign_out_signal();
  Window::closeEvent(event);
}

MenuButton* ToolbarWindow::make_window_manager_button() {
  auto window_manager_button = make_menu_label_button(tr("Window Manager"));
  window_manager_button->setFixedSize(scale(130, 26));
  auto& window_menu = window_manager_button->get_menu();
  window_menu.add_action(tr("Minimize All"), [=] {
    m_minimize_all_signal();
  });
  window_menu.add_action(tr("Restore All"), [=] {
    m_restore_all_signal();
  });
  window_menu.add_action(tr("Import Settings..."), [=] {});
  window_menu.add_action(tr("Export Settings..."), [=] {});
  return window_manager_button;
}

MenuButton* ToolbarWindow::make_recently_closed_button() {
  auto recently_closed_button = make_menu_label_button(tr("Recently Closed"));
  recently_closed_button->setFixedSize(scale(130, 26));
  auto& history_menu = recently_closed_button->get_menu();
  for(auto i = 0; i < m_recent_windows->get_size(); ++i) {
    history_menu.add_action(m_recent_windows->get(i).m_name, [=] {
      auto& window = m_recent_windows->get(i);
      m_open_signal({window.m_type, window.m_name});
    });
  }
  return recently_closed_button;
}

MenuButton* ToolbarWindow::make_blotter_button() {
  auto blotter_button = make_menu_icon_button(imageFromSvg(
    ":/Icons/toolbar_icons/blotter.svg", scale(26, 26)), "Blotter");
  blotter_button->setFixedSize(scale(32, 26));
  auto& blotter_menu = blotter_button->get_menu();
  blotter_menu.add_action(tr("New..."), [] {});
  blotter_menu.add_separator();
  blotter_menu.add_action(tr("Global"), [] {});
  return blotter_button;
}

Button* ToolbarWindow::make_icon_tool_button(WindowType type,
    const QString& icon_path, const QString& open_name) {
  auto button = make_icon_button(
    imageFromSvg(icon_path, scale(26, 26)), displayText(type));
  button->setFixedSize(scale(26, 26));
  button->connect_click_signal([=] {
    if(open_name.isEmpty()) {
      m_open_signal({type, displayText(type)});
    } else {
      m_open_signal({type, open_name});
    }
  });
  return button;
}

const QString& Spire::displayText(ToolbarWindow::WindowType type) {
  if(type == ToolbarWindow::WindowType::CHART) {
    static const auto value = QObject::tr("Chart");
    return value;
  } else if(type == ToolbarWindow::WindowType::BOOK_VIEW) {
    static const auto value = QObject::tr("Book View");
    return value;
  } else if(type == ToolbarWindow::WindowType::TIME_AND_SALES) {
    static const auto value = QObject::tr("Time and Sales");
    return value;
  } else if(type == ToolbarWindow::WindowType::BLOTTER) {
    static const auto value = QObject::tr("Blotter");
    return value;
  } else if(type == ToolbarWindow::WindowType::CANVAS) {
    static const auto value = QObject::tr("Canvas");
    return value;
  } else if(type == ToolbarWindow::WindowType::WATCHLIST) {
    static const auto value = QObject::tr("Watchlist");
    return value;
  } else if(type == ToolbarWindow::WindowType::IMBALANCE_INDICATOR) {
    static const auto value = QObject::tr("Imbalance Indicator");
    return value;
  } else if(type == ToolbarWindow::WindowType::PORTFOLIO_VIEWER) {
    static const auto value = QObject::tr("Portfolio View");
    return value;
  } else if(type == ToolbarWindow::WindowType::KEY_BINDINGS) {
    static const auto value = QObject::tr("Key Bindings");
    return value;
  } else if(type == ToolbarWindow::WindowType::PROFILE) {
    static const auto value = QObject::tr("Profile");
    return value;
  } else {
    static const auto value = QObject::tr("None");
    return value;
  }
}
