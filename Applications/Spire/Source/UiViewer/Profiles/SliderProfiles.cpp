#include "Spire/UiViewer/Profiles/SliderProfiles.hpp"
#include <QLinearGradient>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Slider.hpp"
#include "Spire/Ui/Slider2D.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"

using namespace Spire;
using namespace Spire::Styles;

UiProfile Spire::make_slider_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto type_property = define_enum<int>({{"None", 0}, {"HueSlider", 1}});
  properties.push_back(make_standard_enum_property("type", type_property));
  properties.push_back(
    make_standard_enum_property("orientation", get_orientation_property()));
  properties.push_back(make_standard_property<Decimal>("minimum", 0));
  properties.push_back(make_standard_property<Decimal>("maximum", 360));
  properties.push_back(make_standard_property<Decimal>("default_increment", 1));
  properties.push_back(make_standard_property<Decimal>("shift_increment", 10));
  properties.push_back(make_standard_property<Decimal>("step_size", 0));
  properties.push_back(make_standard_property<Decimal>("current", 0));
  auto profile = UiProfile("Slider", properties, [] (auto& profile) {
    auto model = std::make_shared<LocalScalarValueModel<Decimal>>();
    auto& default_increment =
      get<Decimal>("default_increment", profile.get_properties());
    auto& shift_increment =
      get<Decimal>("shift_increment", profile.get_properties());
    auto modifiers = QHash<Qt::KeyboardModifier, Decimal>(
      {{Qt::NoModifier, default_increment.get()},
        {Qt::ShiftModifier, shift_increment.get()}});
    auto slider = new Slider(model, std::move(modifiers));
    apply_widget_properties(slider, profile.get_properties());
    auto& type = get<int>("type", profile.get_properties());
    type.connect_changed_signal([=] (auto value) {
      if(value == 0) {
        update_style(*slider, [] (auto& style) {
          style.get(Any() > Track()).set(IconImage(QImage()));
          style.get(Any() > Thumb() > is_a<Icon>()).set(IconImage(QImage()));
        });
      } else if(value == 1) {
        auto track_image = QImage(":/Icons/hue-spectrum.png");
        auto thumb_image =
          image_from_svg(":/Icons/color-thumb.svg", scale(14, 14));
        update_style(*slider, [&] (auto& style) {
          style.get(Any() > Track()).set(IconImage(track_image));
          style.get(Any() > Thumb() > is_a<Icon>()).
            set(Fill(boost::optional<QColor>())).
            set(IconImage(thumb_image));
          style.get(Focus() > Thumb() > is_a<Icon>()).
            set(Fill(QColor(0x808080)));
          });
      }
      model->set(model->get());
    });
    auto& orientation =
      get<Qt::Orientation>("orientation", profile.get_properties());
    orientation.connect_changed_signal([=] (auto value) {
      update_style(*slider, [&] (auto& style) {
        style.get(Any()).set(value);
      });
      if(value == Qt::Horizontal) {
        slider->setFixedSize(scale(220, 26));
      } else {
        slider->setFixedSize(scale(26, 220));
      }
    });
    auto& minimum = get<Decimal>("minimum", profile.get_properties());
    minimum.connect_changed_signal([=] (auto value) {
      model->set_minimum(value);
    });
    auto& maximum = get<Decimal>("maximum", profile.get_properties());
    maximum.connect_changed_signal([=] (auto value) {
      model->set_maximum(value);
    });
    auto& step_size = get<Decimal>("step_size", profile.get_properties());
    step_size.connect_changed_signal([=] (auto value) {
      slider->set_step(value);
    });
    auto& current = get<Decimal>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      slider->get_current()->set(value);
    });
    auto current_slot = profile.make_event_slot<QString>("Current");
    slider->get_current()->connect_update_signal(
      [=] (const Decimal& value) {
        current_slot(to_string(value));
      });
    auto submit_slot = profile.make_event_slot<QString>("Submit");
    slider->connect_submit_signal([=] (const Decimal& submission) {
      submit_slot(to_string(submission));
    });
    return slider;
  });
  return profile;
}

