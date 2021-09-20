#include "Spire/UiViewer/StandardUiProfiles.hpp"
#include <QLabel>
#include <QPainter>
#include <QPointer>
#include <QRandomGenerator>
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Checkbox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/DestinationListItem.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/InfoTip.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Ui/KeyTag.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/OrderTypeBox.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/RegionListItem.hpp"
#include "Spire/Ui/ScalarFilterPanel.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"
#include "Spire/Ui/SearchBox.hpp"
#include "Spire/Ui/SecurityListItem.hpp"
#include "Spire/Ui/SideBox.hpp"
#include "Spire/Ui/Tag.hpp"
#include "Spire/Ui/TextAreaBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Tooltip.hpp"
#include "Spire/UiViewer/StandardUiProperties.hpp"
#include "Spire/UiViewer/UiProfile.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  template<typename T>
  struct DecimalBoxProfileProperties {
    using Type = T;

    Type m_current;
    Type m_minimum;
    Type m_maximum;
    Type m_default_increment;
    Type m_alt_increment;
    Type m_ctrl_increment;
    Type m_shift_increment;

    explicit DecimalBoxProfileProperties(Type default_increment)
      : m_current(Type(1)),
        m_minimum(Type(-100)),
        m_maximum(Type(100)),
        m_default_increment(std::move(default_increment)),
        m_alt_increment(5 * m_default_increment),
        m_ctrl_increment(10 * m_default_increment),
        m_shift_increment(20 * m_default_increment) {}
  };

  template<typename B>
  auto setup_decimal_box_profile(UiProfile& profile) {
    using Type = std::decay_t<decltype(*std::declval<B>().get_model())>::Scalar;
    auto model = std::make_shared<LocalScalarValueModel<optional<Type>>>();
    auto& minimum = get<Type>("minimum", profile.get_properties());
    minimum.connect_changed_signal([=] (auto value) {
      model->set_minimum(value);
    });
    auto& maximum = get<Type>("maximum", profile.get_properties());
    maximum.connect_changed_signal([=] (auto value) {
      model->set_maximum(value);
    });
    auto& default_increment =
      get<Type>("default_increment", profile.get_properties());
    auto& alt_increment = get<Type>("alt_increment", profile.get_properties());
    auto& ctrl_increment =
      get<Type>("ctrl_increment", profile.get_properties());
    auto& shift_increment =
      get<Type>("shift_increment", profile.get_properties());
    auto modifiers = QHash<Qt::KeyboardModifier, Type>(
      {{Qt::NoModifier, default_increment.get()},
        {Qt::AltModifier, alt_increment.get()},
        {Qt::ControlModifier, ctrl_increment.get()},
        {Qt::ShiftModifier, shift_increment.get()}});
    auto box = new B(std::move(model), std::move(modifiers));
    box->setFixedWidth(scale_width(100));
    apply_widget_properties(box, profile.get_properties());
    auto& current = get<Type>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      if(box->get_model()->get_current() != value) {
        box->get_model()->set_current(value);
      }
    });
    box->get_model()->connect_current_signal(
      profile.make_event_slot<optional<Type>>(QString::fromUtf8("Current")));
    box->connect_submit_signal(
      profile.make_event_slot<optional<Type>>(QString::fromUtf8("Submit")));
    box->connect_reject_signal(
      profile.make_event_slot<optional<Type>>(QString::fromUtf8("Reject")));
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& placeholder) {
      box->set_placeholder(placeholder);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto value) {
      box->set_read_only(value);
    });
    auto& buttons_visible =
      get<bool>("buttons_visible", profile.get_properties());
    buttons_visible.connect_changed_signal([=] (auto value) {
      auto style = get_style(*box);
      if(value) {
        style.get(Any() > is_a<Button>()).get_block().remove<Visibility>();
      } else {
        style.get(Any() > is_a<Button>()).set(Visibility::NONE);
      }
      set_style(*box, std::move(style));
    });
    return box;
  }

  auto setup_checkable_profile(UiProfile& profile, CheckBox* check_box) {
    auto& label = get<QString>("label", profile.get_properties());
    check_box->set_label(label.get());
    apply_widget_properties(check_box, profile.get_properties());
    label.connect_changed_signal([=] (const auto& value) {
      check_box->set_label(value);
    });
    auto& checked = get<bool>("checked", profile.get_properties());
    checked.connect_changed_signal([=] (auto value) {
      if(check_box->get_model()->get_current() != value) {
        check_box->get_model()->set_current(value);
      }
    });
    check_box->get_model()->connect_current_signal([&] (auto is_checked) {
      checked.set(is_checked);
    });
    check_box->get_model()->connect_current_signal(
      profile.make_event_slot<bool>(QString::fromUtf8("CheckedSignal")));
    auto& read_only = get<bool>("read-only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      check_box->set_read_only(is_read_only);
    });
    auto& layout_direction = get<bool>("left-to-right",
      profile.get_properties());
    layout_direction.connect_changed_signal([=] (auto value) {
      if(value) {
        check_box->setLayoutDirection(Qt::LeftToRight);
      } else {
        check_box->setLayoutDirection(Qt::RightToLeft);
      }
    });
    return check_box;
  }

  void populate_check_box_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties) {
    properties.push_back(make_standard_property<bool>("checked"));
    properties.push_back(
      make_standard_property("label", QString::fromUtf8("Click me!")));
    properties.push_back(make_standard_property<bool>("read-only"));
    properties.push_back(make_standard_property("left-to-right", true));
  }

  template<typename T>
  void populate_decimal_box_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties,
      const DecimalBoxProfileProperties<T>& box_properties) {
    using Type = T;
    properties.push_back(make_standard_property("current",
      box_properties.m_current));
    properties.push_back(make_standard_property("minimum",
      box_properties.m_minimum));
    properties.push_back(make_standard_property("maximum",
      box_properties.m_maximum));
    properties.push_back(
      make_standard_property("default_increment",
        box_properties.m_default_increment));
    properties.push_back(
      make_standard_property("alt_increment",
        box_properties.m_alt_increment));
    properties.push_back(
      make_standard_property("ctrl_increment",
        box_properties.m_ctrl_increment));
    properties.push_back(
      make_standard_property("shift_increment",
        box_properties.m_shift_increment));
    properties.push_back(make_standard_property<QString>("placeholder"));
    properties.push_back(make_standard_property("read_only", false));
    properties.push_back(make_standard_property("buttons_visible", true));
  }

  template<typename T>
  void populate_decimal_box_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties) {
    populate_decimal_box_properties<T>(properties,
      DecimalBoxProfileProperties(1));
  }

  template<typename B, typename B* (*F)(QWidget*)>
  auto setup_enum_box_profile(UiProfile& profile) {
    using Type = B::Type;
    auto box = F(nullptr);
    box->setFixedWidth(scale_width(150));
    apply_widget_properties(box, profile.get_properties());
    auto& current = get<Type>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      box->get_current()->set_current(value);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      box->set_read_only(is_read_only);
    });
    box->connect_submit_signal(
      profile.make_event_slot<std::any>(QString::fromUtf8("Submit")));
    return box;
  }

  template<typename T>
  void populate_enum_box_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties,
      std::vector<std::pair<QString, T>>& current_property) {
    properties.push_back(make_standard_enum_property(
      "current", current_property));
    properties.push_back(make_standard_property("read_only", false));
  }

  template<typename B>
  auto setup_scalar_filter_panel_profile(UiProfile& profile) {
    using Type = std::decay_t<decltype(*std::declval<B>().get_model())>::Scalar;
    auto& title = get<QString>("title", profile.get_properties());
    auto& default_min = get<Type>("default_minimum", profile.get_properties());
    auto& default_max = get<Type>("default_maximum", profile.get_properties());
    auto& default_increment =
      get<Type>("default_increment", profile.get_properties());
    auto button = make_label_button(QString::fromUtf8("Click me"));
    auto min_model = std::make_shared<LocalScalarValueModel<optional<Type>>>(
      default_min.get());
    auto max_model = std::make_shared<LocalScalarValueModel<optional<Type>>>(
      default_max.get());
    button->connect_clicked_signal([&, button, min_model, max_model] {
      min_model->set_increment(default_increment.get());
      max_model->set_increment(default_increment.get());
      auto panel = new ScalarFilterPanel<B>(min_model, max_model,
        default_min.get(), default_max.get(), title.get(), button);
      auto filter_slot =
        profile.make_event_slot<QString>(QString::fromUtf8("SubmitSignal"));
      panel->connect_submit_signal([=] (const auto& min, const auto& max) {
        auto to_string = [&] (const optional<Type>& value) -> QString {
          if(value) {
            return displayTextAny(*value);
          }
          return QString::fromUtf8("null");
        };
        filter_slot(QString("%1, %2").arg(to_string(min)).arg(to_string(max)));
      });
      panel->show();
    });
    return button;
  }

  template<typename T>
  void populate_scalar_filter_panel_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties,
      T default_increment, const QString& default_title) {
    using Type = T;
    properties.push_back(make_standard_property("title", default_title));
    properties.push_back(make_standard_property("default_minimum", Type(1)));
    properties.push_back(make_standard_property("default_maximum", Type(10)));
    properties.push_back(
      make_standard_property("default_increment", default_increment));
  }

  optional<time_duration> parse_duration(const QString& duration) {
    try {
      return duration_from_string(duration.toStdString().c_str());
    } catch(const std::exception&) {
      return {};
    }
  }

  auto make_grid_image(const QSize& cell_size, int column_count,
      int row_count) {
    auto image = QImage(QSize(cell_size.width() * column_count,
      cell_size.height() * row_count), QImage::Format_RGB32);
    image.fill(QColor(0x56C4C5));
    auto painter = QPainter(&image);
    for(auto row = 0; row < row_count; ++row) {
      for(auto column = row % 2; column < column_count; column += 2) {
        auto cell_rect = QRect(QPoint(column * cell_size.width(),
          row * cell_size.width()), cell_size);
        painter.fillRect(cell_rect, QColor(0xA2218E));
        painter.fillRect(cell_rect - QMargins(scale_width(1), scale_height(1),
          scale_width(1), scale_height(1)),
          QColor(0xFDC777));
        painter.setPen(QColor(0x023888));
        cell_rect.translate(translate(5, 5) +
          QPoint(0, painter.fontMetrics().height()));
        painter.drawText(cell_rect.topLeft(),
          QString("(%1, %2)").arg(column).arg(row));
      }
    }
    return image;
  }
}

