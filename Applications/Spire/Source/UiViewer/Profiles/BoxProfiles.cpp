#include "Spire/UiViewer/Profiles/BoxProfiles.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/AdaptiveBox.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/UiViewer/StandardUiProperties.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

UiProfile Spire::make_adaptive_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_widget_size_properties("parent_width", "parent_height", properties);
  auto size_policy_property = define_enum<int>(
    {{"Blue", 0}, {"Green", 1}, {"Yellow", 2}});
  properties.push_back(make_standard_enum_property(
    "horizontal_size_policy", size_policy_property));
  properties.push_back(make_standard_enum_property(
    "vertical_size_policy", size_policy_property));
  properties.push_back(make_standard_property<QString>("label1_current",
    "Label1"));
  properties.push_back(make_standard_property<QString>("label2_current",
    "Label2"));
  properties.push_back(make_standard_property<QString>("label3_current",
    "Label3"));
  properties.push_back(make_standard_property<QString>("label4_current",
    "Label4"));
  properties.push_back(make_standard_property("add_layout", false));
  auto profile = UiProfile(QString::fromUtf8("AdaptiveBox"), properties,
    [] (auto& profile) {
      auto to_size_policy = [] (auto policy) {
        if(policy == 0) {
          return QSizePolicy::Expanding;
        }
        return QSizePolicy::Fixed;
      };
      auto make_color_label = [] (const QColor& background_color) {
        auto label = make_label("");
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        update_style(*label, [&] (auto& style) {
          style.get(ReadOnly() && Disabled()).
            set(BackgroundColor(background_color));
        });
        return label;
      };
      auto make_label_current =
        [&profile] (TextBox* label, const QString& name) {
          auto& label_current = get<QString>(name, profile.get_properties());
          label_current.connect_changed_signal([=] (auto& value) {
            if(label->get_current()->get() != value) {
              label->get_current()->set(value);
            }
          });
      };
      auto label1 = make_color_label(Qt::yellow);
      auto label2 = make_color_label(Qt::green);
      auto label3 = make_color_label(Qt::cyan);
      auto label4 = make_color_label(Qt::magenta);
      make_label_current(label1, "label1_current");
      make_label_current(label2, "label2_current");
      make_label_current(label3, "label3_current");
      make_label_current(label4, "label4_current");
      auto layout1 = make_hbox_layout();
      layout1->setSpacing(scale_width(10));
      layout1->addWidget(label1);
      layout1->addWidget(label2);
      layout1->addWidget(label3);
      layout1->addWidget(label4);
      auto layout2 = make_grid_layout();
      layout2->setSpacing(scale_width(10));
      layout2->addWidget(label1, 0, 0);
      layout2->addWidget(label2, 0, 1);
      layout2->addWidget(label3, 1, 0);
      layout2->addWidget(label4, 1, 1);
      auto layout3 = make_vbox_layout();
      layout3->setSpacing(scale_width(10));
      layout3->addWidget(label1);
      layout3->addWidget(label2);
      layout3->addWidget(label3);
      layout3->addWidget(label4);
      auto adaptive_box = new AdaptiveBox();
      adaptive_box->add(*layout1);
      adaptive_box->add(*layout2);
      adaptive_box->add(*layout3);
      adaptive_box->setObjectName("adaptive_box");
      adaptive_box->setStyleSheet(
        "#adaptive_box {background-color: lightGray;}");
      apply_widget_properties(adaptive_box, profile.get_properties());
      auto box = new Box(adaptive_box);
      apply_widget_size_properties(box,"parent_width", "parent_height",
        profile.get_properties());
      update_style(*box, [] (auto& style) {
        style.get(Any()).set(border(scale_width(1), QColor(0x4B23A0)));
      });
      box->setFixedSize(scale(200, 200));
      auto& horizontal_size_policy = get<int>("horizontal_size_policy",
        profile.get_properties());
      auto horizontal_size_policy_connection =
        horizontal_size_policy.connect_changed_signal([=] (const auto& policy) {
        if(policy == 2) {
          adaptive_box->setFixedWidth(adaptive_box->width());
        } else {
          adaptive_box->setMinimumWidth(0);
          adaptive_box->setMaximumWidth(QWIDGETSIZE_MAX);
          auto size_policy = adaptive_box->sizePolicy();
          size_policy.setHorizontalPolicy(to_size_policy(policy));
          adaptive_box->setSizePolicy(size_policy);
          adaptive_box->updateGeometry();
        }
      });
      auto& vertical_size_policy = get<int>("vertical_size_policy",
        profile.get_properties());
      vertical_size_policy.connect_changed_signal([=] (const auto& policy) {
        if(policy == 2) {
          adaptive_box->setFixedHeight(adaptive_box->height());
        } else {
          adaptive_box->setMinimumHeight(0);
          adaptive_box->setMaximumHeight(QWIDGETSIZE_MAX);
          auto size_policy = adaptive_box->sizePolicy();
          size_policy.setVerticalPolicy(to_size_policy(policy));
          adaptive_box->setSizePolicy(size_policy);
          adaptive_box->updateGeometry();
        }
      });
      auto& width = get<int>("width", profile.get_properties());
      width.connect_changed_signal([=, &horizontal_size_policy] (auto& value) {
        if(adaptive_box->minimumWidth() == adaptive_box->maximumWidth()) {
          horizontal_size_policy.set(2);
        }
      });
      auto& height = get<int>("height", profile.get_properties());
      height.connect_changed_signal([=, &vertical_size_policy] (auto& value) {
        if(adaptive_box->minimumHeight() == adaptive_box->maximumHeight()) {
          vertical_size_policy.set(2);
        }
      });
      auto& add_layout = get<bool>("add_layout", profile.get_properties());
      add_layout.connect_changed_signal([=] (auto& value) {
        if(value) {
          auto layout = make_vbox_layout();
          layout->setSpacing(scale_width(10));
          auto layout_top = make_hbox_layout();
          layout_top->setSpacing(scale_width(10));
          layout_top->addWidget(label1);
          layout_top->addWidget(label2);
          layout_top->addWidget(label3);
          layout->addLayout(layout_top);
          layout->addWidget(label4);
          adaptive_box->add(*layout);
        }
      });
      return box;
    });
  return profile;
}