UiProfile Spire::make_slider_2d_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto type_property = define_enum<int>({{"None", 0}, {"ColorSpectrum", 1}});
  properties.push_back(make_standard_enum_property("type", type_property));
  properties.push_back(make_standard_property<Decimal>("x_minimum", 0));
  properties.push_back(make_standard_property<Decimal>("x_maximum", 100));
  properties.push_back(make_standard_property<Decimal>("y_minimum", 0));
  properties.push_back(make_standard_property<Decimal>("y_maximum", 100));
  properties.push_back(
    make_standard_property<Decimal>("default_x_increment", 1));
  properties.push_back(
    make_standard_property<Decimal>("default_y_increment", 1));
  properties.push_back(
    make_standard_property<Decimal>("shift_x_increment", 10));
  properties.push_back(
    make_standard_property<Decimal>("shift_y_increment", 10));
  properties.push_back(make_standard_property<Decimal>("x_step_size", 0));
  properties.push_back(make_standard_property<Decimal>("y_step_size", 0));
  properties.push_back(make_standard_property<Decimal>("x_current", 0));
  properties.push_back(make_standard_property<Decimal>("y_current", 0));
  auto size = scale(264, 164);
  auto brightness_gradient = QLinearGradient(0, 0, 0, size.height());
  brightness_gradient.setColorAt(0, Qt::transparent);
  brightness_gradient.setColorAt(1, Qt::black);
  auto saturation_gradient = QLinearGradient(0, 0, size.height(), 0);
  saturation_gradient.setColorAt(0, QColor(0xFFFFFF));
  saturation_gradient.setColorAt(1, QColor(0xFFDE00));
  auto track_image = QImage(size, QImage::Format_ARGB32_Premultiplied);
  auto painter = QPainter(&track_image);
  painter.fillRect(QRect({0, 0}, size), saturation_gradient);
  painter.setCompositionMode(QPainter::CompositionMode_Multiply);
  painter.fillRect(QRect({0, 0}, size), brightness_gradient);
  auto profile = UiProfile("Slider2D", properties, [=] (auto& profile) {
    auto x_current_model = std::make_shared<LocalScalarValueModel<Decimal>>();
    auto y_current_model = std::make_shared<LocalScalarValueModel<Decimal>>();
    auto& default_x_increment =
      get<Decimal>("default_x_increment", profile.get_properties());
    auto& default_y_increment =
      get<Decimal>("default_y_increment", profile.get_properties());
    auto& shift_x_increment =
      get<Decimal>("shift_x_increment", profile.get_properties());
    auto& shift_y_increment =
      get<Decimal>("shift_y_increment", profile.get_properties());
    auto x_modifiers = QHash<Qt::KeyboardModifier, Decimal>(
      {{Qt::NoModifier, default_x_increment.get()},
        {Qt::ShiftModifier, shift_x_increment.get()}});
    auto y_modifiers = QHash<Qt::KeyboardModifier, Decimal>(
      {{Qt::NoModifier, default_y_increment.get()},
        {Qt::ShiftModifier, shift_y_increment.get()}});
    auto slider = new Slider2D(x_current_model, y_current_model,
      std::move(x_modifiers), std::move(y_modifiers));
    apply_widget_properties(slider, profile.get_properties());
    slider->setFixedSize(size);
    auto thumb_image =
      image_from_svg(":/Icons/color-thumb.svg", scale(14, 14));
    auto& type = get<int>("type", profile.get_properties());
    type.connect_changed_signal([=] (auto value) {
      if(value == 0) {
        update_style(*slider, [] (auto& style) {
          style.get(Any() > Track()).set(IconImage(QImage()));
          style.get(Any() > Thumb() > is_a<Icon>()).set(IconImage(QImage()));
        });
      } else {
        update_style(*slider, [&] (auto& style) {
          style.get(Any() > Track()).set(IconImage(track_image));
          style.get(Any() > Thumb() > is_a<Icon>()).
            set(Fill(boost::optional<QColor>())).
            set(IconImage(thumb_image));
          style.get(Focus() > Thumb() > is_a<Icon>()).
            set(Fill(QColor(0x808080)));
        });
      }
      x_current_model->set(x_current_model->get());
      y_current_model->set(y_current_model->get());
    });
    auto& x_minimum = get<Decimal>("x_minimum", profile.get_properties());
    x_minimum.connect_changed_signal([=] (auto value) {
      x_current_model->set_minimum(value);
    });
    auto& x_maximum = get<Decimal>("x_maximum", profile.get_properties());
    x_maximum.connect_changed_signal([=] (auto value) {
      x_current_model->set_maximum(value);
    });
    auto& y_minimum = get<Decimal>("y_minimum", profile.get_properties());
    y_minimum.connect_changed_signal([=] (auto value) {
      y_current_model->set_minimum(value);
    });
    auto& y_maximum = get<Decimal>("y_maximum", profile.get_properties());
    y_maximum.connect_changed_signal([=] (auto value) {
      y_current_model->set_maximum(value);
    });
    auto& x_step_size = get<Decimal>("x_step_size", profile.get_properties());
    x_step_size.connect_changed_signal([=] (auto value) {
      slider->set_x_step(value);
    });
    auto& y_step_size = get<Decimal>("y_step_size", profile.get_properties());
    y_step_size.connect_changed_signal([=] (auto value) {
      slider->set_y_step(value);
    });
    auto& x_current = get<Decimal>("x_current", profile.get_properties());
    x_current.connect_changed_signal([=] (auto value) {
      x_current_model->set(value);
    });
    auto& y_current = get<Decimal>("y_current", profile.get_properties());
    y_current.connect_changed_signal([=] (auto value) {
      y_current_model->set(value);
    });
    auto current_slot = profile.make_event_slot<QString>("Current");
    slider->get_x_current()->connect_update_signal([=] (const Decimal& x) {
      current_slot(QString("%1, %2").
        arg(to_string(x)).arg(to_string(slider->get_y_current()->get())));
    });
    slider->get_y_current()->connect_update_signal([=] (const Decimal& y) {
      current_slot(QString("%1, %2").
        arg(to_string(slider->get_x_current()->get())).arg(to_string(y)));
    });
    auto submit_slot = profile.make_event_slot<QString>("Submit");
    slider->connect_submit_signal([=] (const Decimal& x, const Decimal& y) {
      submit_slot(QString("%1, %2").arg(to_string(x)).arg(to_string(y)));
    });
    return slider;
  });
  return profile;
}