UiProfile Spire::make_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("border-size", 1));
  properties.push_back(make_standard_property("border-radius", 0));
  auto profile = UiProfile(QString::fromUtf8("Box"), properties,
    [] (auto& profile) {
      auto box = new Box(nullptr);
      box->setFixedSize(scale(100, 100));
      apply_widget_properties(box, profile.get_properties());
      auto& border_size = get<int>("border-size", profile.get_properties());
      auto& border_radius = get<int>("border-radius", profile.get_properties());
      auto style = StyleSheet();
      style.get(Any()).
        set(BackgroundColor(QColor(0xFFFFFF))).
        set(border(scale_width(border_size.get()), QColor(0xC8C8C8))).
        set(Styles::border_radius(scale_width(border_radius.get()))).
        set(horizontal_padding(scale_width(8)));
      style.get(Hover() || Focus()).
        set(border_color(QColor(0x4B23A0)));
      style.get(Disabled()).
        set(BackgroundColor(QColor(0xF5F5F5))).
        set(border_color(QColor(0xC8C8C8)));
      set_style(*box, std::move(style));
      border_size.connect_changed_signal([=, &border_size] (auto size) {
        auto style = get_style(*box);
        style.get(Any()).set(
          Styles::border_size(scale_width(border_size.get())));
        set_style(*box, std::move(style));
      });
      border_radius.connect_changed_signal([=, &border_radius] (auto radius) {
        auto style = get_style(*box);
        style.get(Any()).set(
          Styles::border_radius(scale_width(border_radius.get())));
        set_style(*box, std::move(style));
      });
      return box;
    });
  return profile;
}

UiProfile Spire::make_check_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_check_box_properties(properties);
  return UiProfile(QString::fromUtf8("CheckBox"), properties,
    [=] (auto& profile) {
      return setup_checkable_profile(profile, new CheckBox());
    });
}

UiProfile Spire::make_decimal_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_property("current", QString::fromUtf8("1")));
  properties.push_back(
    make_standard_property("minimum", QString::fromUtf8("-100")));
  properties.push_back(
    make_standard_property("maximum", QString::fromUtf8("100")));
  properties.push_back(make_standard_property("decimal_places", 2));
  properties.push_back(make_standard_property("leading_zeros", 0));
  properties.push_back(make_standard_property("trailing_zeros", 0));
  properties.push_back(
    make_standard_property("default_increment", QString::fromUtf8("1")));
  properties.push_back(
    make_standard_property("alt_increment", QString::fromUtf8("5")));
  properties.push_back(
    make_standard_property("ctrl_increment", QString::fromUtf8("10")));
  properties.push_back(
    make_standard_property("shift_increment", QString::fromUtf8("20")));
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property("read_only", false));
  properties.push_back(make_standard_property("buttons_visible", true));
  properties.push_back(make_standard_property("apply_sign_styling", false));
  properties.push_back(make_standard_property("apply_tick_styling", false));
  auto profile = UiProfile(QString::fromUtf8("DecimalBox"), properties,
    [] (auto& profile) {
      auto parse_decimal = [] (auto decimal) -> std::optional<Decimal> {
        try {
          return Decimal(decimal.toStdString().c_str());
        } catch(const std::exception&) {
          return {};
        }
      };
      auto model = std::make_shared<LocalScalarValueModel<optional<Decimal>>>();
      auto& minimum = get<QString>("minimum", profile.get_properties());
      minimum.connect_changed_signal([=] (const auto& value) {
        if(auto minimum = parse_decimal(value)) {
          model->set_minimum(*minimum);
        }
      });
      auto& maximum = get<QString>("maximum", profile.get_properties());
      maximum.connect_changed_signal([=] (const auto& value) {
        if(auto maximum = parse_decimal(value)) {
          model->set_maximum(*maximum);
        }
      });
      auto& decimal_places =
        get<int>("decimal_places", profile.get_properties());
      decimal_places.connect_changed_signal([=] (auto value) {
        model->set_increment(pow(Decimal(10), -value));
      });
      auto& default_increment =
        get<QString>("default_increment", profile.get_properties());
      auto& alt_increment =
        get<QString>("alt_increment", profile.get_properties());
      auto& ctrl_increment =
        get<QString>("ctrl_increment", profile.get_properties());
      auto& shift_increment =
        get<QString>("shift_increment", profile.get_properties());
      auto modifiers = QHash<Qt::KeyboardModifier, Decimal>(
        {{Qt::NoModifier, *parse_decimal(default_increment.get())},
         {Qt::AltModifier, *parse_decimal(alt_increment.get())},
         {Qt::ControlModifier, *parse_decimal(ctrl_increment.get())},
         {Qt::ShiftModifier, *parse_decimal(shift_increment.get())}});
      auto decimal_box = new DecimalBox(model, modifiers);
      decimal_box->setFixedWidth(scale_width(100));
      apply_widget_properties(decimal_box, profile.get_properties());
      auto& leading_zeros = get<int>("leading_zeros", profile.get_properties());
      leading_zeros.connect_changed_signal([=] (auto value) {
        auto style = get_style(*decimal_box);
        style.get(Any()).set(LeadingZeros(value));
        set_style(*decimal_box, std::move(style));
      });
      auto& trailing_zeros =
        get<int>("trailing_zeros", profile.get_properties());
      trailing_zeros.connect_changed_signal([=] (auto value) {
        auto style = get_style(*decimal_box);
        style.get(Any()).set(TrailingZeros(value));
        set_style(*decimal_box, std::move(style));
      });
      auto& current = get<QString>("current", profile.get_properties());
      current.connect_changed_signal([=] (const auto& value) {
        if(value == QString::fromUtf8("null")) {
          if(decimal_box->get_model()->get_current()) {
            decimal_box->get_model()->set_current(none);
          }
        } else if(auto decimal = parse_decimal(value)) {
          if(decimal_box->get_model()->get_current() != *decimal) {
            decimal_box->get_model()->set_current(*decimal);
          }
        }
      });
      auto current_slot =
        profile.make_event_slot<QString>(QString::fromUtf8("Current"));
      decimal_box->get_model()->connect_current_signal(
        [=, &current] (const optional<Decimal>& value) {
          auto text = [&] {
            if(value) {
              return QString::fromStdString(value->str(
                Decimal::backend_type::cpp_dec_float_digits10,
                std::ios_base::dec));
            }
            return QString::fromUtf8("null");
          }();
          current.set(text);
          current_slot(text);
        });
      auto submit_slot =
        profile.make_event_slot<QString>(QString::fromUtf8("Submit"));
      decimal_box->connect_submit_signal(
        [=] (const optional<Decimal>& submission) {
          if(submission) {
            submit_slot(QString::fromStdString(submission->str(
              Decimal::backend_type::cpp_dec_float_digits10,
              std::ios_base::dec)));
          } else {
            submit_slot(QString::fromUtf8("null"));
          }
        });
      auto reject_slot =
        profile.make_event_slot<QString>(QString::fromUtf8("Reject"));
      decimal_box->connect_reject_signal(
        [=] (const optional<Decimal>& value) {
          if(value) {
            reject_slot(QString::fromStdString(value->str(
              Decimal::backend_type::cpp_dec_float_digits10,
              std::ios_base::dec)));
          } else {
            reject_slot(QString::fromUtf8("null"));
          }
        });
      auto& placeholder =
        get<QString>("placeholder", profile.get_properties());
      placeholder.connect_changed_signal([=] (const auto& placeholder) {
        decimal_box->set_placeholder(placeholder);
      });
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal([=] (auto value) {
        decimal_box->set_read_only(value);
      });
      auto& buttons_visible =
        get<bool>("buttons_visible", profile.get_properties());
      buttons_visible.connect_changed_signal([=] (auto value) {
        auto style = get_style(*decimal_box);
        if(value) {
          style.get(Any() > is_a<Button>()).get_block().remove<Visibility>();
        } else {
          style.get(Any() > is_a<Button>()).set(Visibility::NONE);
        }
        set_style(*decimal_box, std::move(style));
      });
      auto& apply_sign_styling =
        get<bool>("apply_sign_styling", profile.get_properties());
      apply_sign_styling.connect_changed_signal([=] (auto value) {
        auto style = get_style(*decimal_box);
        if(value) {
          style.get(ReadOnly() && IsPositive()).
            set(TextColor(QColor(0x36BB55)));
          style.get(ReadOnly() && IsNegative()).
            set(TextColor(QColor(0xE63F44)));
        }
        set_style(*decimal_box, std::move(style));
      });
      auto& apply_tick_styling =
        get<bool>("apply_tick_styling", profile.get_properties());
      apply_tick_styling.connect_changed_signal([=] (auto value) {
        auto style = get_style(*decimal_box);
        if(value) {
          style.get(ReadOnly() && Uptick()).
            set(BackgroundColor(
              chain(timeout(QColor(0xEBFFF0), milliseconds(250)),
                linear(QColor(0xEBFFF0), revert, milliseconds(300)))));
          style.get(ReadOnly() && Downtick()).
            set(BackgroundColor(
              chain(timeout(QColor(0xFFF1F1), milliseconds(250)),
                linear(QColor(0xFFF1F1), revert, milliseconds(300)))));
        }
        set_style(*decimal_box, std::move(style));
      });
      return decimal_box;
    });
  return profile;
}

