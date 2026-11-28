#include "Spire/UiViewer/Profiles/WindowProfiles.hpp"
#include <QApplication>
#include <QEvent>
#include <QIcon>
#include <QPointer>
#include <QScreen>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/MenuButton.hpp"
#include "Spire/Ui/Window.hpp"
#include "Spire/Ui/WindowHighlight.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  QPoint calculate_window_position(int row, int column,
      const QSize& window_size) {
    return {scale_width(10) + column * (window_size.width() + scale_width(10)),
      scale_height(10) + row * (window_size.height() + scale_height(10))};
  }

  auto populate_windows(int count) {
    auto windows = std::vector<Window*>();
    auto screen = QApplication::primaryScreen()->availableGeometry();
    auto row = 0;
    auto column = 0;
    for(auto i = 0; i < count; ++i) {
      auto window = new Window();
      window->setWindowFlags(
        window->windowFlags() & ~Qt::WindowMaximizeButtonHint);
      window->setWindowTitle(QString("Window %1").arg(i + 1));
      window->show();
      window->resize(scale(300, 400));
      auto pos = calculate_window_position(row, column, window->size());
      if(pos.x() + window->width() + scale_width(10) > screen.width()) {
        row++;
        column = 0;
        pos = calculate_window_position(row, column, window->size());
      }
      column++;
      window->move(pos);
      windows.push_back(window);
    }
    return windows;
  }

  struct WindowHighlightTester : QObject {
    QWidget* m_main_window;
    std::vector<Window*> m_windows;
    std::vector<std::vector<Window*>> m_groups;
    std::unique_ptr<WindowHighlight> m_window_highlight;

    explicit WindowHighlightTester(ContextMenu& menu)
        : QObject(&menu),
          m_main_window(QApplication::activeWindow()),
          m_windows(populate_windows(8)),
          m_window_highlight(std::make_unique<WindowHighlight>(
            std::make_shared<ArrayListModel<Window*>>())) {
      m_main_window->installEventFilter(this);
      for(auto window : m_windows) {
        window->installEventFilter(this);
      }
      m_groups.push_back({m_windows[0], m_windows[1], m_windows[2]});
      m_groups.push_back({m_windows[3], m_windows[4]});
      m_groups.push_back({m_windows[5]});
      m_groups.push_back({m_windows[6], m_windows[7]});
      for(auto i = 0; i < m_groups.size(); ++i) {
        menu.add_action(QString("Group %1").arg(i + 1), [] {});
      }
      menu.connect_current_signal(
        std::bind_front(&WindowHighlightTester::on_current, this));
    }

    ~WindowHighlightTester() {
      clear(*m_window_highlight->get_current());
      for(auto window : m_windows) {
        delete window;
      }
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(watched == m_main_window && event->type() == QEvent::Close) {
        deleteLater();
      } else if(event->type() == QEvent::Close) {
        for(auto i = m_groups.begin(); i != m_groups.end(); ++i) {
          if(auto j = std::find(i->begin(), i->end(), watched); j != i->end()) {
            i->erase(j);
            if(i->empty()) {
              m_groups.erase(i);
            }
          }
        }
      }
      return QObject::eventFilter(watched, event);
    }

    void on_current(optional<int> current) {
      auto& current_model = *m_window_highlight->get_current();
      current_model.transact([&] {
        clear(current_model);
        if(current && *current >= 0 &&
            *current < static_cast<int>(m_groups.size())) {
          for(auto window : m_groups[*current]) {
            current_model.push(window);
          }
        }
      });
    }
  };
}

UiProfile Spire::make_title_bar_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(
    make_standard_property("title", QString("Spire Desktop")));
  auto icon_property = define_enum<std::tuple<QString, QString>>(
    {{"Spire", {":/Icons/spire.svg", ":/Icons/taskbar_icons/spire.png"}},
    {"KeyBindings",
      {":/Icons/key-bindings.svg", ":/Icons/taskbar_icons/key-bindings.png"}},
    {"TimeAndSales",
      {":/Icons/time-sales.svg", ":/Icons/taskbar_icons/time-sales.png"}}});
  populate_enum_properties(properties, "icon", icon_property);
  properties.push_back(make_standard_property("highlighted", false));
  auto profile = UiProfile("TitleBar", properties, [] (auto& profile) {
    auto& title = get<QString>("title", profile.get_properties());
    auto& icon =
      get<std::tuple<QString, QString>>("icon", profile.get_properties());
    auto& highlighted = get<bool>("highlighted", profile.get_properties());
    auto button = make_label_button("Click me");
    button->connect_click_signal([&title, &icon, &highlighted] {
      auto window = QPointer<Window>(new Window());
      window->setAttribute(Qt::WA_DeleteOnClose);
      title.connect_changed_signal([=] (const auto& text) {
        if(window) {
          window->setWindowTitle(text);
        }
      });
      icon.connect_changed_signal([=] (const auto& value) {
        if(window) {
          window->set_svg_icon(get<0>(value));
          window->setWindowIcon(QIcon(get<1>(value)));
        }
      });
      highlighted.connect_changed_signal([=] (auto is_highlighted) {
        if(!window) {
          return;
        }
        if(is_highlighted) {
          match(*window, Highlighted());
        } else {
          unmatch(*window, Highlighted());
        }
      });
      window->show();
    });
    return button;
  });
  return profile;
}

UiProfile Spire::make_window_highlight_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  auto profile = UiProfile("WindowHighlight", properties, [] (auto& profile) {
    auto button = make_menu_label_button("Window Highlight");
    auto tester = new WindowHighlightTester(button->get_menu());
    return button;
  });
  return profile;
}