UiProfile Spire::make_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto default_style = R"(
    any {
      background_color: 0xFFFFFF;
      padding_top: 1px;
      padding_right: 1px;
      padding_bottom: 1px;
      padding_left: 1px;
      border_top_size: 1px;
      border_right_size: 1px;
      border_bottom_size: 1px;
      border_left_size: 1px;
      border_top_color: 0xC8C8C8;
      border_right_color: 0xC8C8C8;
      border_bottom_color: 0xC8C8C8;
      border_left_color: 0xC8C8C8;
      border_top_left_radius: 0;
      border_top_right_radius: 0;
      border_bottom_right_radius: 0;
      border_bottom_left_radius: 0;
    }
    :hover or :focus {
      border_color: 0x4B23A0;
    }
    :disabled {
      background_color: 0xF5F5F5;
      border_color: 0xC8C8C8;
    }
  )";
  properties.push_back(
    make_style_property("style_sheet", std::move(default_style)));
  auto profile = UiProfile(QString::fromUtf8("Box"), properties,
    [] (auto& profile) {
      auto box = new Box(nullptr);
      box->setFixedSize(scale(100, 100));
      apply_widget_properties(box, profile.get_properties());
      auto& style_sheet =
        get<optional<StyleSheet>>("style_sheet", profile.get_properties());
      style_sheet.connect_changed_signal([=] (const auto& styles) {
        if(styles) {
          set_style(*box, *styles);
        }
      });
      return box;
  });
  return profile;
}
