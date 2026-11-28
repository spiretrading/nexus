#include "Spire/UiViewer/Profiles/SplitViewProfiles.hpp"
#include <QEvent>
#include <QPointer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/SplitView.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Window.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct SplitViewWindow : Window {
    SplitViewWindow(QWidget& body, QWidget* parent)
        : Window(parent) {
      setWindowTitle("SplitView");
      set_body(&body);
    }
  };

  struct SizeObserver : QObject {
    QWidget* m_widget;
    Qt::Orientation m_orientation;
    std::function<void (int, int)> m_size_update;

    SizeObserver(QWidget& widget, std::function<void (int, int)> size_update)
        : QObject(&widget),
          m_widget(&widget),
          m_orientation(Qt::Orientation::Horizontal),
          m_size_update(std::move(size_update)) {
      m_widget->installEventFilter(this);
    }

    void set_orientation(Qt::Orientation orientation) {
      m_orientation = orientation;
      update_size();
    }

    void update_size() {
      if(m_orientation == Qt::Orientation::Horizontal) {
        m_size_update(unscale_width(m_widget->width()), m_widget->width());
      } else {
        m_size_update(unscale_height(m_widget->height()), m_widget->height());
      }
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::Resize) {
        update_size();
      }
      return QObject::eventFilter(watched, event);
    }
  };
}

UiProfile Spire::make_split_view_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property("enabled", true));
  properties.push_back(make_standard_enum_property(
    "orientation", Qt::Orientation::Horizontal, get_orientation_property()));
  properties.push_back(
    make_standard_property("primary_minimum_expanded_width", 200));
  properties.push_back(
    make_standard_property("primary_minimum_expanded_height", 200));
  properties.push_back(
    make_standard_property("secondary_minimum_expanded_width", 200));
  properties.push_back(
    make_standard_property("secondary_minimum_expanded_height", 200));
  auto profile = UiProfile("SplitView", properties, [] (auto& profile) {
    auto& enabled = get<bool>("enabled", profile.get_properties());
    auto& orientation =
      get<Qt::Orientation>("orientation", profile.get_properties());
    auto& primary_minimum_expanded_width =
      get<int>("primary_minimum_expanded_width", profile.get_properties());
    auto& primary_minimum_expanded_height =
      get<int>("primary_minimum_expanded_height", profile.get_properties());
    auto& secondary_minimum_expanded_width =
      get<int>("secondary_minimum_expanded_width", profile.get_properties());
    auto& secondary_minimum_expanded_height =
      get<int>("secondary_minimum_expanded_height", profile.get_properties());
    auto button = make_label_button("Click me");
    button->connect_click_signal(
        [=, window = QPointer<SplitViewWindow>(), &enabled, &orientation,
        &primary_minimum_expanded_width, &primary_minimum_expanded_height,
        &secondary_minimum_expanded_width,
        &secondary_minimum_expanded_height] () mutable {
      if(window) {
        window->activateWindow();
        window->raise();
        return;
      }
      auto primary_box = make_label("");
      primary_box->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Expanding);
      update_style(*primary_box, [] (auto& style) {
        style.get(ReadOnly() && Disabled()).
          set(BackgroundColor(QColor(0xFFDFBF)));
        style.get(Any()).set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
      });
      primary_box->setMinimumWidth(
        scale_width(primary_minimum_expanded_width.get()));
      primary_box->setMinimumHeight(
        scale_height(primary_minimum_expanded_height.get()));
      auto primary_size = new SizeObserver(*primary_box,
        [=] (auto size, auto device_size) {
          auto text = QString("%1 (%2px)").arg(size).arg(device_size);
          if(primary_box->get_current()->get() != text) {
            primary_box->get_current()->set(text);
          }
        });
      auto secondary_box = make_label("");
      secondary_box->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Expanding);
      update_style(*secondary_box, [] (auto& style) {
        style.get(ReadOnly() && Disabled()).
          set(BackgroundColor(QColor(0xFFFFBF)));
        style.get(Any()).set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
      });
      secondary_box->setMinimumWidth(
        scale_width(secondary_minimum_expanded_width.get()));
      secondary_box->setMinimumHeight(
        scale_height(secondary_minimum_expanded_height.get()));
      auto secondary_size = new SizeObserver(*secondary_box,
        [=] (auto size, auto device_size) {
          auto text = QString("%1 (%2px)").arg(size).arg(device_size);
          if(secondary_box->get_current()->get() != text) {
            secondary_box->get_current()->set(text);
          }
        });
      auto view = new SplitView(*primary_box, *secondary_box);
      window = new SplitViewWindow(*view, button);
      window->setAttribute(Qt::WA_DeleteOnClose);
      auto view_size = new SizeObserver(*view,
        [=] (auto size, auto device_size) {
          if(window) {
            window->setWindowTitle(
              QString("SplitView %1 (%2px)").arg(size).arg(device_size));
          }
        });
      enabled.connect_changed_signal([=] (auto value) {
        if(window) {
          view->setEnabled(value);
        }
      });
      orientation.connect_changed_signal([=] (auto orientation) {
        if(window) {
          update_style(*view, [&] (auto& style) {
            style.get(Any()).set(orientation);
          });
          primary_size->set_orientation(orientation);
          secondary_size->set_orientation(orientation);
          view_size->set_orientation(orientation);
        }
      });
      primary_minimum_expanded_width.connect_changed_signal([=] (auto value) {
        if(window) {
          primary_box->setMinimumWidth(scale_width(value));
        }
      });
      primary_minimum_expanded_height.connect_changed_signal([=] (auto value) {
        if(window) {
          primary_box->setMinimumHeight(scale_height(value));
        }
      });
      secondary_minimum_expanded_width.connect_changed_signal([=] (auto value) {
        if(window) {
          secondary_box->setMinimumWidth(scale_width(value));
        }
      });
      secondary_minimum_expanded_height.connect_changed_signal(
        [=] (auto value) {
          if(window) {
            secondary_box->setMinimumHeight(scale_height(value));
          }
        });
      primary_size->set_orientation(orientation.get());
      secondary_size->set_orientation(orientation.get());
      view_size->set_orientation(orientation.get());
      window->show();
      window->resize(scale(1002, 200));
    });
    return button;
  });
  return profile;
}