UiProfile Spire::make_decimal_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property("title",
    QString::fromUtf8("Filter by Decimal Number")));
  properties.push_back(make_standard_property("default_minimum",
    QString::fromUtf8("1.3")));
  properties.push_back(make_standard_property("default_maximum",
    QString::fromUtf8("10.67")));
  properties.push_back(make_standard_property("decimal_places", 2));
  properties.push_back(make_standard_property("leading_zeros", 0));
  properties.push_back(make_standard_property("trailing_zeros", 2));
  auto profile = UiProfile(QString::fromUtf8("DecimalFilterPanel"), properties,
    [] (auto& profile) {
      auto to_decimal = [] (auto decimal) -> boost::optional<Decimal> {
        try {
          return Decimal(decimal.toStdString().c_str());
        } catch(const std::exception&) {
          return {};
        }
      };
      auto to_string = [] (const optional<Decimal>& value) {
        if(value) {
          return QString::fromStdString(value->str(
            Decimal::backend_type::cpp_dec_float_digits10,
            std::ios_base::dec));
        }
        return QString::fromUtf8("null");
      };
      auto& title = get<QString>("title", profile.get_properties());
      auto& default_min =
        get<QString>("default_minimum", profile.get_properties());
      auto& default_max =
        get<QString>("default_maximum", profile.get_properties());
      auto& decimal_places =
        get<int>("decimal_places", profile.get_properties());
      auto& leading_zeros = get<int>("leading_zeros", profile.get_properties());
      auto& trailing_zeros =
        get<int>("trailing_zeros", profile.get_properties());
      auto button = make_label_button(QString::fromUtf8("Click me"));
      auto min_model =
        std::make_shared<LocalScalarValueModel<optional<Decimal>>>(
          to_decimal(default_min.get()));
      auto max_model =
        std::make_shared<LocalScalarValueModel<optional<Decimal>>>(
          to_decimal(default_max.get()));
      button->connect_clicked_signal([=, &decimal_places, &default_min,
          &default_max, &leading_zeros, &trailing_zeros, &profile, &title] {
        min_model->set_increment(pow(Decimal(10), -decimal_places.get()));
        max_model->set_increment(pow(Decimal(10), -decimal_places.get()));
        auto panel = new DecimalFilterPanel(min_model, max_model,
          to_decimal(default_min.get()), to_decimal(default_max.get()),
          title.get(), button);
        auto filter_slot =
          profile.make_event_slot<QString>(QString::fromUtf8("SubmitSignal"));
        panel->connect_submit_signal([=] (const auto& min, const auto& max) {
          filter_slot(to_string(min) + QString::fromUtf8(", ") +
            to_string(max));
        });
        auto style = get_style(*panel);
        style.get(Any() >> is_a<DecimalBox>()).set(
          LeadingZeros(leading_zeros.get()));
        style.get(Any() >> is_a<DecimalBox>()).set(
          TrailingZeros(trailing_zeros.get()));
        set_style(*panel, std::move(style));
        panel->show();
      });
      return button;
    });
  return profile;
}

UiProfile Spire::make_delete_icon_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile(QString::fromUtf8("DeleteIconButton"), properties,
    [] (auto& profile) {
      auto button = make_delete_icon_button();
      apply_widget_properties(button, profile.get_properties());
      button->connect_clicked_signal(
        profile.make_event_slot(QString::fromUtf8("ClickedSignal")));
      return button;
    });
  return profile;
}

UiProfile Spire::make_destination_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile(QString::fromUtf8("DestinationListItem"), properties,
    [] (auto& profile) {
      auto item = new DestinationListItem(
        GetDefaultDestinationDatabase().FromId(DefaultDestinations::TSX()));
      apply_widget_properties(item, profile.get_properties());
      return item;
    });
  return profile;
}

UiProfile Spire::make_drop_down_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("read_only", false));
  properties.push_back(make_standard_property("item_count", 15));
  properties.push_back(make_standard_property<QString>("item_label", "item"));
  auto profile = UiProfile(QString::fromUtf8("DropDownBox"), properties,
    [] (auto& profile) {
      auto& item_count = get<int>("item_count", profile.get_properties());
      auto& item_text = get<QString>("item_label", profile.get_properties());
      auto list_model = std::make_shared<ArrayListModel>();
      for(auto i = 0; i < item_count.get(); ++i) {
        list_model->push(item_text.get() + QString::fromUtf8("%1").arg(i));
      }
      auto list_view =
        new ListView(list_model, [] (const auto& model, auto index) {
          return make_label(model->get<QString>(index));
        });
      auto drop_down_box = new DropDownBox(*list_view);
      drop_down_box->setFixedWidth(scale_width(112));
      apply_widget_properties(drop_down_box, profile.get_properties());
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal([=] (auto is_read_only) {
        drop_down_box->set_read_only(is_read_only);
      });
      drop_down_box->connect_submit_signal(
        profile.make_event_slot<optional<std::any>>(
          QString::fromUtf8("Submit")));
      return drop_down_box;
    });
  return profile;
}

UiProfile Spire::make_drop_down_list_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property("item_count", 15));
  properties.push_back(make_standard_property<QString>("item_label", "item"));
  auto profile = UiProfile(QString::fromUtf8("DropDownList"), properties,
    [] (auto& profile) {
      auto& item_count = get<int>("item_count", profile.get_properties());
      auto& item_text = get<QString>("item_label", profile.get_properties());
      auto button = make_label_button("DropDownList");
      button->connect_clicked_signal([&, button] {
        auto list_model = std::make_shared<ArrayListModel>();
        for(auto i = 0; i < item_count.get(); ++i) {
          list_model->push(item_text.get() + QString::fromUtf8("%1").arg(i));
        }
        auto list_view =
          new ListView(list_model, [&] (const auto& model, auto index) {
            return make_label(model->get<QString>(index));
          });
        auto drop_down_list = new DropDownList(*list_view, button);
        drop_down_list->window()->setAttribute(Qt::WA_DeleteOnClose);
        drop_down_list->show();
      });
      return button;
    });
  return profile;
}

