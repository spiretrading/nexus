#include "Spire/UiViewer/Profiles/NavigationProfiles.hpp"
#include <QImageReader>
#include <QLabel>
#include <QPixmap>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/NavigationView.hpp"
#include "Spire/Ui/ScopeBox.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"
#include "Spire/Ui/TabView.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TickerBox.hpp"
#include "Spire/Ui/TransitionView.hpp"
#include "Spire/UiViewer/Profiles/ProfileModels.hpp"
#include "Spire/UiViewer/StandardUiProperties.hpp"

using namespace Spire;
using namespace Spire::Styles;

UiProfile Spire::make_navigation_view_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("tab_index", 0));
  properties.push_back(make_standard_property("tab_enabled", true));
  properties.push_back(make_standard_property("current", 0));
  auto size_policy_property = define_enum<int>(
    {{"Blue", 0}, {"Green", 1}});
  properties.push_back(make_standard_enum_property(
    "page_horizontal_size_policy", size_policy_property));
  properties.push_back(make_standard_enum_property("page_vertical_size_policy",
    size_policy_property));
  auto profile = UiProfile("NavigationView", properties, [=] (auto& profile) {
    auto& horizontal_size_policy = get<int>("page_horizontal_size_policy",
      profile.get_properties());
    auto& vertical_size_policy = get<int>("page_vertical_size_policy",
      profile.get_properties());
    auto get_size_policy = [] (const auto& size_policy) {
      if(size_policy.get() == 0) {
        return QSizePolicy::Expanding;
      }
      return QSizePolicy::Preferred;
    };
    auto navigation_view = new NavigationView();
    apply_widget_properties(navigation_view, profile.get_properties());
    auto filter_slot = profile.make_event_slot<QString>("CurrentSignal");
    navigation_view->get_current()->connect_update_signal([=] (auto current) {
      filter_slot(QString("%1_%2").arg(current).
        arg(navigation_view->get_label(current)));
    });
    auto page1 = new QWidget();
    page1->setSizePolicy(get_size_policy(horizontal_size_policy),
      get_size_policy(vertical_size_policy));
    page1->setObjectName("page1");
    page1->setStyleSheet("#page1 {background-color: yellow;}");
    auto layout1 = new QVBoxLayout(page1);
    layout1->setSpacing(scale_width(5));
    layout1->addWidget(make_label_button("Button1"));
    layout1->addWidget(make_label_button("Button2"));
    auto model = populate_ticker_query_model();
    auto ticker_box = new TickerBox(model);
    ticker_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    ticker_box->set_placeholder("TickerBox");
    layout1->addWidget(ticker_box);
    auto scope_box = new ScopeBox(populate_scope_box_model());
    scope_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    scope_box->set_placeholder("ScopeBox");
    layout1->addWidget(scope_box);
    navigation_view->add_tab(*page1, "NavTab1");
    auto new_page = [] {
      auto page = new QWidget();
      page->setObjectName("page");
      page->setStyleSheet("#page {background-color: cyan;}");
      auto layout = new QVBoxLayout(page);
      layout->setSpacing(scale_width(5));
      auto text_box1 = new TextBox();
      text_box1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      layout->addWidget(text_box1);
      auto text_box2 = new TextBox();
      text_box2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      layout->addWidget(text_box2);
      return page;
    };
    auto page2 = new_page();
    page2->setSizePolicy(get_size_policy(horizontal_size_policy),
      get_size_policy(vertical_size_policy));
    navigation_view->add_tab(*page2, "NavTab2");
    auto page3 = new_page();
    page3->setFixedSize(scale(200, 90));
    navigation_view->add_tab(*page3, "NavTab3");
    auto page4 = new QWidget();
    page4->setSizePolicy(get_size_policy(horizontal_size_policy),
      get_size_policy(vertical_size_policy));
    page4->setObjectName("page4");
    page4->setStyleSheet("#page4 {background-color: yellow;}");
    auto reader = QImageReader(":/Icons/color-picker-display.png");
    auto image = QPixmap::fromImage(reader.read());
    auto label = new QLabel();
    label->setPixmap(std::move(image));
    auto layout4 = new QHBoxLayout(page4);
    layout4->setSpacing(scale_width(5));
    layout4->addWidget(label);
    navigation_view->add_tab(*page4, "NavTab4");
    auto& tab_index = get<int>("tab_index", profile.get_properties());
    auto& tab_enabled = get<bool>("tab_enabled", profile.get_properties());
    tab_enabled.connect_changed_signal([=, &tab_index] (auto value) {
      auto index = tab_index.get();
      if(index >= 0 && index < navigation_view->get_count()) {
        navigation_view->set_enabled(index, value);
      }
    });
    auto& current = get<int>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto index) {
      if(index >= 0 && index < navigation_view->get_count()) {
        navigation_view->get_current()->set(index);
      }
    });
    return navigation_view;
  });
  return profile;
}

UiProfile Spire::make_tab_view_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("TabView", properties, [] (auto& profile) {
    auto view = new TabView();
    view->add("Positions", *make_label_button("Positions"));
    view->add("Order Log", *make_label_button("Order Log"));
    view->add("Executions", *make_label_button("Executions"));
    view->add("Profit/Loss", *make_label_button("Profit/Loss"));
    apply_widget_properties(view, profile.get_properties());
    return view;
  });
  return profile;
}

UiProfile Spire::make_transition_view_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property<int>("width", 200));
  properties.push_back(make_standard_property<int>("height", 300));
  auto status_property = define_enum<TransitionView::Status>(
    {{"NONE", TransitionView::Status::NONE},
     {"LOADING", TransitionView::Status::LOADING},
     {"READY", TransitionView::Status::READY}});
  properties.push_back(make_standard_enum_property("status", status_property));
  auto profile = UiProfile("TransitionView", properties, [] (auto& profile) {
    auto list_model = std::make_shared<ArrayListModel<QString>>();
    for(auto i = 0; i < 10; ++i) {
      list_model->push(QString("Item%1").arg(i));
    }
    auto list_view = new ListView(list_model,
      [] (const std::shared_ptr<ListModel<QString>>& model, auto index) {
        return make_label(model->get(index));
      });
    update_style(*list_view, [] (auto& style) {
      style.get(Any()).set(Qt::Orientation::Vertical);
    });
    auto transition_view =
      new TransitionView(new ScrollableListBox(*list_view));
    auto box = new Box(transition_view);
    update_style(*box, [] (auto& style) {
      style.get(Any()).set(border(scale_width(1), QColor(0x4B23A0)));
    });
    auto& width = get<int>("width", profile.get_properties());
    width.connect_changed_signal([=] (auto value) {
      if(value != 0) {
        if(unscale_width(transition_view->width()) != value) {
          transition_view->setFixedWidth(scale_width(value));
        }
      }
    });
    auto& height = get<int>("height", profile.get_properties());
    height.connect_changed_signal([=] (auto value) {
      if(value != 0) {
        if(unscale_height(transition_view->height()) != value) {
          transition_view->setFixedHeight(scale_height(value));
        }
      }
    });
    auto& status =
      get<TransitionView::Status>("status", profile.get_properties());
    status.connect_changed_signal([=] (auto s) {
      transition_view->set_status(s);
    });
    return box;
  });
  return profile;
}
