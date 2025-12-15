#include "Spire/Toolbar/ToolbarWindow.hpp"
#include <QFileDialog>
#include <QIcon>
#include <QStandardPaths>
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Toolbar/SettingsPanel.hpp"
#include "Spire/Toolbar/ToolbarWindowSettings.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/LineInputForm.hpp"
#include "Spire/Ui/MenuButton.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace Spire::Styles;

namespace {
  auto make_settings_path(const DirectoryEntry& account) {
    auto path = std::filesystem::path(QStandardPaths::writableLocation(
      QStandardPaths::DocumentsLocation).toStdString());
    path /= account.m_name + "_settings.json";
    return QString::fromStdString(path.string());
  }

  auto make_unique_blotter_name(
      const QString& name, const ListModel<BlotterModel*>& blotters) {
    auto unique_name = name;
    auto count = 1;
    auto is_existing_name = true;
    while(is_existing_name) {
      is_existing_name = false;
      for(auto i = 0; i < blotters.get_size(); ++i) {
        if(blotters.get(i)->GetName() == unique_name.toStdString()) {
          ++count;
          unique_name = QString("%1 %2").arg(name).arg(count);
          is_existing_name = true;
          break;
        }
      }
    }
    return unique_name;
  }

  auto get_recently_closed_icon(ToolbarWindow::WindowType window) {
    if(window == ToolbarWindow::WindowType::BLOTTER) {
      static auto icon = image_from_svg(
        ":/Icons/toolbar/recently_closed/blotter.svg", scale(10, 10));
      return icon;
    } else if(window == ToolbarWindow::WindowType::BOOK_VIEW) {
      static auto icon = image_from_svg(
        ":/Icons/toolbar/recently_closed/book_view.svg", scale(10, 10));
      return icon;
    } else if(window == ToolbarWindow::WindowType::CANVAS) {
      static auto icon = image_from_svg(
        ":/Icons/toolbar/recently_closed/canvas.svg", scale(10, 10));
      return icon;
    } else if(window == ToolbarWindow::WindowType::CHART) {
      static auto icon = image_from_svg(
        ":/Icons/toolbar/recently_closed/chart.svg", scale(10, 10));
      return icon;
    } else if(window == ToolbarWindow::WindowType::KEY_BINDINGS) {
      static auto icon = image_from_svg(
        ":/Icons/toolbar/recently_closed/key_bindings.svg", scale(10, 10));
      return icon;
    } else if(window == ToolbarWindow::WindowType::ORDER_IMBALANCE_INDICATOR) {
      static auto icon = image_from_svg(
        ":/Icons/toolbar/recently_closed/order_imbalance_indicator.svg",
        scale(10, 10));
      return icon;
    } else if(window == ToolbarWindow::WindowType::PORTFOLIO) {
      static auto icon = image_from_svg(
        ":/Icons/toolbar/recently_closed/portfolio.svg", scale(10, 10));
      return icon;
    } else if(window == ToolbarWindow::WindowType::PROFILE) {
      static auto icon = image_from_svg(
        ":/Icons/toolbar/recently_closed/profile.svg", scale(10, 10));
      return icon;
    } else if(window == ToolbarWindow::WindowType::TIME_AND_SALES) {
      static auto icon = image_from_svg(
        ":/Icons/toolbar/recently_closed/time_and_sales.svg", scale(10, 10));
      return icon;
    } else if(window == ToolbarWindow::WindowType::WATCHLIST) {
      static auto icon = image_from_svg(
        ":/Icons/toolbar/recently_closed/watchlist.svg", scale(10, 10));
      return icon;
    }
    throw std::runtime_error("Unknown window type.");
  }
}