UiProfile Spire::make_duration_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", ""));
  properties.push_back(
    make_standard_property<QString>("minimum", "10:10:10.000"));
  properties.push_back(
    make_standard_property<QString>("maximum", "20:20:20.000"));
  properties.push_back(make_standard_property<bool>("read_only"));
  auto profile = UiProfile(QString::fromUtf8("DurationBox"), properties,
    [] (auto& profile) {
      auto model = std::make_shared<LocalOptionalDurationModel>();
      auto duration_box = new DurationBox(model);
      apply_widget_properties(duration_box, profile.get_properties());
      auto& minimum = get<QString>("minimum", profile.get_properties());
      minimum.connect_changed_signal([=] (auto value) {
        if(auto minimum_value = parse_duration(value)) {
          model->set_minimum(minimum_value);
        }
      });
      auto& maximum = get<QString>("maximum", profile.get_properties());
      maximum.connect_changed_signal([=] (auto value) {
        if(auto maximum_value = parse_duration(value)) {
          model->set_maximum(maximum_value);
        }
      });
      auto& current = get<QString>("current", profile.get_properties());
      current.connect_changed_signal([=] (auto value) {
        if(auto current_value = parse_duration(value)) {
          if(duration_box->get_model()->get_current() != *current_value) {
            duration_box->get_model()->set_current(*current_value);
          }
        }
      });
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal([=] (auto is_read_only) {
        duration_box->set_read_only(is_read_only);
      });
      duration_box->get_model()->connect_current_signal(
        profile.make_event_slot<optional<time_duration>>(
          QString::fromUtf8("Current")));
      duration_box->connect_submit_signal(
        profile.make_event_slot<optional<time_duration>>(
          QString::fromUtf8("Submit")));
      duration_box->connect_reject_signal(
        profile.make_event_slot<optional<time_duration>>(
          QString::fromUtf8("Reject")));
      return duration_box;
    });
  return profile;
}

UiProfile Spire::make_duration_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(
    make_standard_property<QString>("default_minimum", "10:10:10.000"));
  properties.push_back(
    make_standard_property<QString>("default_maximum", "20:20:20.000"));
  auto profile = UiProfile(QString::fromUtf8("DurationFilterPanel"), properties,
    [] (auto& profile) {
      auto& default_min=
        get<QString>("default_minimum", profile.get_properties());
      auto& default_max=
        get<QString>("default_maximum", profile.get_properties());
      auto button = make_label_button(QString::fromUtf8("Click me"));
      auto min_model =
        std::make_shared<LocalScalarValueModel<optional<time_duration>>>(
          parse_duration(default_min.get()));
      auto max_model =
        std::make_shared<LocalScalarValueModel<optional<time_duration>>>(
          parse_duration(default_max.get()));
      button->connect_clicked_signal([&, button, min_model, max_model] {
        auto panel = new ScalarFilterPanel<DurationBox>(min_model, max_model,
          parse_duration(default_min.get()), parse_duration(default_max.get()),
          QString::fromUtf8("Filter by Duration"), button);
        auto filter_slot =
          profile.make_event_slot<QString>(QString::fromUtf8("SubmitSignal"));
        panel->connect_submit_signal([=] (const auto& min, const auto& max) {
          auto to_string =
            [&] (const optional<time_duration>& value) -> QString {
              if(value) {
                return to_simple_string(*value).c_str();
              }
              return QString::fromUtf8("null");
            };
          filter_slot(QString("%1, %2").
            arg(to_string(min)).
            arg(to_string(max)));
        });
        panel->show();
      });
      return button;
    });
  return profile;
}

UiProfile Spire::make_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back( make_standard_property<QString>("title",
    QString::fromUtf8("Filter Quantity")));
  auto profile = UiProfile(QString::fromUtf8("FilterPanel"), properties,
    [] (auto& profile) {
      auto& title = get<QString>("title", profile.get_properties());
      auto button = make_label_button(QString::fromUtf8("Click me"));
      button->connect_clicked_signal([&, button] {
        auto component = new QWidget();
        component->setObjectName("component");
        component->setStyleSheet("#component {background-color: #F5F5F5;}");
        auto component_layout = new QGridLayout(component);
        component_layout->setSpacing(0);
        component_layout->setContentsMargins({});
        auto min_box = new TextBox(QString::fromUtf8("Min"));
        min_box->set_read_only(true);
        min_box->setFixedSize(scale(40, 30));
        component_layout->addWidget(min_box, 0, 0);
        auto min_text = new TextBox();
        min_text->setFixedSize(scale(120, 26));
        component_layout->addWidget(min_text, 0, 1);
        auto max_box = new TextBox(QString::fromUtf8("Max"));
        max_box->set_read_only(true);
        max_box->setFixedSize(scale(40, 30));
        component_layout->addWidget(max_box, 1, 0);
        auto max_text = new TextBox();
        max_text->setFixedSize(scale(120, 26));
        component_layout->addWidget(max_text, 1, 1);
        auto panel = new FilterPanel(title.get(), component, button);
        panel->window()->setAttribute(Qt::WA_DeleteOnClose);
        panel->connect_reset_signal(profile.make_event_slot(
          QString::fromUtf8("ResetSignal")));
        panel->show();
      });
      return button;
    });
  return profile;
}

UiProfile Spire::make_focus_observer_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto test_widget_property = define_enum<int>(
    {{"DurationBox", 0}, {"ListItem", 1}, {"LabelButton", 2}, {"ListView", 3}});
  properties.push_back(
    make_standard_enum_property("widget", test_widget_property));
  properties.push_back(make_standard_property("observer_count", 1));
  auto profile = UiProfile(QString::fromUtf8("FocusObserver"), properties,
    [] (auto& profile) {
      static auto observers = std::vector<std::shared_ptr<FocusObserver>>();
      observers.clear();
      auto filter_slot =
        profile.make_event_slot<QString>(QString::fromUtf8("StateSignal"));
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
          return new ListItem(make_label(QString::fromUtf8("ListItem")));
        } else if(value == 2) {
          return make_label_button("Label Button");
        } else {
          auto item_count = 10;
          auto list_model = std::make_shared<ArrayListModel>();
          for(auto i = 0; i < item_count; ++i) {
            list_model->push(QString::fromUtf8("Item%1").arg(i));
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
            if(auto& current = list_view->get_current_model()->get_current()) {
              list_view->get_current_model()->
                set_current((*current + 1) % item_count);
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

UiProfile Spire::make_icon_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("tooltip",
    "Tooltip"));
  auto profile = UiProfile(QString::fromUtf8("IconButton"), properties,
    [] (auto& profile) {
      auto& tooltip = get<QString>("tooltip", profile.get_properties());
      auto button =
        make_icon_button(imageFromSvg(":/Icons/demo.svg", scale(26, 26)),
          tooltip.get());
      apply_widget_properties(button, profile.get_properties());
      button->connect_clicked_signal(
        profile.make_event_slot(QString::fromUtf8("ClickedSignal")));
      return button;
    });
  return profile;
}

UiProfile Spire::make_info_tip_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<bool>("interactive"));
  properties.push_back(
    make_standard_property<int>("body-width", scale_width(100)));
  properties.push_back(
    make_standard_property<int>("body-height", scale_height(30)));
  auto profile = UiProfile(QString::fromUtf8("InfoTip"), properties,
    [] (auto& profile) {
      auto button = make_label_button("Hover me!");
      auto body_label = make_label("Body Label");
      auto label_style = get_style(*body_label);
      label_style.get(Any()).set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
      set_style(*body_label, label_style);
      auto info_tip = new InfoTip(body_label, button);
      apply_widget_properties(button, profile.get_properties());
      auto& interactive = get<bool>("interactive", profile.get_properties());
      interactive.connect_changed_signal([=] (bool is_interactive) {
        info_tip->set_interactive(is_interactive);
      });
      auto& body_width = get<int>("body-width", profile.get_properties());
      body_width.connect_changed_signal([=] (auto width) {
        body_label->setFixedWidth(width);
      });
      auto& body_height = get<int>("body-height", profile.get_properties());
      body_height.connect_changed_signal([=] (auto height) {
        body_label->setFixedHeight(height);
      });
      return button;
    });
  return profile;
}

