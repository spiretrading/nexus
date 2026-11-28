#include "Spire/UiViewer/Profiles/ObserverProfiles.hpp"
#include <stack>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/HoverObserver.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/UiViewer/StandardUiProperties.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct HoverBox {
    Box* m_box;
    HoverObserver m_observer;
    boost::signals2::connection m_connection;

    HoverBox(QString name, Box* box, UiProfile& profile)
      : m_box(box),
        m_observer(*m_box),
        m_connection(m_observer.connect_state_signal(
          [=, slot = profile.make_event_slot<QString>(std::move(name))] (
              HoverObserver::State state) {
            auto to_string = [] (HoverObserver::State state) {
              switch(state) {
                case HoverObserver::State::MOUSE_IN:
                  return QString("MOUSE_IN");
                case HoverObserver::State::MOUSE_OVER:
                  return QString("MOUSE_OVER");
              }
              return QString("NONE");
            };
            slot(to_string(state));
          })) {}
  };
}

UiProfile Spire::make_focus_observer_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto test_widget_property = define_enum<int>(
    {{"DurationBox", 0}, {"LabelButton", 1}, {"ListView", 2}});
  properties.push_back(
    make_standard_enum_property("widget", test_widget_property));
  properties.push_back(make_standard_property("observer_count", 1));
  auto profile = UiProfile("FocusObserver", properties, [] (auto& profile) {
    static auto observers = std::vector<std::shared_ptr<FocusObserver>>();
    observers.clear();
    auto filter_slot = profile.make_event_slot<QString>("StateSignal");
    auto to_string = [] (auto state) {
      if(state == FocusObserver::State::NONE) {
        return "NONE";
      } else if(state == FocusObserver::State::FOCUS_IN) {
        return "FOCUS_IN";
      } else if(state == FocusObserver::State::FOCUS) {
        return "FOCUS";
      } else {
        return "FOCUS_VISIBLE";
      }
    };
    auto& test_widget = get<int>("widget", profile.get_properties());
    auto widget = [&] () -> QWidget* {
      auto value = test_widget.get();
      if(value == 0) {
        return new DurationBox();
      } else if(value == 1) {
        auto label_button = make_label_button("Label Button");
        update_style(*label_button, [&] (auto& style) {
          style.get(Focus() > Body()).set(
            border_color(QColor(Qt::transparent)));
          style.get(FocusVisible() > Body()).set(
            border_color(QColor(0x4B23A0)));
        });
        return label_button;
      } else {
        auto item_count = 10;
        auto list_model = std::make_shared<ArrayListModel<QString>>();
        for(auto i = 0; i < item_count; ++i) {
          list_model->push(QString("Item%1").arg(i));
        }
        auto list_view = new ListView(list_model);
        for(auto i = 0; i < item_count; ++i) {
          auto item_focus_observer = std::make_shared<FocusObserver>(
            *list_view->get_list_item(i));
          item_focus_observer->connect_state_signal([=] (auto state) {
            filter_slot(QString("%1").arg(to_string(state)));
          });
          observers.push_back(item_focus_observer);
        }
        auto timer = new QTimer(list_view);
        QObject::connect(timer, &QTimer::timeout, [=] {
          if(auto& current = list_view->get_current()->get()) {
            list_view->get_current()->set((*current + 1) % item_count);
          }
        });
        timer->start(3000);
        return list_view;
      }
    }();
    apply_widget_properties(widget, profile.get_properties());
    auto& observer_count = get<int>("observer_count",
      profile.get_properties());
    for(int i = 0; i < observer_count.get(); ++i) {
      auto focus_observer = std::make_shared<FocusObserver>(*widget);
      focus_observer->connect_state_signal([=] (auto state) {
        filter_slot(QString("%1").arg(to_string(state)));
        });
      observers.push_back(focus_observer);
    }
    return widget;
  });
  return profile;
}

UiProfile Spire::make_hover_observer_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("HoverObserver", properties, [] (auto& profile) {
    auto container = new QWidget();
    container->setFixedSize(scale(350, 300));
    apply_widget_properties(container, profile.get_properties());
    auto box1_body = new QWidget();
    auto overlap_box1 = make_input_box(new QWidget(), container);
    overlap_box1->setFixedSize(100, 100);
    overlap_box1->move(translate(0, 50));
    auto box1 =
      std::make_shared<HoverBox>("overlap_box1", overlap_box1, profile);
    auto overlap_box2 = make_input_box(new QWidget(), container);
    overlap_box2->setFixedSize(scale(100, 100));
    overlap_box2->move(translate(50, 100));
    auto box2 =
      std::make_shared<HoverBox>("overlap_box2", overlap_box2, profile);
    auto box_stack =
      std::make_shared<std::stack<std::unique_ptr<HoverBox>>>();
    auto parent_box = make_input_box(new QWidget(), container);
    auto parent_box_observer = HoverObserver(*parent_box);
    box_stack->push(
      std::make_unique<HoverBox>("parent", parent_box, profile));
    parent_box->setFixedSize(scale(175, 200));
    parent_box->move(translate(175, 0));
    auto add_button = make_label_button("Add Child", container);
    add_button->move(translate(75, 225));
    add_button->connect_click_signal([=, &profile] {
      auto parent_box = std::move(box_stack->top());
      auto box = make_input_box(new QWidget(), parent_box->m_box);
      box->setFixedSize(parent_box->m_box->size().shrunkBy({scale_width(10),
        scale_height(10), scale_width(10), scale_height(10)}));
      box->move(translate(10, 10));
      box->show();
      box_stack->push(std::make_unique<HoverBox>(
        QString("child_%1").arg(box_stack->size()), box, profile));
    });
    auto remove_button = make_label_button("Remove Child", container);
    remove_button->move(translate(200, 225));
    remove_button->connect_click_signal([=] {
      if(box_stack->size() > 1) {
        auto box = std::move(box_stack->top());
        box_stack->pop();
        box->m_box->deleteLater();
      }
    });
    auto left_button = make_label_button("Move Left", container);
    left_button->move(translate(75, 265));
    left_button->connect_click_signal([=] {
      container->window()->move(
        container->window()->x() - scale_width(50), container->window()->y());
    });
    auto right_button = make_label_button("Move Right", container);
    right_button->move(translate(200, 265));
    right_button->connect_click_signal([=] {
      container->window()->move(
        container->window()->x() + scale_width(50), container->window()->y());
    });
    return container;
  });
  return profile;
}