ToolbarWindow::ToolbarWindow(DirectoryEntry account, AccountRoles roles,
    std::shared_ptr<RecentlyClosedWindowListModel> recently_closed_windows,
    std::shared_ptr<ListModel<BlotterModel*>> pinned_blotters, QWidget* parent)
    : Window(parent),
      m_account(std::move(account)),
      m_recently_closed_windows(std::move(recently_closed_windows)),
      m_pinned_blotters(std::move(pinned_blotters)) {
  setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
  set_svg_icon(":/Icons/spire.svg");
  setWindowIcon(QIcon(":/Icons/spire-icon-48x48.png"));
  setWindowTitle(QString("Spire - Signed in as %1").arg(
    QString::fromStdString(m_account.m_name)));
  auto top_layout = make_hbox_layout();
  top_layout->addWidget(make_window_manager_button());
  top_layout->addStretch();
  auto recently_closed_button = make_recently_closed_button();
  m_recently_closed_menu = &recently_closed_button->get_menu();
  populate_recently_closed_menu();
  m_recently_closed_windows_connection =
    m_recently_closed_windows->connect_operation_signal(std::bind_front(
      &ToolbarWindow::on_recently_closed_window_operation, this));
  top_layout->addWidget(recently_closed_button);
  auto bottom_layout = make_hbox_layout();
  bottom_layout->setSpacing(scale_width(4));
  bottom_layout->addWidget(
    make_icon_tool_button(WindowType::CANVAS, ":/Icons/toolbar/canvas.svg",
      QColor(0x00848A), QColor(0x00696E), QColor(0x00A88B)));
  bottom_layout->addWidget(make_icon_tool_button(
    WindowType::BOOK_VIEW, ":/Icons/toolbar/book_view.svg",
    QColor(0x406ABF), QColor(0x404ABF), QColor(0x4392D6)));
  bottom_layout->addWidget(make_icon_tool_button(
    WindowType::TIME_AND_SALES, ":/Icons/toolbar/time_and_sales.svg",
    QColor(0x26BF4A), QColor(0x2CAC79), QColor(0x1FD364)));
  bottom_layout->addWidget(
    make_icon_tool_button(WindowType::CHART, ":/Icons/toolbar/chart.svg",
      QColor(0x7F5EEC), QColor(0x684BC7), QColor(0x8D78EC)));
  bottom_layout->addWidget(make_icon_tool_button(
    WindowType::WATCHLIST, ":/Icons/toolbar/watchlist.svg",
    QColor(0xE67A44), QColor(0xCB6431), QColor(0xF28E38)));
  bottom_layout->addWidget(make_icon_tool_button(
    WindowType::ORDER_IMBALANCE_INDICATOR,
    ":/Icons/toolbar/order_imbalance_indicator.svg",
    QColor(0xBF9540), QColor(0x9A7324), QColor(0xE0B04F)));
  auto blotter_button = make_blotter_button();
  m_blotter_menu = &blotter_button->get_menu();
  populate_blotter_menu();
  m_pinned_blotter_connection = m_pinned_blotters->connect_operation_signal(
    std::bind_front(&ToolbarWindow::on_blotter_operation, this));
  bottom_layout->addWidget(blotter_button);
  if(roles.test(AccountRole::MANAGER) ||
      roles.test(AccountRole::ADMINISTRATOR)) {
    bottom_layout->addWidget(make_icon_tool_button(
      WindowType::ACCOUNT_DIRECTORY, ":/Icons/toolbar/account_directory.svg",
      QColor(0x4392D6), QColor(0x406ABF), QColor(0x70C1EB)));
    bottom_layout->addWidget(make_icon_tool_button(
      WindowType::PORTFOLIO, ":/Icons/toolbar/portfolio.svg",
      QColor(0x406ABF), QColor(0x404ABF), QColor(0x4392D6)));
  }
  bottom_layout->addSpacing(scale_width(4));
  auto separator = new Box();
  separator->setFixedSize(scale(1, 16));
  update_style(*separator, [] (auto& styles) {
    styles.get(Any()).set(BackgroundColor(QColor(0xC8C8C8)));
  });
  bottom_layout->addWidget(separator);
  bottom_layout->addSpacing(scale_width(4));
  bottom_layout->addWidget(make_icon_tool_button(
    WindowType::KEY_BINDINGS, ":/Icons/toolbar/key_bindings.svg",
    QColor(0x808080), QColor(0x535353), QColor(0xA0A0A0)));
  bottom_layout->addWidget(
    make_icon_tool_button(WindowType::PROFILE, ":/Icons/toolbar/profile.svg",
      QColor(0x70C1EB), QColor(0x4392D6), QColor(0x70C1EB)));
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

const std::shared_ptr<RecentlyClosedWindowListModel>&
    ToolbarWindow::get_recently_closed_windows() const {
  return m_recently_closed_windows;
}

const std::shared_ptr<ListModel<BlotterModel*>>&
    ToolbarWindow::get_pinned_blotters() const {
  return m_pinned_blotters;
}

connection ToolbarWindow::connect_open_signal(
    const OpenSignal::slot_type& slot) const {
  return m_open_signal.connect(slot);
}

connection ToolbarWindow::connect_reopen_signal(
    const ReopenSignal::slot_type& slot) const {
  return m_reopen_signal.connect(slot);
}

connection ToolbarWindow::connect_open_blotter_signal(
    const OpenBlotterSignal::slot_type& slot) const {
  return m_open_blotter_signal.connect(slot);
}

connection ToolbarWindow::connect_minimize_all_signal(
    const MinimizeAllSignal::slot_type& slot) const {
  return m_minimize_all_signal.connect(slot);
}

connection ToolbarWindow::connect_restore_all_signal(
    const RestoreAllSignal::slot_type& slot) const {
  return m_restore_all_signal.connect(slot);
}

connection ToolbarWindow::connect_import_signal(
    const ImportSignal::slot_type& slot) const {
  return m_import_signal.connect(slot);
}

connection ToolbarWindow::connect_export_signal(
    const ExportSignal::slot_type& slot) const {
  return m_export_signal.connect(slot);
}

connection ToolbarWindow::connect_new_blotter_signal(
    const NewBlotterSignal::slot_type& slot) const {
  return m_new_blotter_signal.connect(slot);
}

connection ToolbarWindow::connect_sign_out_signal(
    const SignOutSignal::slot_type& slot) const {
  return m_sign_out_signal.connect(slot);
}

std::unique_ptr<WindowSettings> ToolbarWindow::GetWindowSettings() const {
  return std::make_unique<ToolbarWindowSettings>(*this);
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
  window_menu.add_action(
    tr("Import Settings"), std::bind_front(&ToolbarWindow::on_import, this));
  window_menu.add_action(
    tr("Export Settings"), std::bind_front(&ToolbarWindow::on_export, this));
  return window_manager_button;
}

MenuButton* ToolbarWindow::make_recently_closed_button() const {
  auto recently_closed_button = make_menu_label_button(tr("Recently Closed"));
  recently_closed_button->setFixedSize(scale(130, 26));
  return recently_closed_button;
}

MenuButton* ToolbarWindow::make_blotter_button() {
  auto blotter_button = make_menu_icon_button(
    image_from_svg(":/Icons/toolbar/blotter.svg", scale(26, 26)),
    to_text(WindowType::BLOTTER));
  update_style(*blotter_button, [&] (auto& style) {
    style.get(Any() > is_a<Icon>()).set(Fill(QColor(0x00BFA0)));
    style.get(Hover() > is_a<Icon>()).set(Fill(QColor(0x00A88B)));
    style.get(Any() > (Press() || FocusIn()) > is_a<Icon>()).
      set(Fill(QColor(0x00D6BB)));
  });
  blotter_button->setFixedSize(scale(32, 26));
  return blotter_button;
}

Button* ToolbarWindow::make_icon_tool_button(
    WindowType type, const QString& icon_path, QColor fill,
    QColor hover_color, QColor press_color) const {
  auto button =
    make_icon_button(image_from_svg(icon_path, scale(26, 26)), to_text(type));
  update_style(*button, [&] (auto& style) {
    style.get(Any() > is_a<Icon>()).set(Fill(fill));
    style.get(Hover() > is_a<Icon>()).set(Fill(hover_color));
    style.get(Press() > is_a<Icon>()).set(Fill(press_color));
  });
  button->setFixedSize(scale(26, 26));
  button->connect_click_signal([=] {
    m_open_signal(type);
  });
  return button;
}

void ToolbarWindow::populate_recently_closed_menu() {
  m_recently_closed_menu->reset();
  for(auto i = 0; i < m_recently_closed_windows->get_size(); ++i) {
    auto& window = m_recently_closed_windows->get(i);
    m_recently_closed_menu->add_action(
      QString::fromStdString(window->GetName()), [=] {
        auto window = m_recently_closed_windows->get(i);
        m_reopen_signal(*window);
      });
  }
}

void ToolbarWindow::populate_blotter_menu() {
  m_blotter_menu->reset();
  m_blotter_menu->add_action(
    tr("New..."), std::bind_front(&ToolbarWindow::on_new_blotter_action, this));
  m_blotter_menu->add_separator();
  for(auto i = 0; i != m_pinned_blotters->get_size(); ++i) {
    auto blotter = m_pinned_blotters->get(i);
    m_blotter_menu->add_action(QString::fromStdString(blotter->GetName()), [=] {
      m_open_blotter_signal(*blotter);
    });
  }
}

void ToolbarWindow::on_recently_closed_window_operation(
    const RecentlyClosedWindowListModel::Operation& operation) {
  populate_recently_closed_menu();
}

void ToolbarWindow::on_import() {
  auto path = QFileDialog::getOpenFileName(this,
    tr("Select the settings file."), make_settings_path(m_account),
    tr("Settings (*.json)"));
  if(path.isNull()) {
    return;
  }
  m_settings_panel = new SettingsPanel(SettingsPanel::Mode::IMPORT, *this);
  m_settings_panel->show();
  m_settings_panel->connect_commit_signal(
    [=] (const auto& settings) {
      m_import_signal(settings, std::filesystem::path(path.toStdString()));
    });
}

void ToolbarWindow::on_export() {
  m_settings_panel = new SettingsPanel(SettingsPanel::Mode::EXPORT, *this);
  m_settings_panel->show();
  m_settings_panel->connect_commit_signal(
    [=] (const auto& settings) {
      auto path = QFileDialog::getSaveFileName(this,
        tr("Select the settings file."), make_settings_path(m_account),
        tr("Settings (*.json)"));
      if(path.isNull()) {
        return;
      }
      m_export_signal(settings, std::filesystem::path(path.toStdString()));
    });
}

void ToolbarWindow::on_new_blotter_action() {
  m_new_blotter_form = new LineInputForm(tr("New Blotter"), *this);
  m_new_blotter_form->connect_submit_signal(
    std::bind_front(&ToolbarWindow::on_new_blotter_submission, this));
  m_new_blotter_form->show();
}

void ToolbarWindow::on_new_blotter_submission(const QString& name) {
  auto unique_name = make_unique_blotter_name(name, *m_pinned_blotters);
  m_new_blotter_form->deleteLater();
  m_new_blotter_signal(unique_name);
}

void ToolbarWindow::on_blotter_operation(
    const ListModel<BlotterModel*>::Operation& operation) {
  populate_blotter_menu();
}

const QString& Spire::to_text(ToolbarWindow::WindowType type) {
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
  } else if(type == ToolbarWindow::WindowType::ORDER_IMBALANCE_INDICATOR) {
    static const auto value = QObject::tr("Order Imbalance Indicator");
    return value;
  } else if(type == ToolbarWindow::WindowType::PORTFOLIO) {
    static const auto value = QObject::tr("Portfolio");
    return value;
  } else if(type == ToolbarWindow::WindowType::KEY_BINDINGS) {
    static const auto value = QObject::tr("Key Bindings");
    return value;
  } else if(type == ToolbarWindow::WindowType::PROFILE) {
    static const auto value = QObject::tr("Profile");
    return value;
  } else if(type == ToolbarWindow::WindowType::ACCOUNT_DIRECTORY) {
    static const auto value = QObject::tr("Account Directory");
    return value;
  } else {
    static const auto value = QObject::tr("None");
    return value;
  }
}