UiProfile Spire::make_input_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("read_only", false));
  properties.push_back(
    make_standard_property<QString>("label", QString::fromUtf8("Label")));
  auto profile = UiProfile(QString::fromUtf8("InputBox"), properties,
    [] (auto& profile) {
      auto& label = get<QString>("label", profile.get_properties());
      auto input_box = make_input_box(make_label(label.get()));
      apply_widget_properties(input_box, profile.get_properties());
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal([=] (auto is_read_only) {
        if(is_read_only) {
          match(*input_box, ReadOnly());
        } else {
          unmatch(*input_box, ReadOnly());
        }
      });
      return input_box;
    });
  return profile;
}

UiProfile Spire::make_integer_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_decimal_box_properties<int>(properties);
  auto profile = UiProfile(QString::fromUtf8("IntegerBox"),
    properties, setup_decimal_box_profile<IntegerBox>);
  return profile;
}

UiProfile Spire::make_integer_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_scalar_filter_panel_properties<int>(
    properties, 1, QString::fromUtf8("Filter by Integer"));
  auto profile = UiProfile(QString::fromUtf8("IntegerFilterPanel"), properties,
    setup_scalar_filter_panel_profile<IntegerBox>);
  return profile;
}

UiProfile Spire::make_key_input_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current"));
  auto profile = UiProfile("KeyInputBox", properties, [] (auto& profile) {
    auto box = new KeyInputBox();
    box->setFixedSize(scale_width(100), scale_height(26));
    apply_widget_properties(box, profile.get_properties());
    auto& current = get<QString>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      if(value.isEmpty()) {
        if(box->get_current()->get_current() != QKeySequence()) {
          box->get_current()->set_current(QKeySequence());
        }
      } else {
        auto sequence = QKeySequence(value);
        if(sequence.count() != 0 && sequence[0] != Qt::Key::Key_unknown &&
            box->get_current()->get_current() != sequence) {
          box->get_current()->set_current(sequence);
        }
      }
    });
    box->get_current()->connect_current_signal(
      profile.make_event_slot<QKeySequence>(QString::fromUtf8("Current")));
    box->get_current()->connect_current_signal([&current] (const auto& value) {
      current.set(value.toString());
    });
    box->connect_submit_signal(
      profile.make_event_slot<QKeySequence>(QString::fromUtf8("Submit")));
    return box;
  });
  return profile;
}

UiProfile Spire::make_key_tag_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("key", "f1"));
  auto profile = UiProfile(QString::fromUtf8("KeyTag"), properties,
    [] (auto& profile) {
      auto key_tag = new KeyTag();
      apply_widget_properties(key_tag, profile.get_properties());
      auto& key = get<QString>("key", profile.get_properties());
      key.connect_changed_signal([=] (auto key_text) {
        auto key = [&] {
          if(key_text.toLower() == "alt") {
            return Qt::Key_Alt;
          } else if(key_text.toLower() == "ctrl") {
            return Qt::Key_Control;
          } else if(key_text.toLower() == "shift") {
            return Qt::Key_Shift;
          } else if(auto sequence = QKeySequence::fromString(key_text);
              !sequence.isEmpty()) {
            return Qt::Key(sequence[0]);
          }
          return Qt::Key_unknown;
        }();
        key_tag->get_model()->set_current(key);
      });
      return key_tag;
    });
  return profile;
}

UiProfile Spire::make_label_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_property<QString>("label", QString::fromUtf8("Click me!")));
  properties.push_back(make_standard_property<QColor>("pressed-color",
    QColor(0x4B23A0)));
  auto profile = UiProfile(QString::fromUtf8("LabelButton"), properties,
    [] (auto& profile) {
      auto& label = get<QString>("label", profile.get_properties());
      auto button = make_label_button(label.get());
      apply_widget_properties(button, profile.get_properties());
      auto& pressed_color = get<QColor>("pressed-color",
        profile.get_properties());
      pressed_color.connect_changed_signal([=] (const auto& color) {
        auto style = get_style(*button);
        style.get(Press() / Body()).
          set(BackgroundColor(color));
        set_style(*button, std::move(style));
      });
      button->connect_clicked_signal(
        profile.make_event_slot(QString::fromUtf8("ClickedSignal")));
      return button;
    });
  return profile;
}

UiProfile Spire::make_label_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(
    make_standard_property("label", QString::fromUtf8("Label")));
  auto profile = UiProfile(QString::fromUtf8("Label"), properties,
    [] (auto& profile) {
      auto& label = get<QString>("label", profile.get_properties());
      auto label_box = make_label(label.get());
      return label_box;
    });
  return profile;
}

UiProfile Spire::make_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("selected", false));
  auto profile = UiProfile(QString::fromUtf8("ListItem"), properties,
    [] (auto& profile) {
      auto item = new ListItem(make_label(QString::fromUtf8("Test Component")));
      item->setFixedWidth(scale_width(100));
      apply_widget_properties(item, profile.get_properties());
      item->connect_current_signal(
        profile.make_event_slot(QString::fromUtf8("Current")));
      item->connect_submit_signal(
        profile.make_event_slot(QString::fromUtf8("Submit")));
      auto& selected = get<bool>("selected", profile.get_properties());
      selected.connect_changed_signal([=] (auto value) {
        item->set_selected(value);
      });
      return item;
    });
  return profile;
}

UiProfile Spire::make_list_view_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("random_height_seed", 0));
  properties.push_back(make_standard_property("gap", 2));
  properties.push_back(make_standard_property("overflow_gap", 5));
  auto navigation_property = define_enum<EdgeNavigation>(
    {{"CONTAIN", EdgeNavigation::CONTAIN},
     {"WRAP", EdgeNavigation::WRAP}});
  properties.push_back(
    make_standard_enum_property("edge_navigation", navigation_property));
  auto direction_property = define_enum<Qt::Orientation>(
    {{"Vertical", Qt::Vertical}, {"Horizontal", Qt::Horizontal}});
  properties.push_back(
    make_standard_enum_property("direction", direction_property));
  auto overflow_property = define_enum<Overflow>(
    {{"WRAP", Overflow::WRAP}, {"NONE", Overflow::NONE}});
  properties.push_back(
    make_standard_enum_property("overflow", overflow_property));
  auto selection_mode_property = define_enum<SelectionMode>(
    {{"NONE", SelectionMode::NONE},
     {"SINGLE", SelectionMode::SINGLE}});
  properties.push_back(
    make_standard_enum_property("selection_mode", selection_mode_property));
  auto change_item_property = define_enum<int>({{"Delete", 0}, {"Add", 1}});
  properties.push_back(
    make_standard_enum_property("change_item", change_item_property));
  properties.push_back(make_standard_property("change_item_index", -1));
  properties.push_back(make_standard_property("current_item", -1));
  properties.push_back(make_standard_property("select_item", -1));
  properties.push_back(make_standard_property("disable_item", -1));
  properties.push_back(make_standard_property("enable_item", -1));
  properties.push_back(make_standard_property("auto_set_current_null", false));
  auto profile = UiProfile(QString::fromUtf8("ListView"), properties,
    [=] (auto& profile) {
      auto& random_height_seed =
        get<int>("random_height_seed", profile.get_properties());
      auto& direction =
        get<Qt::Orientation>("direction", profile.get_properties());
      auto& change_item = get<int>("change_item", profile.get_properties());
      auto& change_item_index =
        get<int>("change_item_index", profile.get_properties());
      auto random_generator = QRandomGenerator(random_height_seed.get());
      auto list_model = std::make_shared<ArrayListModel>();
      for(auto i = 0; i < 66; ++i) {
        if(i == 10) {
          list_model->push(QString::fromUtf8("llama"));
        } else if(i == 11) {
          list_model->push(QString::fromUtf8("llamb"));
        } else if(i == 12) {
          list_model->push(QString::fromUtf8("lllama"));
        } else if(i == 20) {
          list_model->push(QString::fromUtf8("llbma"));
        } else if(i == 30) {
          list_model->push(QString::fromUtf8("llxy"));
        } else {
          list_model->push(QString::fromUtf8("Item%1").arg(i));
        }
      }
      change_item_index.connect_changed_signal(
        [=, &change_item, index = 0] (auto value) mutable {
          if(value < 0 || value >= list_model->get_size()) {
            return;
          }
          if(change_item.get() == 0) {
            list_model->remove(value);
          } else {
            list_model->insert(
              QString::fromUtf8("newItem%1").arg(index++), value);
          }
        });
      auto list_view =
        new ListView(list_model, [&] (const auto& model, auto index) {
          auto label = make_label(model->get<QString>(index));
          if(random_height_seed.get() == 0) {
            auto random_size = random_generator.bounded(30, 70);
            if(direction.get() == Qt::Vertical) {
              label->setFixedHeight(scale_height(random_size));
            } else {
              label->setFixedWidth(scale_height(random_size));
            }
          }
          auto style = get_style(*label);
          style.get(+Any() << Disabled()).set(TextColor(QColor(0xFF0000)));
          set_style(*label, std::move(style));
          return label;
        });
      apply_widget_properties(list_view, profile.get_properties());
      auto& gap = get<int>("gap", profile.get_properties());
      gap.connect_changed_signal([=] (auto value) {
        if(value < 0) {
          return;
        }
        auto style = get_style(*list_view);
        style.get(Any()).set(ListItemGap(scale_width(value)));
        set_style(*list_view, std::move(style));
      });
      auto& overflow_gap = get<int>("overflow_gap", profile.get_properties());
      overflow_gap.connect_changed_signal([=] (auto value) {
        if(value < 0) {
          return;
        }
        auto style = get_style(*list_view);
        style.get(Any()).set(ListOverflowGap(scale_width(value)));
        set_style(*list_view, std::move(style));
      });
      direction.connect_changed_signal([=] (auto value) {
        auto style = get_style(*list_view);
        style.get(Any()).set(value);
        set_style(*list_view, std::move(style));
      });
      auto& overflow = get<Overflow>("overflow", profile.get_properties());
      overflow.connect_changed_signal([=] (auto value) {
        auto style = get_style(*list_view);
        style.get(Any()).set(value);
        set_style(*list_view, std::move(style));
      });
      auto& navigation =
        get<EdgeNavigation>("edge_navigation", profile.get_properties());
      navigation.connect_changed_signal([=] (auto value) {
        auto style = get_style(*list_view);
        style.get(Any()).set(value);
        set_style(*list_view, std::move(style));
      });
      auto& selection_mode =
        get<SelectionMode>("selection_mode", profile.get_properties());
      selection_mode.connect_changed_signal([=] (auto value) {
        auto style = get_style(*list_view);
        style.get(Any()).set(value);
        set_style(*list_view, std::move(style));
      });
      auto& current_item = get<int>("current_item", profile.get_properties());
      current_item.connect_changed_signal([=] (auto index) {
        if(index == -1) {
          list_view->get_current_model()->set_current(none);
        } else if(index >= 0 && index < list_model->get_size()) {
          list_view->get_current_model()->set_current(index);
        }
      });
      auto& select_item = get<int>("select_item", profile.get_properties());
      select_item.connect_changed_signal([=] (auto index) {
        if(index == -1) {
          list_view->get_selection_model()->set_current(none);
        } else if(index >= 0 && index < list_model->get_size()) {
          list_view->get_selection_model()->set_current(index);
        }
      });
      auto& disable_item = get<int>("disable_item", profile.get_properties());
      disable_item.connect_changed_signal([=] (auto value) {
        if(auto item = list_view->get_list_item(value)) {
          item->setDisabled(true);
        }
      });
      auto& enable_item = get<int>("enable_item", profile.get_properties());
      enable_item.connect_changed_signal([=] (auto value) {
        if(auto item = list_view->get_list_item(value)) {
          item->setDisabled(false);
        }
      });
      auto& auto_set_current_null = get<bool>("auto_set_current_null",
        profile.get_properties());
      list_view->get_current_model()->connect_current_signal(
        [&, list_view] (const auto& current) {
          if(current && auto_set_current_null.get()) {
            QTimer::singleShot(2000, [list_view] {
              list_view->get_current_model()->set_current(none);
            });
          }
        });
      list_view->get_current_model()->connect_current_signal(
        profile.make_event_slot<optional<int>>(QString::fromUtf8("Current")));
      list_view->get_selection_model()->connect_current_signal(
        profile.make_event_slot<optional<int>>(QString::fromUtf8("Selection")));
      list_view->connect_submit_signal(
        profile.make_event_slot<optional<std::any>>(
          QString::fromUtf8("Submit")));
      return list_view;
    });
  return profile;
}

UiProfile Spire::make_money_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_decimal_box_properties<Money>(
    properties, DecimalBoxProfileProperties(Money::ONE));
  auto profile = UiProfile(QString::fromUtf8("MoneyBox"),
    properties, setup_decimal_box_profile<MoneyBox>);
  return profile;
}

UiProfile Spire::make_money_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_scalar_filter_panel_properties<Money>(
    properties, Money::CENT, QString::fromUtf8("Filter by Money"));
  auto profile = UiProfile(QString::fromUtf8("MoneyFilterPanel"),
    properties, setup_scalar_filter_panel_profile<MoneyBox>);
  return profile;
}

UiProfile Spire::make_order_type_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_property = define_enum<OrderType>(
    {{"Limit", OrderType::LIMIT},
     {"Market", OrderType::MARKET},
     {"Pegged", OrderType::PEGGED},
     {"Stop", OrderType::STOP}});
  populate_enum_box_properties(properties, current_property);
  auto profile = UiProfile(QString::fromUtf8("OrderTypeBox"), properties,
    std::bind_front(setup_enum_box_profile<OrderTypeBox, make_order_type_box>));
  return profile;
}

UiProfile Spire::make_overlay_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property("close_on_blur", true));
  auto positioning_property = define_enum<OverlayPanel::Positioning>(
    {{"NONE", OverlayPanel::Positioning::NONE},
     {"PARENT", OverlayPanel::Positioning::PARENT}});
  properties.push_back(
    make_standard_enum_property("positioning", positioning_property));
  auto profile = UiProfile(QString::fromUtf8("OverlayPanel"), properties,
    [=] (auto& profile) {
      auto& close_on_blur =
        get<bool>("close_on_blur", profile.get_properties());
      auto& positioning =
        get<OverlayPanel::Positioning>("positioning", profile.get_properties());
      auto button = make_label_button(QString::fromUtf8("Click me"));
      auto close_on_blur_connection = std::make_shared<scoped_connection>();
      auto positioning_connection = std::make_shared<scoped_connection>();
      auto panel = QPointer<OverlayPanel>();
      button->connect_clicked_signal(
        [=, &profile, &close_on_blur, &positioning] () mutable {
          if(panel && !close_on_blur.get()) {
            return;
          }
          auto body = new QWidget();
          auto container_layout = new QVBoxLayout(body);
          container_layout->setSpacing(0);
          container_layout->setContentsMargins(
            scale_width(1), scale_height(1), scale_width(1), scale_height(1));
          auto title_layout = new QHBoxLayout();
          title_layout->setSpacing(scale_width(3));
          auto title_name = new QLabel(QString::fromUtf8("Filter Date"));
          title_layout->addWidget(title_name);
          auto close_button =
            make_icon_button(imageFromSvg(":/Icons/close.svg", scale(26, 26)));
          close_button->setFixedSize(scale(26, 26));
          close_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
          close_button->connect_clicked_signal([=] {
            close_button->window()->close();
          });
          title_layout->addWidget(close_button);
          container_layout->addLayout(title_layout);
          container_layout->addSpacing(scale_height(3));
          auto content_layout = new QGridLayout();
          content_layout->setSpacing(scale_width(5));
          content_layout->setContentsMargins(
            {scale_width(4), scale_height(4), scale_width(4), scale_height(4)});
          content_layout->addWidget(
            new QLabel(QString::fromUtf8("Start Date:")), 0, 0);
          auto text_box1 = new TextBox();
          text_box1->setFixedSize(scale(120, 26));
          content_layout->addWidget(text_box1, 0, 1);
          content_layout->addWidget(new QLabel(QString::fromUtf8("End Date:")),
            1, 0);
          auto text_box2 = new TextBox();
          text_box2->setFixedSize(scale(120, 26));
          content_layout->addWidget(text_box2, 1, 1);
          content_layout->addWidget(make_label_button(
            QString::fromUtf8("Reset")), 2, 1);
          container_layout->addLayout(content_layout);
          panel = new OverlayPanel(body, button);
          panel->setAttribute(Qt::WA_DeleteOnClose);
          panel->set_closed_on_blur(close_on_blur.get());
          panel->set_positioning(positioning.get());
          panel->show();
        });
      return button;
    });
  return profile;
}

UiProfile Spire::make_quantity_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto box_properties = DecimalBoxProfileProperties(Quantity(1));
  box_properties.m_minimum = Quantity(0);
  populate_decimal_box_properties<Quantity>(properties, box_properties);
  auto profile = UiProfile(QString::fromUtf8("QuantityBox"),
    properties, setup_decimal_box_profile<QuantityBox>);
  return profile;
}

UiProfile Spire::make_quantity_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_scalar_filter_panel_properties<Quantity>(properties, 1,
    QString::fromUtf8("Filter by Quantity"));
  auto profile = UiProfile(QString::fromUtf8("QuantityFilterPanel"),
    properties, setup_scalar_filter_panel_profile<QuantityBox>);
  return profile;
}

UiProfile Spire::make_radio_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_check_box_properties(properties);
  return UiProfile(QString::fromUtf8("RadioButton"), properties,
    [=] (auto& profile) {
      return setup_checkable_profile(profile, make_radio_button());
    });
}

UiProfile Spire::make_region_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto type_property = define_enum<int>(
    {{"SECURITY", 0}, {"MARKET", 1}, {"COUNTRY", 2}});
  properties.push_back(make_standard_enum_property("type", type_property));
  auto profile = UiProfile(QString::fromUtf8("RegionListItem"), properties,
    [] (auto& profile) {
      auto& type = get<int>("type", profile.get_properties());
      auto region = [&] {
        if(type.get() == 0) {
          auto security = ParseWildCardSecurity("MSFT.NSDQ",
            GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
          auto region = Region(*security);
          region.SetName("Microsoft Corporation");
          return region;
        } else if(type.get() == 1) {
          auto market =
            GetDefaultMarketDatabase().FromCode(DefaultMarkets::NSEX());
          auto region = Region(market);
          region.SetName(market.m_description);
          return region;
        } else {
          auto country_code = DefaultCountries::US();
          auto region = Region(country_code);
          region.SetName(
            GetDefaultCountryDatabase().FromCode(country_code).m_name);
          return region;
        }
      }();
      auto item = new RegionListItem(region);
      apply_widget_properties(item, profile.get_properties());
      item->setMinimumSize(0, 0);
      item->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
      return item;
    });
  return profile;
}

UiProfile Spire::make_scroll_bar_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("vertical", true));
  properties.push_back(make_standard_property("start_range", 0));
  properties.push_back(make_standard_property("end_range", 1000));
  properties.push_back(make_standard_property("page_size", 100));
  properties.push_back(make_standard_property("line_size", 10));
  properties.push_back(make_standard_property("position", 0));
  auto profile = UiProfile(QString::fromUtf8("ScrollBar"), properties,
    [] (auto& profile) {
      auto& vertical = get<bool>("vertical", profile.get_properties());
      auto orientation = [&] {
        if(vertical.get()) {
          return Qt::Vertical;
        } else {
          return Qt::Horizontal;
        }
      }();
      auto scroll_bar = new ScrollBar(orientation);
      if(orientation == Qt::Vertical) {
        scroll_bar->setFixedSize(scale(13, 200));
      } else {
        scroll_bar->setFixedSize(scale(200, 13));
      }
      apply_widget_properties(scroll_bar, profile.get_properties());
      auto& start_range = get<int>("start_range", profile.get_properties());
      start_range.connect_changed_signal([scroll_bar] (auto value) {
        auto range = scroll_bar->get_range();
        range.m_start = value;
        scroll_bar->set_range(range);
      });
      auto& end_range = get<int>("end_range", profile.get_properties());
      end_range.connect_changed_signal([scroll_bar] (auto value) {
        auto range = scroll_bar->get_range();
        range.m_end = value;
        scroll_bar->set_range(range);
      });
      auto& page_size = get<int>("page_size", profile.get_properties());
      page_size.connect_changed_signal([scroll_bar] (auto value) {
        scroll_bar->set_page_size(value);
      });
      auto& line_size = get<int>("line_size", profile.get_properties());
      line_size.connect_changed_signal([scroll_bar] (auto value) {
        scroll_bar->set_line_size(value);
      });
      auto& position = get<int>("position", profile.get_properties());
      position.connect_changed_signal([scroll_bar] (auto value) {
        scroll_bar->set_position(value);
      });
      scroll_bar->connect_position_signal(
        profile.make_event_slot<int>(QString::fromUtf8("Position")));
      return scroll_bar;
    });
  return profile;
}

UiProfile Spire::make_scroll_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto display_policy_property = define_enum<ScrollBox::DisplayPolicy>(
    {{"NEVER", ScrollBox::DisplayPolicy::NEVER},
     {"ALWAYS", ScrollBox::DisplayPolicy::ALWAYS},
     {"ON_OVERFLOW", ScrollBox::DisplayPolicy::ON_OVERFLOW},
     {"ON_ENGAGE", ScrollBox::DisplayPolicy::ON_ENGAGE}});
  properties.push_back(make_standard_enum_property(
    "horizontal_display_policy", display_policy_property));
  properties.push_back(make_standard_enum_property(
    "vertical_display_policy", display_policy_property));
  properties.push_back(make_standard_property("horizontal-padding", 10));
  properties.push_back(make_standard_property("vertical-padding", 10));
  properties.push_back(
    make_standard_property("border-color", QColor(0xC8C8C8)));
  properties.push_back(make_standard_property("rows", 10));
  properties.push_back(make_standard_property("columns", 10));
  auto profile = UiProfile(QString::fromUtf8("ScrollBox"), properties,
    [] (auto& profile) {
      auto label = new QLabel();
      auto& columns = get<int>("columns", profile.get_properties());
      auto& rows = get<int>("rows", profile.get_properties());
      label->setPixmap(QPixmap::fromImage(
        make_grid_image(scale(100, 100), columns.get(), rows.get())));
      auto scroll_box = new ScrollBox(label);
      scroll_box->setFixedSize(scale(320, 240));
      apply_widget_properties(scroll_box, profile.get_properties());
      auto& horizontal_display_policy = get<ScrollBox::DisplayPolicy>(
        "horizontal_display_policy", profile.get_properties());
      horizontal_display_policy.connect_changed_signal([=] (auto value) {
        scroll_box->set_horizontal(value);
      });
      auto& vertical_display_policy = get<ScrollBox::DisplayPolicy>(
        "vertical_display_policy", profile.get_properties());
      vertical_display_policy.connect_changed_signal([=] (auto value) {
        scroll_box->set_vertical(value);
      });
      auto& horizontal_padding =
        get<int>("horizontal-padding", profile.get_properties());
      horizontal_padding.connect_changed_signal([=] (auto padding) {
        auto style = get_style(*scroll_box);
        style.get(Any()).set(Styles::horizontal_padding(padding));
        set_style(*scroll_box, std::move(style));
      });
      auto& vertical_padding =
        get<int>("vertical-padding", profile.get_properties());
      vertical_padding.connect_changed_signal([=] (auto padding) {
        auto style = get_style(*scroll_box);
        style.get(Any()).set(Styles::vertical_padding(padding));
        set_style(*scroll_box, std::move(style));
      });
      auto& border_color =
        get<QColor>("border-color", profile.get_properties());
      border_color.connect_changed_signal([=] (const auto& color) {
        auto style = get_style(*scroll_box);
        style.get(Any()).set(border(scale_width(1), color));
        set_style(*scroll_box, std::move(style));
      });
      return scroll_box;
    });
  return profile;
}

UiProfile Spire::make_scrollable_list_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto direction_property = define_enum<Qt::Orientation>(
    {{"Vertical", Qt::Vertical}, {"Horizontal", Qt::Horizontal}});
  properties.push_back(
    make_standard_enum_property("direction", direction_property));
  auto overflow_property = define_enum<Overflow>(
    {{"NONE", Overflow::NONE}, {"WRAP", Overflow::WRAP}});
  properties.push_back(
    make_standard_enum_property("overflow", overflow_property));
  auto profile = UiProfile(QString::fromUtf8("ScrollableListBox"), properties,
    [] (auto& profile) {
      auto list_model = std::make_shared<ArrayListModel>();
      for(auto i = 0; i < 15; ++i) {
        list_model->push(QString::fromUtf8("Item%1").arg(i));
      }
      auto list_view = new ListView(list_model,
        [] (const auto& model, auto index) {
          return make_label(model->get<QString>(index));
        });
      auto scrollable_list_box = new ScrollableListBox(*list_view);
      apply_widget_properties(scrollable_list_box, profile.get_properties());
      auto& direction =
        get<Qt::Orientation>("direction", profile.get_properties());
      direction.connect_changed_signal([=] (auto value) {
        auto style = get_style(*list_view);
        style.get(Any()).set(value);
        set_style(*list_view, std::move(style));
      });
      auto& overflow = get<Overflow>("overflow", profile.get_properties());
      overflow.connect_changed_signal([=] (auto value) {
        auto style = get_style(*list_view);
        style.get(Any()).set(value);
        set_style(*list_view, std::move(style));
      });
      return scrollable_list_box;
    });
  return profile;
}

UiProfile Spire::make_search_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("placeholder"));
  auto profile = UiProfile(QString::fromUtf8("SearchBox"), properties,
    [] (auto& profile) {
      auto search_box = new SearchBox();
      search_box->setFixedWidth(scale_width(180));
      apply_widget_properties(search_box, profile.get_properties());
      auto& placeholder = get<QString>("placeholder", profile.get_properties());
      placeholder.connect_changed_signal([=] (const auto& text) {
        search_box->set_placeholder(text);
      });
      search_box->get_model()->connect_current_signal(
        profile.make_event_slot<QString>(QString::fromUtf8("Current")));
      search_box->connect_submit_signal(
        profile.make_event_slot<QString>(QString::fromUtf8("Submit")));
      return search_box;
    });
  return profile;
}

UiProfile Spire::make_security_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile(QString::fromUtf8("SecurityListItem"), properties,
    [] (auto& profile) {
      auto security = ParseWildCardSecurity("AB.NYSE",
        GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
      auto security_info = SecurityInfo(
        *security, "Alliancebernstein Holding LP", "", 0);
      auto item = new SecurityListItem(security_info);
      apply_widget_properties(item, profile.get_properties());
      return item;
    });
  return profile;
}

UiProfile Spire::make_side_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_property = define_enum<Side>(
    {{"Buy", Side::BID}, {"Sell", Side::ASK}});
  populate_enum_box_properties(properties, current_property);
  auto profile = UiProfile(QString::fromUtf8("SideBox"), properties,
    std::bind_front(setup_enum_box_profile<SideBox, make_side_box>));
  return profile;
}

UiProfile Spire::make_tag_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("label", "CAN"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile(QString::fromUtf8("Tag"), properties,
    [] (auto& profile) {
      auto& label = get<QString>("label", profile.get_properties());
      auto tag = new Tag(label.get());
      apply_widget_properties(tag, profile.get_properties());
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal([=] (auto is_read_only) {
        tag->set_read_only(is_read_only);
      });
      tag->connect_delete_signal(
        profile.make_event_slot(QString::fromUtf8("DeleteSignal")));
      tag->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
      tag->setMinimumSize(0, 0);
      return tag;
    });
  return profile;
}

UiProfile Spire::make_text_area_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current"));
  properties.push_back(make_standard_property<bool>("read_only"));
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property<int>("line-height", 120));
  auto horizontal_alignment_property = define_enum<Qt::Alignment>(
    {{"LEFT", Qt::AlignLeft},
     {"RIGHT", Qt::AlignRight},
     {"CENTER", Qt::AlignHCenter},
     {"JUSTIFY", Qt::AlignJustify}});
  properties.push_back(make_standard_enum_property(
    "horizontal-align", horizontal_alignment_property));
  auto profile = UiProfile(QString::fromUtf8("TextAreaBox"), properties,
    [] (auto& profile) {
      auto text_area_box = new TextAreaBox();
      apply_widget_properties(text_area_box, profile.get_properties());
      auto& current = get<QString>("current", profile.get_properties());
      current.connect_changed_signal([=] (const auto& value) {
        if(text_area_box->get_model()->get_current() != value) {
          text_area_box->get_model()->set_current(value);
        }
        text_area_box->adjustSize();
      });
      text_area_box->get_model()->connect_current_signal(
        [&] (const auto& value) {
          if(current.get() != value) {
            current.set(value);
          }
        });
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal([=] (auto is_read_only) {
        text_area_box->set_read_only(is_read_only);
      });
      auto& placeholder = get<QString>("placeholder", profile.get_properties());
      placeholder.connect_changed_signal([=] (const auto& text) {
        text_area_box->set_placeholder(text);
      });
      auto& line_height = get<int>("line-height", profile.get_properties());
      line_height.connect_changed_signal(
        [=] (auto line_height) {
          auto style = get_style(*text_area_box);
          style.get(Any()).set(LineHeight(
            static_cast<double>(line_height) / 100));
          set_style(*text_area_box, style);
        });
      auto& horizontal_alignment = get<Qt::Alignment>("horizontal-align",
        profile.get_properties());
      horizontal_alignment.connect_changed_signal(
        [&, text_area_box] (auto alignment) {
          auto style = get_style(*text_area_box);
          style.get(Any()).
            set(TextAlign(
              Qt::Alignment(alignment) | Qt::AlignTop));
          set_style(*text_area_box, std::move(style));
        });
      text_area_box->connect_submit_signal(profile.make_event_slot<QString>(
        QString::fromUtf8("Submit")));
      text_area_box->get_model()->connect_current_signal(
        profile.make_event_slot<QString>(QString::fromUtf8("Current")));
      return text_area_box;
    });
  return profile;
}

UiProfile Spire::make_text_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<bool>("read_only"));
  properties.push_back(make_standard_property<QString>("current"));
  properties.push_back(make_standard_property<QString>("placeholder"));
  auto profile = UiProfile(QString::fromUtf8("TextBox"), properties,
    [] (auto& profile) {
      auto text_box = new TextBox();
      text_box->setFixedWidth(scale_width(100));
      apply_widget_properties(text_box, profile.get_properties());
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal([=] (auto is_read_only) {
        text_box->set_read_only(is_read_only);
      });
      auto& current = get<QString>("current", profile.get_properties());
      current.connect_changed_signal([=] (const auto& current) {
        if(text_box->get_model()->get_current() != current) {
          text_box->get_model()->set_current(current);
        }
      });
      auto& placeholder = get<QString>("placeholder", profile.get_properties());
      placeholder.connect_changed_signal([=] (const auto& text) {
        text_box->set_placeholder(text);
      });
      text_box->get_model()->connect_current_signal(
        [&current] (const auto& value) {
          current.set(value);
        });
      text_box->get_model()->connect_current_signal(
        profile.make_event_slot<QString>(QString::fromUtf8("Current")));
      text_box->connect_submit_signal(profile.make_event_slot<QString>(
        QString::fromUtf8("Submit")));
      text_box->connect_reject_signal(profile.make_event_slot<QString>(
        QString::fromUtf8("Reject")));
      return text_box;
    });
  return profile;
}

UiProfile Spire::make_time_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", ""));
  properties.push_back(make_standard_property<bool>("read_only"));
  auto profile = UiProfile(QString::fromUtf8("TimeBox"), properties,
    [] (auto& profile) {
      auto parse_time = [] (auto time) -> boost::optional<time_duration> {
        try {
          return boost::posix_time::duration_from_string(
            time.toStdString().c_str());
        } catch(const std::exception&) {
          return {};
        }
      };
      auto& current = get<QString>("current", profile.get_properties());
      auto time_box = make_time_box();
      apply_widget_properties(time_box, profile.get_properties());
      current.connect_changed_signal([=] (auto value) {
        if(auto current_value = parse_time(value)) {
          if(time_box->get_model()->get_current() != *current_value) {
            time_box->get_model()->set_current(*current_value);
          }
        }
      });
      auto& read_only = get<bool>("read_only", profile.get_properties());
      read_only.connect_changed_signal([=] (auto is_read_only) {
        time_box->set_read_only(is_read_only);
      });
      time_box->get_model()->connect_current_signal(
        profile.make_event_slot<optional<time_duration>>(
          QString::fromUtf8("Current")));
      time_box->connect_submit_signal(
        profile.make_event_slot<optional<time_duration>>(
          QString::fromUtf8("Submit")));
      time_box->connect_reject_signal(
        profile.make_event_slot<optional<time_duration>>(
          QString::fromUtf8("Reject")));
      return time_box;
    });
  return profile;
}

UiProfile Spire::make_tooltip_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(
    make_standard_property("tooltip-text", QString::fromUtf8("Tooltip Text")));
  auto profile = UiProfile(QString::fromUtf8("Tooltip"), properties,
    [] (auto& profile) {
      auto label = make_label("Hover me!");
      auto& tooltip_text =
        get<QString>("tooltip-text", profile.get_properties());
      auto tooltip = new Tooltip(tooltip_text.get(), label);
      tooltip_text.connect_changed_signal([=] (const auto& text) {
        tooltip->set_label(text);
      });
      return label;
    });
  return profile;
}
