#include "Spire/UiViewer/StandardUiProfiles.hpp"
#include <stack>
#include <QFontDatabase>
#include <QImageReader>
#include <QLabel>
#include <QPainter>
#include <QPointer>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSpinBox>
#include <QStringBuilder>
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Spire/KeyBindings/OrderFieldInfoTip.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Spire/ScalarValueModelDecorator.hpp"
#include "Spire/Spire/TableValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Spire/ValidatedValueModel.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/Block.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/AdaptiveBox.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CalendarDatePicker.hpp"
#include "Spire/Ui/Checkbox.hpp"
#include "Spire/Ui/ClosedFilterPanel.hpp"
#include "Spire/Ui/ColorBox.hpp"
#include "Spire/Ui/ColorCodePanel.hpp"
#include "Spire/Ui/ColorPicker.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DateBox.hpp"
#include "Spire/Ui/DateFilterPanel.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/DeletableListItem.hpp"
#include "Spire/Ui/DestinationBox.hpp"
#include "Spire/Ui/DestinationListItem.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/EditableTableView.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/EyeDropper.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/FontBox.hpp"
#include "Spire/Ui/FontFamilyBox.hpp"
#include "Spire/Ui/FontStyleBox.hpp"
#include "Spire/Ui/HexColorBox.hpp"
#include "Spire/Ui/HighlightBox.hpp"
#include "Spire/Ui/HighlightPicker.hpp"
#include "Spire/Ui/HighlightSwatch.hpp"
#include "Spire/Ui/HoverObserver.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/InfoPanel.hpp"
#include "Spire/Ui/InfoTip.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Ui/KeyTag.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListSelectionModel.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/MarketBox.hpp"
#include "Spire/Ui/MenuButton.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/NavigationView.hpp"
#include "Spire/Ui/OrderTypeBox.hpp"
#include "Spire/Ui/OrderTypeFilterPanel.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/PercentBox.hpp"
#include "Spire/Ui/PopupBox.hpp"
#include "Spire/Ui/ProgressBar.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/RegionBox.hpp"
#include "Spire/Ui/RegionDropDownBox.hpp"
#include "Spire/Ui/RegionListItem.hpp"
#include "Spire/Ui/ResponsiveLabel.hpp"
#include "Spire/Ui/ScalarFilterPanel.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"
#include "Spire/Ui/SearchBox.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/SecurityListItem.hpp"
#include "Spire/Ui/SecurityView.hpp"
#include "Spire/Ui/SideBox.hpp"
#include "Spire/Ui/SideFilterPanel.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"
#include "Spire/Ui/Slider.hpp"
#include "Spire/Ui/Slider2D.hpp"
#include "Spire/Ui/SplitView.hpp"
#include "Spire/Ui/SubmenuItem.hpp"
#include "Spire/Ui/TabView.hpp"
#include "Spire/Ui/TableHeader.hpp"
#include "Spire/Ui/TableHeaderItem.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/Tag.hpp"
#include "Spire/Ui/TagBox.hpp"
#include "Spire/Ui/TagComboBox.hpp"
#include "Spire/Ui/TextAreaBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TimeInForceBox.hpp"
#include "Spire/Ui/TimeInForceFilterPanel.hpp"
#include "Spire/Ui/ToggleButton.hpp"
#include "Spire/Ui/Tooltip.hpp"
#include "Spire/Ui/TransitionView.hpp"
#include "Spire/Ui/Window.hpp"
#include "Spire/UiViewer/StandardUiProperties.hpp"
#include "Spire/UiViewer/UiProfile.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {

  /** Keeps a model synchronized with a property (and vice-versa). */
  template<typename T>
  void link(const std::shared_ptr<T>& model,
      TypedUiProperty<typename T::Type>& property) {
    property.connect_changed_signal([=] (auto value) {
      if(model->get() != value) {
        model->set(value);
      }
    });
    model->connect_update_signal([&] (auto value) {
      if(property.get() != value) {
        property.set(value);
      }
    });
  }

  /** Keeps a getter/setter synchronized with a property. */
  template<typename G, typename S, typename W, typename T>
  void link(const G& getter, const S& setter, W& widget,
      TypedUiProperty<T>& property) {
    if((widget.*getter)() != property.get()) {
      property.set((widget.*getter)());
    }
    property.connect_changed_signal([=, &widget] (auto value) {
      if((widget.*getter)() != value) {
        (widget.*setter)(value);
      }
    });
  }

  template<typename E>
  auto to_string_converter(
      const std::vector<std::pair<QString, E>>& definition) requires
      std::is_enum_v<E> {
    return [=] (E value) {
      for(auto e : definition) {
        if(e.second == value) {
          return e.first;
        }
      }
      return QString("N/A");
    };
  }

  QString to_string(const TableView::CurrentModel::Type index) {
    if(index) {
      return QString("(%1, %2)").arg(index->m_row).arg(index->m_column);
    }
    return QString("None");
  }

  QString to_string(const Decimal& value) {
    return QString::fromStdString(value.str(
      Decimal::backend_type::cpp_dec_float_digits10, std::ios_base::dec));
  }

  QString to_string(const HighlightColor& highlight) {
    return highlight.m_background_color.name() + " " +
      highlight.m_text_color.name();
  }

  void update_widget_style(QWidget& widget,
      const optional<StyleSheet>& styles) {
    if(!styles) {
      return;
    }
    update_style(widget, [&] (auto& style) {
      for(auto& rule : styles->get_rules()) {
        for(auto i = rule.get_block().begin(); i != rule.get_block().end();
            ++i) {
          style.get(rule.get_selector()).set(*i);
        }
      }
    });
  }

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
  auto create_decimal_box_profile(UiProfile& profile) {
    using Type =
      std::decay_t<decltype(*std::declval<B>().get_current())>::Scalar;
    auto model = std::make_shared<LocalScalarValueModel<optional<Type>>>();
    auto& minimum = get<Type>("minimum", profile.get_properties());
    minimum.connect_changed_signal([=] (auto value) {
      model->set_minimum(value);
    });
    auto& maximum = get<Type>("maximum", profile.get_properties());
    maximum.connect_changed_signal([=] (auto value) {
      model->set_maximum(value);
    });
    auto& min_max_enabled = get<bool>("min_max_enabled", profile.get_properties());
    min_max_enabled.connect_changed_signal([=, &minimum, &maximum] (auto value) {
      if(value) {
        model->set_minimum(minimum.get());
        model->set_maximum(maximum.get());
      } else {
        model->set_minimum(none);
        model->set_maximum(none);
      }
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
      if(box->get_current()->get() != value) {
        box->get_current()->set(value);
      }
    });
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& placeholder) {
      box->set_placeholder(placeholder);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    auto& buttons_visible =
      get<bool>("buttons_visible", profile.get_properties());
    read_only.connect_changed_signal([=, &buttons_visible] (auto value) {
      box->set_read_only(value);
      if(value) {
        buttons_visible.set(false);
      } else {
        buttons_visible.set(true);
      }
    });
    buttons_visible.connect_changed_signal([=] (auto value) {
      update_style(*box, [&] (auto& style) {
        if(value) {
          style.get(Any() > (DownButton() || UpButton())).
            set(Visibility::VISIBLE);
        } else {
          style.get(Any() > (DownButton() || UpButton())).
            set(Visibility::NONE);
        }
      });
    });
    return box;
  }

  template<typename B>
  auto setup_decimal_box_profile(UiProfile& profile) {
    using Type =
      std::decay_t<decltype(*std::declval<B>().get_current())>::Scalar;
    auto box = create_decimal_box_profile<B>(profile);
    box->get_current()->connect_update_signal(
      profile.make_event_slot<optional<Type>>("Current"));
    box->connect_submit_signal(
      profile.make_event_slot<optional<Type>>("Submit"));
    box->connect_reject_signal(
      profile.make_event_slot<optional<Type>>("Reject"));
    return box;
  }

  template<typename B>
  auto setup_decimal_box_with_decimal_profile(UiProfile& profile) {
    auto box = create_decimal_box_profile<B>(profile);
    auto& decimal_places =
      get<int>("decimal_places", profile.get_properties());
    decimal_places.connect_changed_signal([=] (auto value) {
      std::dynamic_pointer_cast<LocalScalarValueModel<optional<Decimal>>>(
        box->get_current())->set_increment(pow(Decimal(10), -value));
    });
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*box, styles);
    });
    auto& current = get<Decimal>("current", profile.get_properties());
    auto current_slot = profile.make_event_slot<QString>("Current");
    box->get_current()->connect_update_signal(
      [=, &current] (const optional<Decimal>& value) {
        auto text = [&] {
          if(value) {
            return to_string(*value);
          }
          return QString("null");
        }();
        if(value) {
          current.set(*value);
        }
        current_slot(text);
      });
    auto submit_slot = profile.make_event_slot<QString>("Submit");
    box->connect_submit_signal(
      [=] (const optional<Decimal>& submission) {
        if(submission) {
          submit_slot(to_string(*submission));
        } else {
          submit_slot(QString("null"));
        }
      });
    auto reject_slot = profile.make_event_slot<QString>("Reject");
    box->connect_reject_signal(
      [=] (const optional<Decimal>& value) {
        if(value) {
          submit_slot(to_string(*value));
        } else {
          reject_slot(QString("null"));
        }
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
      if(check_box->get_current()->get() != value) {
        check_box->get_current()->set(value);
      }
    });
    check_box->get_current()->connect_update_signal([&] (auto is_checked) {
      checked.set(is_checked);
    });
    check_box->get_current()->connect_update_signal(
      profile.make_event_slot<bool>("CheckedSignal"));
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
    properties.push_back(make_standard_property("label", QString("Click me!")));
    properties.push_back(make_standard_property<bool>("read-only"));
    properties.push_back(make_standard_property("left-to-right", true));
  }

  template<typename T>
  void populate_decimal_box_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties,
      const DecimalBoxProfileProperties<T>& box_properties) {
    using Type = T;
    properties.push_back(
      make_standard_property("current", box_properties.m_current));
    properties.push_back(
      make_standard_property("minimum", box_properties.m_minimum));
    properties.push_back(
      make_standard_property("maximum", box_properties.m_maximum));
    properties.push_back(make_standard_property(
      "default_increment", box_properties.m_default_increment));
    properties.push_back(make_standard_property(
      "alt_increment", box_properties.m_alt_increment));
    properties.push_back(make_standard_property(
      "ctrl_increment", box_properties.m_ctrl_increment));
    properties.push_back(make_standard_property(
      "shift_increment", box_properties.m_shift_increment));
    properties.push_back(make_standard_property<QString>("placeholder"));
    properties.push_back(make_standard_property("read_only", false));
    properties.push_back(make_standard_property("buttons_visible", true));
    properties.push_back(make_standard_property("min_max_enabled", true));
  }

  void populate_decimal_box_with_decimal_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties,
      const DecimalBoxProfileProperties<Decimal>& box_properties) {
    populate_decimal_box_properties<Decimal>(properties, box_properties);
    properties.push_back(make_standard_property("decimal_places", 4));
    auto default_style = R"(
      any {
        leading_zeros: 0;
        trailing_zeros: 0;
        text_align: left;
      }
      :read_only and :is_positive {
        text_color: 0x36BB55;
      }
      :read_only and :is_negative {
        text_color: 0xE63F44;
      }
      :read_only and :uptick {
        background_color: chain(timeout(0xEBFFF0, 250ms),
         linear(0xEBFFF0, revert, 300ms));
      }
      :read_only and :downtick {
        background_color: chain(timeout(0xFFF1F1, 250ms),
         linear(0xFFF1F1, revert, 300ms));
      }
    )";
    properties.push_back(make_style_property("style_sheet",
      std::move(default_style)));
  }

  template<typename T>
  void populate_decimal_box_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties) {
    populate_decimal_box_properties<T>(properties,
      DecimalBoxProfileProperties(1));
  }

  template<typename T,
    typename ClosedFilterPanel* (*f)(std::shared_ptr<ListModel<T>>, QWidget&)>
  auto setup_closed_filter_panel_profile(UiProfile& profile) {
    auto& properties = profile.get_properties();
    auto model = std::make_shared<ArrayListModel<T>>();
    for(auto property : properties) {
      if(get<bool>(property->get_name(), profile.get_properties()).get()) {
        model->push(*from_text<T>(property->get_name()));
      }
    }
    auto button = make_label_button("Click me");
    auto panel = f(model, *button);
    for(auto i = 0; i < static_cast<int>(properties.size()); ++i) {
      auto& checked =
        get<bool>(properties[i]->get_name(), profile.get_properties());
      checked.connect_changed_signal([=] (const auto& value) {
        if(panel->get_table()->get<bool>(i, 1) != value) {
          panel->get_table()->set(i, 1, value);
        }
      });
    }
    panel->get_table()->connect_operation_signal(
      [=, &profile] (const TableModel::Operation& operation) {
        visit(operation,
          [=, &profile] (const TableModel::UpdateOperation& operation) {
            auto value = panel->get_table()->get<bool>(operation.m_row, 1);
            auto& checked = get<bool>(properties[operation.m_row]->get_name(),
              profile.get_properties());
            if(checked.get() != value) {
              checked.set(value);
            }
          });
      });
    auto submit_filter_slot = profile.make_event_slot<QString>("SubmitSignal");
    panel->connect_submit_signal(
      [=] (const std::shared_ptr<AnyListModel>& submission) {
        auto result = QString();
        for(auto i = 0; i < submission->get_size(); ++i) {
          result += to_text(submission->get(i)) + " ";
        }
        submit_filter_slot(result);
      });
    button->connect_click_signal([=] { panel->show(); });
    return button;
  }

  template<typename B, typename B* (*F)(QWidget*)>
  auto setup_enum_box_profile(UiProfile& profile) {
    using Type = B::Type;
    auto box = F(nullptr);
    box->setFixedWidth(scale_width(150));
    apply_widget_properties(box, profile.get_properties());
    auto& current = get<Type>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      box->get_current()->set(value);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      box->set_read_only(is_read_only);
    });
    box->connect_submit_signal(profile.make_event_slot<std::any>("Submit"));
    return box;
  }

  template<typename T>
  void populate_enum_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties,
      const QString& property_name,
      std::vector<std::pair<QString, T>>& property) {
    properties.push_back(make_standard_enum_property(property_name, property));
  }

  void populate_font_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties,
      const QString& property_name) {
    auto font_database = QFontDatabase();
    auto font1 = font_database.font("Roboto", "Regular", -1);
    font1.setPixelSize(scale_width(12));
    auto font2 = font_database.font("Roboto", "Thin", -1);
    font2.setPixelSize(scale_width(12));
    auto font3 = font1;
    font3.setPixelSize(scale_width(8));
    auto font4 = font_database.font("Tahoma", "Bold", -1);
    font4.setPixelSize(scale_width(16));
    auto font5 = font_database.font("Segoe UI", "Light Italic", -1);
    font5.setPixelSize(scale_width(10));
    auto font_property = define_enum<QFont>(
      {{"Roboto, Regular, 12", font1}, {"Roboto, Thin, 12", font2},
      {"Roboto, Regular, 8", font3}, {"Tahoma, Bold, 16", font4},
      {"Segoe UI, Light Italic, 10", font5}});
    populate_enum_properties(properties, property_name, font_property);
  }

  template<typename B>
  auto setup_scalar_filter_panel_profile(UiProfile& profile) {
    using Panel = ScalarFilterPanel<B>;
    using Type = typename Panel::Type;
    auto button = make_label_button("Click me");
    auto range = std::make_shared<LocalValueModel<typename Panel::Range>>();
    button->connect_click_signal([=, &profile] {
      auto& title = get<QString>("title", profile.get_properties());
      auto panel = new Panel(range, title.get(), *button);
      auto filter_slot = profile.make_event_slot<QString>("SubmitSignal");
      panel->connect_submit_signal(
        [=] (const typename Panel::Range& submission) {
          auto to_string = [&] (const auto& value) {
            if(value) {
              return to_text(*value);
            }
            return QString("null");
          };
          filter_slot(QString("%1, %2").
            arg(to_string(submission.m_min)).
            arg(to_string(submission.m_max)));
        });
      panel->show();
    });
    return button;
  }

  void populate_scalar_filter_panel_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties,
      const QString& default_title) {
    properties.push_back(make_standard_property("title", default_title));
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
          scale_width(1), scale_height(1)), QColor(0xFDC777));
        painter.setPen(QColor(0x023888));
        cell_rect.translate(
          translate(5, 5) + QPoint(0, painter.fontMetrics().height()));
        painter.drawText(
          cell_rect.topLeft(), QString("(%1, %2)").arg(column).arg(row));
      }
    }
    return image;
  }

  auto create_panel_body(const QString& button_name) {
    auto body = new QWidget();
    body->setFixedSize(scale(200, 200));
    auto container_layout = new QVBoxLayout(body);
    container_layout->setSpacing(0);
    container_layout->setContentsMargins(scale_width(1),
      scale_height(1), scale_width(1), scale_height(1));
    auto create_button = make_label_button(button_name, body);
    container_layout->addWidget(create_button);
    auto close_button = make_label_button("Close", body);
    close_button->connect_click_signal([=] { body->window()->close(); });
    container_layout->addWidget(close_button);
    return body;
  }

  void create_child_panel(bool close_on_focus_out, bool draggable,
      OverlayPanel::Positioning positioning, Button* parent) {
    auto body = create_panel_body("Show child panel");
    auto panel = new OverlayPanel(*body, *parent);
    auto button = body->findChild<Button*>();
    button->connect_click_signal([=] {
      create_child_panel(close_on_focus_out, draggable, positioning, button);
    });
    panel->setAttribute(Qt::WA_DeleteOnClose);
    panel->set_closed_on_focus_out(close_on_focus_out);
    panel->set_is_draggable(draggable);
    panel->set_positioning(positioning);
    panel->show();
  }

  std::shared_ptr<SecurityInfoQueryModel> populate_security_query_model() {
    auto security_infos = std::vector<SecurityInfo>();
    security_infos.emplace_back(ParseSecurity("MRU.TSX"),
      "Metro Inc.", "", 0);
    security_infos.emplace_back(ParseSecurity("MG.TSX"),
      "Magna International Inc.", "", 0);
    security_infos.emplace_back(ParseSecurity("MGA.TSX"),
      "Mega Uranium Ltd.", "", 0);
    security_infos.emplace_back(ParseSecurity("MGAB.TSX"),
      "Mackenzie Global Fixed Income Alloc ETF", "", 0);
    security_infos.emplace_back(ParseSecurity("MON.NYSE"),
      "Monsanto Co.", "", 0);
    security_infos.emplace_back(ParseSecurity("MFC.TSX"),
      "Manulife Financial Corporation", "", 0);
    security_infos.emplace_back(ParseSecurity("MX.TSX"),
      "Methanex Corporation", "", 0);
    auto model = std::make_shared<LocalQueryModel<SecurityInfo>>();
    for(auto& security_info : security_infos) {
      model->add(to_text(security_info.m_security).toLower(), security_info);
      model->add(
        QString::fromStdString(security_info.m_name).toLower(), security_info);
    }
    return model;
  }

  auto populate_tag_box_model() {
    auto model = std::make_shared<ArrayListModel<QString>>();
    model->push("CAN");
    model->push("MSFT.NSDQ");
    model->push("XIU.TSX");
    return model;
  }

  auto populate_tag_combo_box_model() {
    auto model = std::make_shared<LocalQueryModel<QString>>();
    model->add("TSX");
    model->add("TSXV");
    model->add("TSO.ASX");
    model->add("TSU.TSX");
    model->add("TSN.TSXV");
    model->add("TSL.NYSE");
    model->add("MSFT.NSDQ");
    model->add("XDRX");
    model->add("XIU.TSX");
    model->add("AUS");
    model->add("CAN");
    model->add("CHN");
    model->add("JPN");
    model->add("USA");
    return model;
  }

  auto populate_region_box_model() {
    auto securities = std::vector<std::pair<std::string, std::string>>{
      {"MSFT.NSDQ", "Microsoft Corporation"},
      {"MG.TSX", "Magna International Inc."},
      {"MRU.TSX", "Metro Inc."},
      {"MFC.TSX", "Manulife Financial Corporation"},
      {"MX.TSX", "Methanex Corporation"},
      {"TSO.ASX", "Tesoro Resources Limited"}};
    auto markets = std::vector<MarketCode>{DefaultMarkets::NSEX(),
      DefaultMarkets::ISE(), DefaultMarkets::CSE(), DefaultMarkets::TSX(),
      DefaultMarkets::TSXV(), DefaultMarkets::BOSX()};
    auto countries = std::vector<CountryCode>{DefaultCountries::US(),
      DefaultCountries::CA(), DefaultCountries::AU(), DefaultCountries::JP(),
      DefaultCountries::CN()};
    auto model = std::make_shared<LocalQueryModel<Region>>();
    for(auto& security_info : securities) {
      auto security = ParseSecurity(security_info.first);
      auto region = Region(security);
      region.SetName(security_info.second);
      model->add(to_text(security).toLower(), region);
      model->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    for(auto& market_code : markets) {
      auto market = GetDefaultMarketDatabase().FromCode(market_code);
      auto region = Region(market);
      region.SetName(market.m_description);
      model->add(to_text(MarketToken(market.m_code)).toLower(), region);
      model->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    for(auto& country : countries) {
      auto region = Region(country);
      region.SetName(
        GetDefaultCountryDatabase().FromCode(country).m_name);
      model->add(to_text(country).toLower(), region);
      model->add(QString::fromStdString(region.GetName()).toLower(), region);
    }
    return model;
  }

  auto key_input_box_validator(const QKeySequence& sequence) {
    if(sequence.count() == 0) {
      return QValidator::Intermediate;
    } else if(sequence.count() > 1) {
      return QValidator::Invalid;
    }
    auto key = sequence[0];
    key &= ~Qt::ShiftModifier;
    key &= ~Qt::ControlModifier;
    key &= ~Qt::AltModifier;
    key &= ~Qt::MetaModifier;
    key &= ~Qt::KeypadModifier;
    key &= ~Qt::GroupSwitchModifier;
    if(key >= Qt::Key_F1 && key <= Qt::Key_F32) {
      return QValidator::Acceptable;
    }
    return QValidator::Invalid;
  }

  auto populate_key_input_box_model(const QKeySequence& key) {
    auto model = make_validated_value_model(&key_input_box_validator,
      std::make_shared<LocalKeySequenceValueModel>(key));
    return model;
  }

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

  auto parse_date(const QString& string) -> boost::optional<date> {
    try {
      auto parsed_date = from_string(string.toStdString());
      if(!parsed_date.is_not_a_date()) {
        return parsed_date;
      }
    } catch(const std::exception&) {}
    return {};
  }

  const auto& get_orientation_property() {
    static auto property = define_enum<Qt::Orientation>(
      {{"HORIZONTAL", Qt::Orientation::Horizontal},
       {"VERTICAL", Qt::Orientation::Vertical}});
    return property;
  }

  const auto& get_order_property() {
    static auto property = define_enum<TableHeaderItem::Order>(
      {{"NONE", TableHeaderItem::Order::NONE},
        {"UNORDERED", TableHeaderItem::Order::UNORDERED},
        {"ASCENDING", TableHeaderItem::Order::ASCENDING},
        {"DESCENDING", TableHeaderItem::Order::DESCENDING}});
    return property;
  }

  const auto& get_filter_property() {
    static auto property = define_enum<TableFilter::Filter>(
      {{"NONE", TableFilter::Filter::NONE},
        {"FILTERED", TableFilter::Filter::FILTERED},
        {"UNFILTERED", TableFilter::Filter::UNFILTERED}});
    return property;
  }

  const auto& get_preset_property() {
    static auto property = define_enum<ColorPicker::Preset>(
      {{"BASIC", ColorPicker::Preset::BASIC},
        {"WITH_TRANSPARENCY", ColorPicker::Preset::WITH_TRANSPARENCY},
        {"ADVANCED", ColorPicker::Preset::ADVANCED}});
    return property;
  }

  auto make_header_model() {
    auto model = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    model->push({"Name", "Name",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Order Type", "Ord Type",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Quantity", "Qty",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    model->push({"Key", "Key",
      TableHeaderItem::Order::NONE, TableFilter::Filter::UNFILTERED});
    return model;
  }

  EditableBox* make_row_cell(const std::shared_ptr<TableModel>& table,
      int row, int column) {
    if(column == 0) {
      return new EditableBox(
        *new TextBox(make_table_value_model<QString>(table, row, column)));
    } else if(column == 1) {
      return new EditableBox(*make_order_type_box(
        make_table_value_model<OrderType>(table, row, column)));
    } else if(column == 2) {
      auto model = make_scalar_value_model_decorator(
        make_table_value_model<optional<Quantity>>(table, row, column));
      return new EditableBox(*new QuantityBox(std::move(model)));
    } else if(column == 3) {
      return new EditableBox(
        *new KeyInputBox(make_validated_value_model(&key_input_box_validator,
          make_table_value_model<QKeySequence>(table, row, column))),
        [] (const auto& key) {
          return key_input_box_validator(key) == QValidator::Acceptable;
        });
    }
    return nullptr;
  }
}

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

UiProfile Spire::make_calendar_date_picker_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_date = boost::gregorian::day_clock::local_day();
  properties.push_back(
    make_standard_property("current", to_text(current_date)));
  properties.push_back(make_standard_property(
    "min", to_text(current_date - boost::gregorian::months(2))));
  properties.push_back(make_standard_property(
    "max", to_text(current_date + boost::gregorian::months(2))));
  auto profile = UiProfile("CalendarDatePicker", properties,
    [] (auto& profile) {
      auto model = std::make_shared<LocalOptionalDateModel>();
      auto& current = get<QString>("current", profile.get_properties());
      model->set(parse_date(current.get()));
      auto& min = get<QString>("min", profile.get_properties());
      if(auto min_date = parse_date(min.get())) {
        model->set_minimum(min_date);
      } else {
        model->set_minimum(date(1900, 1, 1));
      }
      auto& max = get<QString>("max", profile.get_properties());
      if(auto max_date = parse_date(max.get())) {
        model->set_maximum(*max_date);
      } else {
        model->set_maximum(date(2100, 12, 31));
      }
      auto calendar = new CalendarDatePicker(model);
      apply_widget_properties(calendar, profile.get_properties());
      current.connect_changed_signal([=] (const auto& value) {
        auto date = parse_date(value);
        if(date && !date->is_not_a_date() && *date != model->get()) {
          model->set(*date);
        }
      });
      calendar->get_current()->connect_update_signal([&current] (auto day) {
        if(day) {
          current.set(to_text(*day));
        }
      });
      calendar->get_current()->connect_update_signal(
        profile.make_event_slot<optional<date>>("Current"));
      calendar->connect_submit_signal(
        profile.make_event_slot<optional<date>>("Submit"));
      return calendar;
    });
  return profile;
}

UiProfile Spire::make_check_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_check_box_properties(properties);
  return UiProfile("CheckBox", properties, [] (auto& profile) {
    return setup_checkable_profile(profile, new CheckBox());
  });
}

UiProfile Spire::make_closed_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property("item_count", 7));
  properties.push_back(make_standard_property<QString>("item_label", "item"));
  properties.push_back(make_standard_property("checked_item", -1));
  properties.push_back(make_standard_property("unchecked_item", -1));
  properties.push_back(make_standard_property("insert_item", -1));
  properties.push_back(make_standard_property("remove_item", -1));
  auto profile = UiProfile("ClosedFilterPanel", properties, [] (auto& profile) {
    auto& item_count = get<int>("item_count", profile.get_properties());
    auto& item_text = get<QString>("item_label", profile.get_properties());
    auto model = std::make_shared<ArrayTableModel>();
    for(auto i = 0; i < item_count.get(); ++i) {
      model->push({item_text.get() + QString("%1").arg(i), false});
    }
    auto current_filter_slot =
      profile.make_event_slot<QString>("CurrentSignal");
    model->connect_operation_signal(
      [=] (const TableModel::Operation& operation) {
        visit(operation, [=] (const TableModel::UpdateOperation& operation) {
          auto result = QString();
          for(auto i = 0; i < model->get_row_size(); ++i) {
            if(model->get<bool>(i, 1)) {
              result += QString("%1 ").arg(i);
            }
          }
          current_filter_slot(result);
        });
      });
    auto& checked_item = get<int>("checked_item", profile.get_properties());
    checked_item.connect_changed_signal([=] (const auto& value) {
      if(value < 0 || value >= model->get_row_size()) {
        return;
      }
      model->set(value, 1, true);
    });
    auto& unchecked_item = get<int>("unchecked_item", profile.get_properties());
    unchecked_item.connect_changed_signal([=] (const auto& value) {
      if(value < 0 || value >= model->get_row_size()) {
        return;
      }
      model->set(value, 1, false);
    });
    auto& insert_item = get<int>("insert_item", profile.get_properties());
    insert_item.connect_changed_signal(
      [=, index = 0] (const auto& value) mutable {
        if(value < 0 || value > model->get_row_size()) {
          return;
        }
        model->insert({QString("newItem%1").arg(index++), false}, value);
      });
    auto& remove_item = get<int>("remove_item", profile.get_properties());
    remove_item.connect_changed_signal([=] (const auto& value) {
      if(value < 0 || value >= model->get_row_size()) {
        return;
      }
      model->remove(value);
    });
    auto button = make_label_button("Click me");
    auto panel = new ClosedFilterPanel(model, "Filter by something", *button);
    auto submit_filter_slot = profile.make_event_slot<QString>("SubmitSignal");
    panel->connect_submit_signal(
      [=] (const std::shared_ptr<AnyListModel>& submission) {
        auto result = QString();
        for(auto i = 0; i < submission->get_size(); ++i) {
          result += to_text(submission->get(i)) + " ";
        }
        submit_filter_slot(result);
      });
    button->connect_click_signal([=] { panel->show(); });
    return button;
  });
  return profile;
}

UiProfile Spire::make_color_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("read_only", false));
  properties.push_back(make_standard_property("current", QColor(0xF0D109)));
  properties.push_back(make_standard_enum_property("preset",
    get_preset_property()));
  auto profile = UiProfile("ColorBox", properties, [] (auto& profile) {
    auto& preset = get<ColorPicker::Preset>("preset", profile.get_properties());
    auto& current = get<QColor>("current", profile.get_properties());
    auto color_box = new ColorBox(
      std::make_shared<LocalColorModel>(current.get()), preset.get());
    color_box->setFixedSize(scale(100, 26));
    apply_widget_properties(color_box, profile.get_properties());
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&ColorBox::set_read_only, color_box));
    current.connect_changed_signal([=] (const auto& color) {
      if(color.isValid()) {
        color_box->get_current()->set(color);
      }
    });
    auto current_slot = profile.make_event_slot<QString>("Current");
    color_box->get_current()->connect_update_signal([=] (const auto& current) {
      current_slot(QString("Hex:%1 Alpha:%2").
        arg(current.name()).arg(std::round(current.alphaF() * 100)));
    });
    auto submit_slot = profile.make_event_slot<QString>("Submit");
    color_box->connect_submit_signal([=] (const auto& submission) {
      submit_slot(QString("Hex:%1 Alpha:%2").
        arg(submission.name()).arg(std::round(submission.alphaF() * 100)));
    });
    return color_box;
  });
  return profile;
}

UiProfile Spire::make_color_code_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QColor>("current"));
  auto mode_property = define_enum<ColorCodePanel::Mode>(
    {{"HEX", ColorCodePanel::Mode::HEX}, {"RGB", ColorCodePanel::Mode::RGB},
      {"HSB", ColorCodePanel::Mode::HSB}});
  properties.push_back(make_standard_enum_property("mode", mode_property));
  properties.push_back(make_standard_property("alpha_visible", true));
  auto profile = UiProfile("ColorCodePanel", properties, [] (auto& profile) {
    auto panel = new ColorCodePanel();
    apply_widget_properties(panel, profile.get_properties());
    auto& current = get<QColor>("current", profile.get_properties());
    current.connect_changed_signal([=] (const auto& color) {
      if(color.isValid()) {
        panel->get_current()->set(color);
      }
    });
    auto& mode = get<ColorCodePanel::Mode>("mode", profile.get_properties());
    mode.connect_changed_signal([=] (auto value) {
      panel->set_mode(value);
    });
    auto& alpha_visible = get<bool>("alpha_visible", profile.get_properties());
    alpha_visible.connect_changed_signal(
      std::bind_front(&ColorCodePanel::set_alpha_visible, panel));
    auto current_slot = profile.make_event_slot<QString>("Current");
    panel->get_current()->connect_update_signal([=] (const auto& current) {
      auto hue = [&] {
        if(current.hueF() < 0) {
          return 0.0;
        }
        return std::round(current.hueF() * 360);
      }();
      current_slot(
        QString("Hex:%1 Hue:%2 Saturation:%3 Brightness:%4 Alpha:%5").
          arg(current.name()).arg(hue).
          arg(std::round(current.saturationF() * 100)).
          arg(std::round(current.valueF() * 100)).
          arg(std::round(current.alphaF() * 100)));
    });
    return panel;
  });
  return profile;
}

UiProfile Spire::make_color_picker_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property("current", QColor()));
  properties.push_back(make_standard_enum_property("preset",
    get_preset_property()));
  auto profile = UiProfile("ColorPicker", properties, [] (auto& profile) {
    auto& preset = get<ColorPicker::Preset>("preset", profile.get_properties());
    auto button = make_label_button("ColorPicker");
    auto picker = new ColorPicker(preset.get(), *button);
    auto& current = get<QColor>("current", profile.get_properties());
    current.connect_changed_signal([=] (const auto& color) {
      if(color.isValid()) {
        picker->get_current()->set(color);
      }
    });
    auto current_slot = profile.make_event_slot<QString>("Current");
    picker->get_current()->connect_update_signal([=] (const auto& current) {
      current_slot(QString("Hex:%1 Alpha:%2").
        arg(current.name()).arg(std::round(current.alphaF() * 100)));
    });
    button->connect_click_signal([=] { picker->show(); });
    return button;
  });
  return profile;
}

UiProfile Spire::make_combo_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", "Car"));
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("ComboBox", properties, [] (auto& profile) {
    auto model = std::make_shared<LocalQueryModel<QString>>();
    model->add("Almond");
    model->add("Amber");
    model->add("Amberose");
    model->add("Apple");
    model->add("Beige");
    model->add("Bronze");
    model->add("Brown");
    model->add("Black");
    model->add("Car");
    auto& current = get<QString>("current", profile.get_properties());
    auto current_model =
      std::make_shared<LocalValueModel<QString>>(current.get());
    auto box =
      new ComboBox(model, current_model, &ListView::default_item_builder);
    box->setFixedWidth(scale_width(112));
    apply_widget_properties(box, profile.get_properties());
    auto current_connection = box->get_current()->connect_update_signal(
      profile.make_event_slot<std::any>("Current"));
    current.connect_changed_signal([=] (const auto& current) {
      if(auto value = model->parse(current)) {
        box->get_current()->set(*value);
      } else {
        auto current_blocker = shared_connection_block(current_connection);
        box->get_current()->set(current);
      }
    });
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& placeholder) {
      box->set_placeholder(placeholder);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&ComboBox<QString>::set_read_only, box));
    box->connect_submit_signal(profile.make_event_slot<QString>("Submit"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_context_menu_profile() {
  auto profile = UiProfile(QString::fromUtf8("ContextMenu"), {},
    [] (auto& profile) {
      auto button = make_label_button(QString::fromUtf8("Click me"));
      button->connect_click_signal([=, &profile] {
        auto menu = new ContextMenu(*button);
        menu->add_action(
          "Undo", profile.make_event_slot<>(QString("Action:Undo")));
        auto view_menu = new ContextMenu(*static_cast<QWidget*>(menu));
        view_menu->add_action(
          "Large", profile.make_event_slot<>(QString("Action:Large")));
        view_menu->add_action(
          "Medium", profile.make_event_slot<>(QString("Action:Medium")));
        view_menu->add_action(
          "Small", profile.make_event_slot<>(QString("Action:Small")));
        view_menu->add_separator();
        auto empty_menu = new ContextMenu(*static_cast<QWidget*>(view_menu));
        view_menu->add_menu("Empty", *empty_menu);
        menu->add_menu("View", *view_menu);
        auto sort_menu = new ContextMenu(*static_cast<QWidget*>(menu));
        sort_menu->add_action(
          "Name", profile.make_event_slot<>(QString("Action:Name")));
        sort_menu->add_action(
          "Size", profile.make_event_slot<>(QString("Action:Size")));
        auto type_menu = new ContextMenu(*static_cast<QWidget*>(sort_menu));
        type_menu->add_action(
          "Security", profile.make_event_slot<>(QString("Action:Security")));
        type_menu->add_action(
          "Side", profile.make_event_slot<>(QString("Action:Side")));
        sort_menu->add_menu("Type", *type_menu);
        menu->add_menu("Sort by", *sort_menu);
        menu->add_separator();
        menu->add_action(
          "Cut", profile.make_event_slot<>(QString("Action:Cut")));
        menu->add_action(
          "Copy", profile.make_event_slot<>(QString("Action:Copy")));
        menu->add_action(
          "Paste", profile.make_event_slot<>(QString("Action:Paste")));
        menu->add_separator();
        auto date_model = std::make_shared<LocalBooleanModel>();
        date_model->set(true);
        date_model->connect_update_signal(
          profile.make_event_slot<bool>(QString("Date CheckedSignal")));
        menu->add_check_box("Date", date_model);
        auto time_model = menu->add_check_box("Time");
        time_model->connect_update_signal(
          profile.make_event_slot<bool>(QString("Time CheckedSignal")));
        menu->add_separator();
        menu->add_action("This is a long name for test",
          profile.make_event_slot<>(
            QString("Action:This is a long name for test")));
        show_under_cursor(*menu);
      });
      return button;
    });
  return profile;
}

UiProfile Spire::make_date_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_date = day_clock::local_day();
  properties.push_back(
    make_standard_property("current", to_text(current_date)));
  properties.push_back(make_standard_property("format", DateFormat::YYYYMMDD));
  properties.push_back(make_standard_property("read_only", false));
  properties.push_back(
    make_standard_property("min", to_text(current_date - months(2))));
  properties.push_back(
    make_standard_property("max", to_text(current_date + months(2))));
  auto profile = UiProfile("DateBox", properties, [] (auto& profile) {
    auto model = std::make_shared<LocalOptionalDateModel>();
    model->connect_update_signal(
      profile.make_event_slot<optional<date>>("Current"));
    auto& current = get<QString>("current", profile.get_properties());
    model->connect_update_signal([&current] (const auto& value) {
      if(value) {
        current.set(QString::fromStdString(std::to_string(value->year()) +
          "-" + std::to_string(value->month()) + "-" +
          std::to_string(value->day())));
      } else {
        current.set("");
      }
    });
    current.connect_changed_signal([=] (const auto& current) {
      if(current.isEmpty()) {
        if(model->get()) {
          model->set(none);
        }
      } else {
        auto date = parse_date(current);
        if(date && model->get() != date) {
          model->set(date);
        }
      }
    });
    auto& min = get<QString>("min", profile.get_properties());
    min.connect_changed_signal([=] (const auto& min) {
      model->set_minimum(parse_date(min));
    });
    auto& max = get<QString>("max", profile.get_properties());
    max.connect_changed_signal([=] (const auto& max) {
      model->set_maximum(parse_date(max));
    });
    auto date_box = new DateBox(model);
    apply_widget_properties(date_box, profile.get_properties());
    date_box->connect_submit_signal(
      profile.make_event_slot<optional<date>>("Submit"));
    date_box->connect_reject_signal(
      profile.make_event_slot<optional<date>>("Reject"));
    auto& format = get<DateFormat>("format", profile.get_properties());
    format.connect_changed_signal([=] (auto format) {
      update_style(*date_box, [&] (auto& style) {
        style.get(Any()).set(format);
      });
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto value) {
      date_box->set_read_only(value);
    });
    return date_box;
  });
  return profile;
}

UiProfile Spire::make_date_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  auto current_date = day_clock::local_day();
  properties.push_back(make_standard_property(
    "default_start_date", to_text(current_date - months(3))));
  properties.push_back(
    make_standard_property("default_end_date", to_text(current_date)));
  properties.push_back(make_standard_property("default_offset_value", 1));
  auto default_unit_property = define_enum<DateFilterPanel::DateUnit>(
    {{"Day", DateFilterPanel::DateUnit::DAY},
     {"Week", DateFilterPanel::DateUnit::WEEK},
     {"Month", DateFilterPanel::DateUnit::MONTH},
     {"Year", DateFilterPanel::DateUnit::YEAR}});
  properties.push_back(make_standard_enum_property(
    "default_date_unit", default_unit_property));
  auto profile = UiProfile("DateFilterPanel", properties, [] (auto& profile) {
    auto& default_start_date =
      get<QString>("default_start_date", profile.get_properties());
    auto& default_end_date =
      get<QString>("default_end_date", profile.get_properties());
    auto& default_offset_value =
      get<int>("default_offset_value", profile.get_properties());
    auto& default_date_unit = get<DateFilterPanel::DateUnit>(
      "default_date_unit", profile.get_properties());
    auto button = make_label_button("Click me");
    auto model = std::make_shared<LocalValueModel<DateFilterPanel::DateRange>>();
    auto default_date_range = DateFilterPanel::DateRange();
    default_date_range.m_start = parse_date(default_start_date.get());
    default_date_range.m_end = parse_date(default_end_date.get());
    default_date_range.m_offset = DateFilterPanel::DateOffset{
      default_date_unit.get(), default_offset_value.get()};
    auto panel = new DateFilterPanel(model, default_date_range, *button);
    default_start_date.connect_changed_signal([=] (const auto& value) {
      auto range = panel->get_default_range();
      range.m_start = parse_date(value);
      panel->set_default_range(range);
    });
    default_end_date.connect_changed_signal([=] (const auto& value) {
      auto range = panel->get_default_range();
      range.m_end = parse_date(value);
      panel->set_default_range(range);
    });
    default_offset_value.connect_changed_signal([=] (const auto& value) {
      auto range = panel->get_default_range();
      range.m_offset->m_value = value;
      panel->set_default_range(range);
    });
    default_date_unit.connect_changed_signal([=] (const auto& value) {
      auto range = panel->get_default_range();
      range.m_offset->m_unit = value;
      panel->set_default_range(range);
    });
    auto filter_slot = profile.make_event_slot<QString>("SubmitSignal");
    panel->connect_submit_signal(
      [=] (const DateFilterPanel::DateRange& submission) {
        auto result = QString();
        if(submission.m_start) {
          result += to_text(*submission.m_start);
        } else {
          result += "none";
        }
        result += " - ";
        if(submission.m_end) {
          result += to_text(*submission.m_end);
        } else {
          result += "none";
        }
        if(submission.m_offset) {
          auto to_string = [] (auto unit) {
            if(unit == DateFilterPanel::DateUnit::DAY) {
              return "Day";
            } else if(unit == DateFilterPanel::DateUnit::WEEK) {
              return "Week";
            } else if(unit == DateFilterPanel::DateUnit::MONTH) {
              return "Month";
            } else {
              return "Year";
            }
          };
          result += QString("; %1 %2").
            arg(submission.m_offset->m_value).
            arg(to_string(submission.m_offset->m_unit));
        }
        filter_slot(result);
      });
    button->connect_click_signal([=] {
      panel->show();
    });
    return button;
  });
  return profile;
}

UiProfile Spire::make_decimal_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_decimal_box_with_decimal_properties(
    properties, DecimalBoxProfileProperties(Decimal(1)));
  auto profile = UiProfile("DecimalBox",
    properties, setup_decimal_box_with_decimal_profile<DecimalBox>);
  return profile;
}

UiProfile Spire::make_decimal_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(
    make_standard_property("title", QString("Filter Decimal")));
  auto profile = UiProfile("DecimalFilterPanel", properties,
    [] (auto& profile) {
      auto to_decimal = [] (auto decimal) -> optional<Decimal> {
        try {
          return Decimal(decimal.toStdString().c_str());
        } catch(const std::exception&) {
          return {};
        }
      };
      auto to_string = [] (const auto& value) {
        if(value) {
          return ::to_string(*value);
        }
        return QString("null");
      };
      auto& title = get<QString>("title", profile.get_properties());
      auto button = make_label_button("Click me");
      auto range = std::make_shared<
        LocalValueModel<ScalarFilterPanel<DecimalBox>::Range>>();
      button->connect_click_signal([=, &profile, &title] {
        auto panel = new DecimalFilterPanel(range, title.get(), *button);
        auto submit_slot = profile.make_event_slot<QString>("SubmitSignal");
        panel->connect_submit_signal([=] (const auto& submission) {
          submit_slot(to_string(submission.m_min) + QString(", ") +
            to_string(submission.m_max));
        });
        panel->show();
      });
      return button;
    });
  return profile;
}

UiProfile Spire::make_deletable_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("DeletableListItem", properties,
    [] (auto& profile) {
      auto item = new DeletableListItem(*make_label("XIU.TSX"));
      item->setFixedWidth(scale_width(120));
      apply_widget_properties(item, profile.get_properties());
      item->connect_delete_signal(profile.make_event_slot("DeleteSignal"));
      return item;
  });
  return profile;
}

UiProfile Spire::make_delete_icon_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("DeleteIconButton", properties, [] (auto& profile) {
    auto button = make_delete_icon_button();
    apply_widget_properties(button, profile.get_properties());
    button->connect_click_signal(profile.make_event_slot("ClickSignal"));
    return button;
  });
  return profile;
}

UiProfile Spire::make_destination_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", "TSX"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("DestinationBox", properties, [] (auto& profile) {
    auto selection = GetDefaultDestinationDatabase().SelectEntries(
      [] (auto& value) { return true; });
    auto destinations =
      std::make_shared<ArrayListModel<DestinationDatabase::Entry>>();
    for(auto& destination : selection) {
      destinations->push(destination);
    }
    auto& current = get<QString>("current", profile.get_properties());
    auto current_model =
      std::make_shared<LocalDestinationModel>(current.get().toStdString());
    auto box = make_destination_box(current_model, destinations);
    box->setFixedWidth(scale_width(112));
    apply_widget_properties(box, profile.get_properties());
    auto current_connection = box->get_current()->connect_update_signal(
      profile.make_event_slot<Destination>("Current"));
    current.connect_changed_signal([=] (const auto& current) {
      box->get_current()->set(current.toStdString());
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&DestinationBox::set_read_only, box));
    box->connect_submit_signal(profile.make_event_slot<Destination>("Submit"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_destination_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("DestinationListItem", properties,
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
  auto profile = UiProfile("DropDownBox", properties, [] (auto& profile) {
    auto& item_count = get<int>("item_count", profile.get_properties());
    auto& item_text = get<QString>("item_label", profile.get_properties());
    auto list_model = std::make_shared<ArrayListModel<QString>>();
    for(auto i = 0; i < item_count.get(); ++i) {
      list_model->push(item_text.get() + QString("%1").arg(i));
    }
    auto drop_down_box = new DropDownBox(list_model);
    drop_down_box->setFixedWidth(scale_width(112));
    apply_widget_properties(drop_down_box, profile.get_properties());
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      drop_down_box->set_read_only(is_read_only);
    });
    auto current_slot = profile.make_event_slot<optional<std::any>>("Current");
    drop_down_box->get_current()->set(list_model->get_size() - 1);
    drop_down_box->get_current()->connect_update_signal(
      [=] (auto current) {
        if(current) {
          current_slot(drop_down_box->get_list()->get(*current));
        } else {
          current_slot(none);
        }
      });
    drop_down_box->connect_submit_signal(
      profile.make_event_slot<optional<std::any>>("Submit"));
    return drop_down_box;
  });
  return profile;
}

UiProfile Spire::make_drop_down_list_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property("item_count", 15));
  properties.push_back(make_standard_property<QString>("item_label", "item"));
  auto profile = UiProfile("DropDownList", properties, [] (auto& profile) {
    auto& item_count = get<int>("item_count", profile.get_properties());
    auto& item_text = get<QString>("item_label", profile.get_properties());
    auto button = make_label_button("DropDownList");
    button->connect_click_signal([&, button] {
      auto list_model = std::make_shared<ArrayListModel<QString>>();
      for(auto i = 0; i < item_count.get(); ++i) {
        list_model->push(item_text.get() + QString("%1").arg(i));
      }
      auto list_view =
        new ListView(list_model,
          [&] (const std::shared_ptr<ListModel<QString>>& model, auto index) {
            return make_label(model->get(index));
          });
      auto drop_down_list = new DropDownList(*list_view, *button);
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
  auto profile = UiProfile("DurationBox", properties, [] (auto& profile) {
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
        if(duration_box->get_current()->get() != *current_value) {
          duration_box->get_current()->set(*current_value);
        }
      }
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      duration_box->set_read_only(is_read_only);
    });
    duration_box->get_current()->connect_update_signal(
      profile.make_event_slot<optional<time_duration>>("Current"));
    duration_box->connect_submit_signal(
      profile.make_event_slot<optional<time_duration>>("Submit"));
    duration_box->connect_reject_signal(
      profile.make_event_slot<optional<time_duration>>("Reject"));
    return duration_box;
  });
  return profile;
}

UiProfile Spire::make_duration_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  auto profile = UiProfile("DurationFilterPanel", properties,
    [] (auto& profile) {
      auto button = make_label_button(QString("Click me"));
      auto range =
        std::make_shared<LocalValueModel<DurationFilterPanel::Range>>();
      button->connect_click_signal([=, &profile] {
        auto panel =
          new DurationFilterPanel(range, "Filter by Duration", *button);
        auto filter_slot = profile.make_event_slot<QString>("SubmitSignal");
        panel->connect_submit_signal([=] (const DurationFilterPanel::Range& submission) {
          auto to_string =
            [&] (const auto& value) {
              if(value) {
                return QString::fromStdString(to_simple_string(*value));
              }
              return QString("null");
            };
          filter_slot(QString("%1, %2").
            arg(to_string(submission.m_min)).
            arg(to_string(submission.m_max)));
        });
        panel->show();
      });
      return button;
    });
  return profile;
}

UiProfile Spire::make_editable_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto test_widget_property = define_enum<int>(
    {{"TextBox", 0}, {"DropDownBox", 1}, {"DecimalBox", 2}, {"QuantityBox", 3},
      {"KeyInputBox", 4}, {"RegionBox", 5}});
  properties.push_back(
    make_standard_enum_property("input_box", test_widget_property));
  auto profile = UiProfile("EditableBox", properties, [] (auto& profile) {
    auto& test_widget = get<int>("input_box", profile.get_properties());
    auto input_box = [&] {
      auto value = test_widget.get();
      if(value == 0) {
        return new EditableBox(*new TextBox("TextBox"));
      } else if(value == 1) {
        auto list_model = std::make_shared<ArrayListModel<QString>>();
        for(auto i = 0; i < 5; ++i) {
          list_model->push(QString("item%1").arg(i));
        }
        return new EditableBox(*new DropDownBox(list_model));
      } else if(value == 2) {
        return new EditableBox(*new DecimalBox(
          std::make_shared<LocalOptionalDecimalModel>(Decimal(1))));
      } else if(value == 3) {
        return new EditableBox(*new QuantityBox(
          std::make_shared<LocalOptionalQuantityModel>(Quantity(1))));
      } else if(value == 4) {
        return new EditableBox(*new KeyInputBox(
          populate_key_input_box_model(QKeySequence("F1"))),
          [] (const auto& sequence) {
            return key_input_box_validator(sequence) == QValidator::Acceptable;
          });
      }
      auto query_model = populate_region_box_model();
      auto current = std::make_shared<LocalValueModel<Region>>();
      current->set(std::any_cast<Region>(*query_model->parse("TSX")));
      return new EditableBox(*new RegionBox(query_model, current));
    }();
    input_box->setMinimumWidth(scale_width(112));
    apply_widget_properties(input_box, profile.get_properties());
    return input_box;
  });
  return profile;
}

UiProfile Spire::make_editable_table_view_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(std::make_shared<
    StandardUiProperty<TableModel::RemoveOperation>>(
      "remove", TableModel::RemoveOperation(-1), [] (
          auto parent, auto& property) {
        auto form = new QWidget();
        auto layout = make_hbox_layout(form);
        auto row = new QSpinBox();
        auto button = new QPushButton("Remove");
        layout->addWidget(row);
        layout->addWidget(button);
        QObject::connect(button, &QPushButton::pressed, [=, &property] {
          property.set(TableModel::RemoveOperation(row->value()));
        });
        return form;
      }));
  auto profile = UiProfile("EditableTableView", properties, [] (auto& profile) {
    auto table = std::make_shared<ArrayTableModel>();
    table->push({QString("Test1"), OrderType(OrderType::MARKET),
      optional<Quantity>(10), QKeySequence("F3")});
    table->push({QString("Test2"), OrderType(OrderType::STOP),
      optional<Quantity>(20), QKeySequence("F7")});
    table->push({QString("Test3"), OrderType(OrderType::LIMIT),
      optional<Quantity>(30), QKeySequence("Ctrl+F2")});
    auto builder = EditableTableViewBuilder(table).
      set_header(make_header_model()).
      set_item_builder(make_row_cell);
    auto table_view = builder.make();
    apply_widget_properties(table_view, profile.get_properties());
    table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto& remove_operation =
      get<TableModel::RemoveOperation>("remove", profile.get_properties());
    remove_operation.connect_changed_signal([=] (const auto& operation) {
      if(operation.m_index >= 0 &&
          operation.m_index < table->get_row_size()) {
        table->remove(operation.m_index);
      }
    });
    return table_view;
  });
  return profile;
}

UiProfile Spire::make_eye_dropper_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property("popup_window", false));
  auto profile = UiProfile("EyeDropper", properties, [] (auto& profile) {
    auto create_eye_dropper = [] (QWidget* parent, UiProfile& profile) {
      auto eye_dropper = new EyeDropper(parent);
      eye_dropper->show();
      eye_dropper->get_current()->connect_update_signal(
        profile.make_event_slot<QColor>("Current"));
      eye_dropper->connect_submit_signal(
        profile.make_event_slot<QColor>("Submit"));
      eye_dropper->connect_reject_signal(
        profile.make_event_slot<QColor>("Reject"));
    };
    auto button = make_label_button("EyeDropper");
    auto& popup_window = get<bool>("popup_window", profile.get_properties());
    button->connect_click_signal([&, button] {
      if(popup_window.get()) {
        auto body = create_panel_body("EyeDropper");
        auto panel = new OverlayPanel(*body, *button);
        auto child_button = body->findChild<Button*>();
        child_button->connect_click_signal([=, &profile] {
          create_eye_dropper(child_button, profile);
        });
        panel->setAttribute(Qt::WA_DeleteOnClose);
        panel->set_closed_on_focus_out(true);
        panel->set_is_draggable(true);
        panel->set_positioning(OverlayPanel::Positioning::NONE);
        panel->show();
      } else {
        create_eye_dropper(button, profile);
      }
    });
    return button;
  });
  return profile;
}

UiProfile Spire::make_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(
    make_standard_property<QString>("title", QString("Filter Quantity")));
  auto profile = UiProfile("FilterPanel", properties, [] (auto& profile) {
    auto& title = get<QString>("title", profile.get_properties());
    auto button = make_label_button("Click me");
    button->connect_click_signal([&, button] {
      auto component = new QWidget();
      component->setObjectName("component");
      component->setStyleSheet("#component {background-color: #F5F5F5;}");
      auto component_layout = new QGridLayout(component);
      component_layout->setSpacing(0);
      component_layout->setContentsMargins({});
      auto min_box = new TextBox("Min");
      min_box->set_read_only(true);
      min_box->setFixedSize(scale(40, 30));
      component_layout->addWidget(min_box, 0, 0);
      auto min_text = new TextBox();
      min_text->setFixedSize(scale(120, 26));
      component_layout->addWidget(min_text, 0, 1);
      auto max_box = new TextBox("Max");
      max_box->set_read_only(true);
      max_box->setFixedSize(scale(40, 30));
      component_layout->addWidget(max_box, 1, 0);
      auto max_text = new TextBox();
      max_text->setFixedSize(scale(120, 26));
      component_layout->addWidget(max_text, 1, 1);
      auto panel = new FilterPanel(title.get(), component, *button);
      panel->window()->setAttribute(Qt::WA_DeleteOnClose);
      panel->connect_reset_signal(profile.make_event_slot("ResetSignal"));
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

UiProfile Spire::make_font_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_size_properties("parent_width", "parent_height", properties);
  populate_font_properties(properties, "current");
  auto profile = UiProfile("FontBox", properties, [] (auto& profile) {
    auto font_box = new FontBox();
    auto body_widget = new QWidget();
    auto layout = make_vbox_layout(body_widget);
    layout->addWidget(font_box);
    layout->addSpacing(scale_height(20));
    layout->addStretch(1);
    auto label = make_label("Handgloves");
    layout->addWidget(label);
    auto box = new Box(body_widget);
    update_style(*box, [] (auto& style) {
      style.get(Any()).set(border(scale_width(1), QColor(0x4B23A0)));
    });
    apply_widget_size_properties(box, "parent_width", "parent_height",
      profile.get_properties());
    box->setFixedWidth(scale_width(200));
    auto& current = get<QFont>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto& font) {
      if(font_box->get_current()->get() != font) {
        font_box->get_current()->set(font);
      }
    });
    auto current_slot =
      profile.make_event_slot<QString>(QString::fromUtf8("Current"));
    font_box->get_current()->connect_update_signal([=] (auto& font) {
      update_style(*label, [&] (auto& style) {
        style.get(Any()).set(Font(font));
      });
      current_slot(QString("%1, %2, %3").arg(font.family()).
        arg(QFontDatabase().styleString(font)).
        arg(unscale_width(font.pixelSize())));
    });
    return box;
  });
  return profile;
}

UiProfile Spire::make_font_family_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_property = define_enum<QString>(
    {{"Roboto", "Roboto"}, {"Source Sans Pro", "Source Sans Pro"},
    {"Tahoma", "Tahoma"}, {"Times New Roman", "Times New Roman"}});
  populate_enum_properties(properties, "current", current_property);
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("FontFamilyBox", properties, [] (auto& profile) {
    auto box = make_font_family_box("Roboto");
    box->setFixedWidth(scale_width(150));
    apply_widget_properties(box, profile.get_properties());
    auto& current = get<QString>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      box->get_current()->set(value);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&FontFamilyBox::set_read_only, box));
    box->get_current()->connect_update_signal(
      profile.make_event_slot<QString>("Current"));
    box->connect_submit_signal(profile.make_event_slot<QString>("Submit"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_font_style_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto font_family_property = define_enum<QString>(
    {{"Roboto", "Roboto"}, {"Source Sans Pro", "Source Sans Pro"},
    {"Tahoma", "Tahoma"}, {"Times New Roman", "Times New Roman"},
    {"System", "System"}, {"Cambria Math", "Cambria Math"},
    {"Webdings", "Webdings"}});
  populate_enum_properties(properties, "font_family", font_family_property);
  properties.push_back(make_standard_property<QString>("current", "Regular"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("FontStyleBox", properties, [] (auto& profile) {
    auto family_model = std::make_shared<LocalValueModel<QString>>("Roboto");
    auto box = make_font_style_box(family_model);
    box->setFixedWidth(scale_width(150));
    apply_widget_properties(box, profile.get_properties());
    auto& font_family = get<QString>("font_family", profile.get_properties());
    font_family.connect_changed_signal([=] (auto& value) {
      family_model->set(value);
    });
    auto& current = get<QString>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto& value) {
      auto styles = QFontDatabase().styles(family_model->get());
      if(styles.contains(value, Qt::CaseInsensitive)) {
        if(box->get_current()->get() != value) {
          box->get_current()->set(value);
        }
      }
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&FontStyleBox::set_read_only, box));
    box->get_current()->connect_update_signal(
      profile.make_event_slot<std::any>("Current"));
    box->get_current()->connect_update_signal([&current] (const auto& value) {
      current.set(value);
    });
    box->connect_submit_signal(profile.make_event_slot<std::any>("Submit"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_hex_color_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QColor>("current"));
  properties.push_back(make_standard_property<QColor>("rejected", Qt::red));
  auto profile = UiProfile("HexColorBox", properties, [] (auto& profile) {
    auto& rejected = get<QColor>("rejected", profile.get_properties());
    auto model = make_validated_value_model<QColor>([&] (QColor value) {
      if(value == rejected.get()) {
        return QValidator::State::Invalid;
      }
      return QValidator::State::Acceptable;
    });
    auto box = new HexColorBox(model);
    box->setFixedWidth(scale_width(120));
    apply_widget_properties(box, profile.get_properties());
    link(box->get_current(), get<QColor>("current", profile.get_properties()));
    box->get_current()->connect_update_signal(
      profile.make_event_slot<QColor>("Current"));
    box->connect_submit_signal(profile.make_event_slot<QColor>("Submit"));
    box->connect_reject_signal(profile.make_event_slot<QColor>("Reject"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_highlight_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("read_only", false));
  properties.push_back(
    make_standard_property("background_color", QColor(0xFFFFFF)));
  properties.push_back(make_standard_property("text_color", QColor(Qt::black)));
  populate_font_properties(properties, "font");
  auto profile = UiProfile("HighlightBox", properties, [] (auto& profile) {
    auto& background_color =
      get<QColor>("background_color", profile.get_properties());
    auto& text_color = get<QColor>("text_color", profile.get_properties());
    auto highlight_box = new HighlightBox(
      std::make_shared<LocalHighlightColorModel>(
        HighlightColor{background_color.get(), text_color.get()}));
    highlight_box->setFixedSize(scale(100, 26));
    apply_widget_properties(highlight_box, profile.get_properties());
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&HighlightBox::set_read_only, highlight_box));
    background_color.connect_changed_signal([=] (const auto& color) {
      auto highlight = highlight_box->get_current()->get();
      if(highlight.m_background_color.name() != color.name()) {
        highlight.m_background_color = color;
        highlight_box->get_current()->set(highlight);
      }
    });
    text_color.connect_changed_signal([=] (const auto& color) {
      auto highlight = highlight_box->get_current()->get();
      if(highlight.m_text_color.name() != color.name()) {
        highlight.m_text_color = color;
        highlight_box->get_current()->set(highlight);
      }
    });
    auto& font = get<QFont>("font", profile.get_properties());
    font.connect_changed_signal([=] (const auto& value) {
      update_style(*highlight_box, [&] (auto& style) {
        style.get(Any() > is_a<TextBox>()).set(Font(value));
      });
    });
    auto current_slot = profile.make_event_slot<QString>("Current");
    highlight_box->get_current()->connect_update_signal(
      [=, &background_color, &text_color] (const auto& highlight) {
        if(background_color.get().name() !=
            highlight.m_background_color.name()) {
          background_color.set(highlight.m_background_color);
        }
        if(text_color.get().name() != highlight.m_text_color.name()) {
          text_color.set(highlight.m_text_color);
        }
        current_slot(to_string(highlight));
      });
    auto submit_slot = profile.make_event_slot<QString>("Submit");
    highlight_box->connect_submit_signal([=] (const auto& submission) {
      submit_slot(to_string(submission));
    });
    return highlight_box;
  });
  return profile;
}

UiProfile Spire::make_highlight_picker_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(
    make_standard_property("background_color", QColor(0xFFFFC4)));
  properties.push_back(make_standard_property("text_color", QColor(0x521C00)));
  auto profile = UiProfile("HighlightPicker", properties, [] (auto& profile) {
    auto& background_color =
      get<QColor>("background_color", profile.get_properties());
    auto& text_color = get<QColor>("text_color", profile.get_properties());
    auto button = make_label_button("HighlightPicker");
    auto picker = new HighlightPicker(
      std::make_shared<LocalValueModel<HighlightPicker::Highlight>>(
        HighlightPicker::Highlight{background_color.get(), text_color.get()}),
      *button);
    background_color.connect_changed_signal([=] (const auto& color) {
      auto highlight = picker->get_current()->get();
      if(highlight.m_background_color.name() != color.name()) {
        highlight.m_background_color = color;
        picker->get_current()->set(highlight);
      }
      update_style(*button, [&] (auto& style) {
        style.get(Any() > Body()).set(BackgroundColor(color));
      });
    });
    text_color.connect_changed_signal([=] (const auto& color) {
      auto highlight = picker->get_current()->get();
      if(highlight.m_text_color.name() != color.name()) {
        highlight.m_text_color = color;
        picker->get_current()->set(highlight);
      }
      update_style(*button, [&] (auto& style) {
        style.get(Any() > Body()).set(TextColor(color));
      });
    });
    auto current_slot = profile.make_event_slot<QString>("Current");
    picker->get_current()->connect_update_signal(
      [=, &background_color, &text_color] (const auto& highlight) {
        if(background_color.get().name() !=
            highlight.m_background_color.name()) {
          background_color.set(highlight.m_background_color);
        }
        if(text_color.get().name() != highlight.m_text_color.name()) {
          text_color.set(highlight.m_text_color);
        }
        current_slot(to_string(highlight));
      });
    button->connect_click_signal([=] { picker->show(); });
    return button;
  });
  return profile;
}

UiProfile Spire::make_highlight_swatch_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_property("background_color", QColor(0xFFFFC4)));
  properties.push_back(make_standard_property("text_color", QColor(0x521C00)));
  auto profile = UiProfile("HighlightSwatch", properties, [] (auto& profile) {
    auto swatch = new HighlightSwatch();
    apply_widget_properties(swatch, profile.get_properties());
    auto& background_color =
      get<QColor>("background_color", profile.get_properties());
    background_color.connect_changed_signal([=] (auto color) {
      auto highlight = swatch->get_current()->get();
      highlight.m_background_color = color;
      swatch->get_current()->set(highlight);
    });
    auto& text_color = get<QColor>("text_color", profile.get_properties());
    text_color.connect_changed_signal([=] (auto color) {
      auto highlight = swatch->get_current()->get();
      highlight.m_text_color = color;
      swatch->get_current()->set(highlight);
    });
    return swatch;
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

UiProfile Spire::make_icon_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("tooltip", "Tooltip"));
  properties.push_back(make_standard_property<bool>("disable_on_click", false));
  auto profile = UiProfile("IconButton", properties, [] (auto& profile) {
    auto& tooltip = get<QString>("tooltip", profile.get_properties());
    auto button = make_icon_button(
      imageFromSvg(":/Icons/demo.svg", scale(26, 26)), tooltip.get());
    apply_widget_properties(button, profile.get_properties());
    auto& disable_on_click =
      get<bool>("disable_on_click", profile.get_properties());
    auto click_signal_slot = profile.make_event_slot("ClickSignal");
    button->connect_click_signal([=, &disable_on_click] () {
      button->setDisabled(disable_on_click.get());
      click_signal_slot();
    });
    return button;
  });
  return profile;
}

UiProfile Spire::make_icon_toggle_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("tooltip", "Tooltip"));
  auto profile = UiProfile("IconToggleButton", properties, [] (auto& profile) {
    auto& tooltip = get<QString>("tooltip", profile.get_properties());
    auto button = make_icon_toggle_button(
      imageFromSvg(":/Icons/demo.svg", scale(26, 26)), tooltip.get());
    apply_widget_properties(button, profile.get_properties());
    return button;
  });
  return profile;
}

UiProfile Spire::make_info_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("message", "Message"));
  auto severity_property = define_enum<InfoPanel::Severity>(
    {{"INFO", InfoPanel::Severity::INFO},
     {"SUCCESS", InfoPanel::Severity::SUCCESS},
     {"WARNING", InfoPanel::Severity::WARNING},
     {"ERROR", InfoPanel::Severity::ERROR}});
  properties.push_back(
    make_standard_enum_property("severity", severity_property));
  auto profile = UiProfile("InfoPanel", properties, [] (auto& profile) {
    auto button = make_label_button("Click me");
    auto& message = get<QString>("message", profile.get_properties());
    auto& severity =
      get<InfoPanel::Severity>("severity", profile.get_properties());
    auto info_panel = QPointer<InfoPanel>();
    button->connect_click_signal([=, &message, &severity] () mutable {
      if(info_panel) {
        return;
      }
      info_panel = new InfoPanel(severity.get(), message.get(), *button);
      info_panel->show();
    });
    return button;
  });
  return profile;
}

UiProfile Spire::make_info_tip_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_property<QString>("label", QString("Body Label")));
  properties.push_back(make_standard_property<bool>("interactive"));
  auto default_style = R"(
    any {
      padding_top: 8px;
      padding_right: 8px;
      padding_bottom: 8px;
      padding_left: 8px;
      border_size: 1px;
    }
    :read_only and :disabled {
      background_color: 0xFFFFFF;
    }
  )";
  properties.push_back(make_style_property("style_sheet",
    std::move(default_style)));
  auto profile = UiProfile("InfoTip", properties, [] (auto& profile) {
    auto button = make_label_button("Hover me!");
    auto body_label = make_label("");
    update_style(*body_label, [&] (auto& style) {
      style.get(Any()).set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    });
    auto info_tip = new InfoTip(body_label, button);
    apply_widget_properties(button, profile.get_properties());
    auto& label = get<QString>("label", profile.get_properties());
    label.connect_changed_signal([=] (auto value) {
      body_label->get_current()->set(value);
    });
    auto& interactive = get<bool>("interactive", profile.get_properties());
    interactive.connect_changed_signal([=] (bool is_interactive) {
      info_tip->set_interactive(is_interactive);
    });
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*info_tip, styles);
    });
    return button;
  });
  return profile;
}

UiProfile Spire::make_input_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("read_only", false));
  properties.push_back(make_standard_property<QString>("label", "Label"));
  auto profile = UiProfile("InputBox", properties, [] (auto& profile) {
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
  auto profile =
    UiProfile("IntegerBox", properties, setup_decimal_box_profile<IntegerBox>);
  return profile;
}

UiProfile Spire::make_integer_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_scalar_filter_panel_properties(
    properties, QString("Filter Integer"));
  auto profile = UiProfile("IntegerFilterPanel", properties,
    setup_scalar_filter_panel_profile<IntegerBox>);
  return profile;
}

UiProfile Spire::make_key_input_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", "F1"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("KeyInputBox", properties, [] (auto& profile) {
    auto& current = get<QString>("current", profile.get_properties());
    auto box = new KeyInputBox(populate_key_input_box_model(current.get()));
    box->setFixedWidth(scale_width(100));
    apply_widget_properties(box, profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      if(value.isEmpty()) {
        if(box->get_current()->get() != QKeySequence()) {
          box->get_current()->set(QKeySequence());
        }
      } else {
        auto sequence = QKeySequence(value);
        if(sequence.count() != 0 && sequence[0] != Qt::Key::Key_unknown &&
            box->get_current()->get() != sequence) {
          box->get_current()->set(sequence);
        }
      }
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&KeyInputBox::set_read_only, box));
    box->get_current()->connect_update_signal(
      profile.make_event_slot<QKeySequence>("Current"));
    box->get_current()->connect_update_signal([&current] (const auto& value) {
      current.set(value.toString());
    });
    box->connect_submit_signal(profile.make_event_slot<QKeySequence>("Submit"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_key_tag_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("key", "f1"));
  auto profile = UiProfile("KeyTag", properties, [] (auto& profile) {
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
      key_tag->get_current()->set(key);
    });
    return key_tag;
  });
  return profile;
}

UiProfile Spire::make_label_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_property<QString>("label", QString("Click me!")));
  auto default_style = R"(
    :press > :body {
      background_color: 0x7E71B8;
    }
  )";
  properties.push_back(make_style_property("style_sheet",
    std::move(default_style)));
  auto profile = UiProfile("LabelButton", properties, [] (auto& profile) {
    auto& label = get<QString>("label", profile.get_properties());
    auto button = make_label_button(label.get());
    apply_widget_properties(button, profile.get_properties());
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*button, styles);
    });
    button->connect_click_signal(profile.make_event_slot("ClickSignal"));
    return button;
  });
  return profile;
}

UiProfile Spire::make_label_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property("label", QString("Label")));
  auto profile = UiProfile("Label", properties, [] (auto& profile) {
    auto& label = get<QString>("label", profile.get_properties());
    auto label_box = make_label(label.get());
    return label_box;
  });
  return profile;
}

UiProfile Spire::make_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("current", false));
  properties.push_back(make_standard_property("selected", false));
  auto profile = UiProfile("ListItem", properties, [] (auto& profile) {
    auto item = new ListItem(*make_label("Test Component"));
    item->setFixedWidth(scale_width(100));
    apply_widget_properties(item, profile.get_properties());
    item->connect_submit_signal(profile.make_event_slot("Submit"));
    auto& current = get<bool>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      if(value) {
        match(*item, Current());
      } else {
        unmatch(*item, Current());
      }
    });
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
  properties.push_back(
    make_standard_enum_property("direction", get_orientation_property()));
  auto overflow_property = define_enum<Overflow>(
    {{"WRAP", Overflow::WRAP}, {"NONE", Overflow::NONE}});
  properties.push_back(
    make_standard_enum_property("overflow", overflow_property));
  auto selection_mode_property = define_enum<ListSelectionModel::Mode>(
    {{"NONE", ListSelectionModel::Mode::NONE},
     {"SINGLE", ListSelectionModel::Mode::SINGLE},
     {"MULTI", ListSelectionModel::Mode::MULTI}});
  properties.push_back(make_standard_enum_property("selection_mode",
    ListSelectionModel::Mode::SINGLE, selection_mode_property));
  auto change_item_property = define_enum<int>({{"Delete", 0}, {"Add", 1}});
  properties.push_back(
    make_standard_enum_property("change_item", change_item_property));
  properties.push_back(make_standard_property("change_item_index", -1));
  properties.push_back(make_standard_property("current_item", -1));
  properties.push_back(make_standard_property("disable_item", -1));
  properties.push_back(make_standard_property("enable_item", -1));
  properties.push_back(make_standard_property("auto_set_current_null", false));
  properties.push_back(make_standard_property("delete_submission", false));
  auto profile = UiProfile("ListView", properties, [=] (auto& profile) {
    auto& random_height_seed =
      get<int>("random_height_seed", profile.get_properties());
    auto& direction =
      get<Qt::Orientation>("direction", profile.get_properties());
    auto& change_item = get<int>("change_item", profile.get_properties());
    auto& change_item_index =
      get<int>("change_item_index", profile.get_properties());
    auto random_generator = QRandomGenerator(random_height_seed.get());
    auto list_model = std::make_shared<ArrayListModel<QString>>();
    for(auto i = 0; i < 66; ++i) {
      if(i == 1) {
        list_model->push("ala");
      } else if(i == 5) {
        list_model->push("alb");
      } else if(i == 6) {
        list_model->push("abc");
      } else if(i == 10) {
        list_model->push("llama");
      } else if(i == 11) {
        list_model->push("llamb");
      } else if(i == 12) {
        list_model->push("lllama");
      } else if(i == 20) {
        list_model->push("llbma");
      } else if(i == 30) {
        list_model->push("llxy");
      } else {
        list_model->push(QString("Item%1").arg(i));
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
          list_model->insert(QString("newItem%1").arg(index++), value);
        }
      });
    auto selection_model = std::make_shared<ListSelectionModel>();
    auto list_view =
      new ListView(list_model, selection_model,
        [&] (const std::shared_ptr<ListModel<QString>>& model, auto index) {
          auto label = make_label(model->get(index));
          if(random_height_seed.get() == 0) {
            auto random_size = random_generator.bounded(30, 70);
            if(direction.get() == Qt::Vertical) {
              label->setFixedHeight(scale_height(random_size));
            } else {
              label->setFixedWidth(scale_height(random_size));
            }
          }
          update_style(*label, [&] (auto& style) {
            style.get(+Any() << Disabled()).set(TextColor(QColor(0xFF0000)));
          });
          return label;
        });
    apply_widget_properties(list_view, profile.get_properties());
    auto& gap = get<int>("gap", profile.get_properties());
    gap.connect_changed_signal([=] (auto value) {
      if(value < 0) {
        return;
      }
      update_style(*list_view, [&] (auto& style) {
        style.get(Any()).set(ListItemGap(scale_width(value)));
      });
    });
    auto& overflow_gap = get<int>("overflow_gap", profile.get_properties());
    overflow_gap.connect_changed_signal([=] (auto value) {
      if(value < 0) {
        return;
      }
      update_style(*list_view, [&] (auto& style) {
        style.get(Any()).set(ListOverflowGap(scale_width(value)));
      });
    });
    direction.connect_changed_signal([=] (auto value) {
      update_style(*list_view, [&] (auto& style) {
        style.get(Any()).set(value);
      });
    });
    auto& overflow = get<Overflow>("overflow", profile.get_properties());
    overflow.connect_changed_signal([=] (auto value) {
      update_style(*list_view, [&] (auto& style) {
        style.get(Any()).set(value);
      });
    });
    auto& selection_mode =
      get<ListSelectionModel::Mode>("selection_mode", profile.get_properties());
    selection_mode.connect_changed_signal([=] (auto value) {
      selection_model->set_mode(value);
    });
    auto& navigation =
      get<EdgeNavigation>("edge_navigation", profile.get_properties());
    navigation.connect_changed_signal([=] (auto value) {
      update_style(*list_view, [&] (auto& style) {
        style.get(Any()).set(value);
      });
    });
    auto& current_item = get<int>("current_item", profile.get_properties());
    current_item.connect_changed_signal([=] (auto index) {
      if(index == -1) {
        list_view->get_current()->set(none);
      } else if(index >= 0 && index < list_model->get_size()) {
        list_view->get_current()->set(index);
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
    auto& auto_set_current_null =
      get<bool>("auto_set_current_null", profile.get_properties());
    list_view->get_current()->connect_update_signal(
      [&, list_view] (const auto& current) {
        if(current && auto_set_current_null.get()) {
          QTimer::singleShot(2000, [list_view] {
            list_view->get_current()->set(none);
          });
        }
      });
    list_view->get_current()->connect_update_signal(
      profile.make_event_slot<optional<int>>("Current"));
    list_view->get_selection()->connect_operation_signal(
      profile.make_event_slot<AnyListModel::Operation>("Selection"));
    list_view->connect_submit_signal(
      profile.make_event_slot<optional<std::any>>("Submit"));
    if(get<bool>("delete_submission", profile.get_properties()).get()) {
      list_view->connect_submit_signal([=] (auto& value) {
        if(auto index = list_view->get_current()->get()) {
          list_model->remove(*index);
        }
      });
    }
    return list_view;
  });
  return profile;
}

UiProfile Spire::make_market_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", "ARCX"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("MarketBox", properties, [] (auto& profile) {
    auto& current = get<QString>("current", profile.get_properties());
    auto current_model = std::make_shared<LocalMarketModel>(
      current.get().toStdString());
    auto box = make_market_box(current_model, GetDefaultMarketDatabase());
    box->setFixedWidth(scale_width(112));
    apply_widget_properties(box, profile.get_properties());
    current.connect_changed_signal([=] (const auto& current) {
      if(current.length() != 4) {
        return;
      }
      auto code = MarketCode(current.toUpper().toStdString().c_str());
      auto& market = GetDefaultMarketDatabase().FromCode(code);
      if(!market.m_code.IsEmpty()) {
        box->get_current()->set(code);
      }
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&MarketBox::set_read_only, box));
    box->get_current()->connect_update_signal(
      profile.make_event_slot<MarketToken>("Current"));
    box->connect_submit_signal(profile.make_event_slot<MarketToken>("Submit"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_menu_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("MenuButton", properties, [] (auto& profile) {
    auto label = make_label("MenuButton");
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    update_style(*label, [] (auto& style) {
      style.get(Any()).
        set(border(scale_width(1), QColor(0xC8C8C8))).
        set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
        set(horizontal_padding(scale_width(8))).
        set(vertical_padding(scale_height(5)));
    });
    auto menu_button = new MenuButton(*label);
    menu_button->setFixedWidth(scale_width(120));
    auto& menu = menu_button->get_menu();
    menu.add_action("Minimize All",
      profile.make_event_slot<>(QString("Action:Minimize All")));
    menu.add_action("Restore All",
      profile.make_event_slot<>(QString("Action:Restore All")));
    menu.add_action("This is a long name for test",
      profile.make_event_slot<>(
        QString("Action:This is a long name for test")));
    apply_widget_properties(menu_button, profile.get_properties());
    return menu_button;
  });
  return profile;
}

UiProfile Spire::make_menu_label_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("MenuLabelButton", properties, [] (auto& profile) {
    auto menu_button = make_menu_label_button("Window Manager");
    auto& menu = menu_button->get_menu();
    menu.add_action("Minimize All",
      profile.make_event_slot<>(QString("Action:Minimize All")));
    menu.add_action("Restore All",
      profile.make_event_slot<>(QString("Action:Restore All")));
    menu.add_action("Import Settings...",
      profile.make_event_slot<>(QString("Action:Import Settings")));
    menu.add_action("Export Settings...",
      profile.make_event_slot<>(QString("Action:Export Settings")));
    menu_button->setFixedWidth(scale_width(130));
    apply_widget_properties(menu_button, profile.get_properties());
    return menu_button;
  });
  return profile;
}

UiProfile Spire::make_menu_icon_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("tooltip", "Tooltip"));
  auto profile = UiProfile("MenuIconButton", properties, [] (auto& profile) {
    auto& tooltip = get<QString>("tooltip", profile.get_properties());
    auto menu_button = make_menu_icon_button(
      imageFromSvg(":/Icons/toolbar_icons/blotter.svg", scale(26, 26)),
      tooltip.get());
    apply_widget_properties(menu_button, profile.get_properties());
    auto& menu = menu_button->get_menu();
    menu.add_action("New...", profile.make_event_slot<>(QString("Action:New")));
    menu.add_separator();
    menu.add_action("Global",
      profile.make_event_slot<>(QString("Action:Global")));
    menu.add_action("Australia",
      profile.make_event_slot<>(QString("Action:Australia")));
    menu.add_action("North America",
      profile.make_event_slot<>(QString("Action:North America")));
    return menu_button;
  });
  return profile;
}

UiProfile Spire::make_money_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_decimal_box_properties<Money>(
    properties, DecimalBoxProfileProperties(Money::ONE));
  auto profile =
    UiProfile("MoneyBox", properties, setup_decimal_box_profile<MoneyBox>);
  return profile;
}

UiProfile Spire::make_money_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_scalar_filter_panel_properties(properties, "Filter Money");
  auto profile = UiProfile("MoneyFilterPanel", properties,
    setup_scalar_filter_panel_profile<MoneyBox>);
  return profile;
}

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
    auto model = populate_security_query_model();
    auto security_box = new SecurityBox(model);
    security_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    security_box->set_placeholder("SecurityBox");
    layout1->addWidget(security_box);
    auto region_box = new RegionBox(populate_region_box_model());
    region_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    region_box->set_placeholder("RegionBox");
    layout1->addWidget(region_box);
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

UiProfile Spire::make_number_label_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  auto number_type_property = define_enum<QString>(
    {{"Integer", "int"}, {"Unsigned Integer", "uint"},
    {"Integer64", "int64"}, {"Unsigned Integer64", "uint64"},
    {"double", "double"}, {"Quantity", "Quantity"}, {"Money", "Money"}});
  populate_enum_properties(properties, "number_type", number_type_property);
  properties.push_back(make_standard_property<Decimal>("number", 1000000));
  properties.push_back(make_standard_property("omit_group_separator", false));
  auto profile = UiProfile("NumberLabel", properties, [] (auto& profile) {
    auto convert =
      [&] (double value, const QString& type, bool is_omit_separator) {
        auto locale = QLocale();
        if(is_omit_separator) {
          locale.setNumberOptions(QLocale::OmitGroupSeparator);
        }
        if(type == "int") {
          return to_text(static_cast<int>(value), locale);
        } else if(type == "uint") {
          return to_text(static_cast<unsigned int>(value), locale);
        } else if(type == "int64") {
          return to_text(static_cast<std::int64_t>(value), locale);
        } else if(type == "uint64") {
          return to_text(static_cast<std::uint64_t>(value), locale);
        } else if(type == "Quantity") {
          return to_text(Quantity(value), locale);
        } else if(type == "Money") {
          return to_text(Money(Quantity(value)), locale);
        }
        return to_text(value, locale);
      };
    auto& type = get<QString>("number_type", profile.get_properties());
    auto& number = get<Decimal>("number", profile.get_properties());
    auto& separator =
      get<bool>("omit_group_separator", profile.get_properties());
    auto label = std::make_shared<LocalTextModel>();
    type.connect_changed_signal([=, &number, &separator] (const auto& type) {
      label->set(
        convert(number.get().convert_to<double>(), type, separator.get()));
    });
    number.connect_changed_signal([=, &type, &separator] (const auto& value) {
      label->set(
        convert(value.convert_to<double>(), type.get(), separator.get()));
    });
    separator.connect_changed_signal([=, &type, &number] (auto value) {
      label->set(convert(number.get().convert_to<double>(), type.get(), value));
    });
    return make_label(label);
  });
  return profile;
}

UiProfile Spire::make_order_field_info_tip_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("name", QString("TSXPegType")));
  properties.push_back(make_standard_property("description", QString(
    "Peg to the protected NBBO. Available on undisplayed orders only.")));
  properties.push_back(make_standard_property("value1", QString(
    "C,Contra Midpoint Only Plus. Order is priced at the Protected NBBO "
    "midpoint. Can trade only against other Contra Midpoint Only Plus "
    "orders.")));
  properties.push_back(make_standard_property("value2", QString(
    "D,Contra Midpoint Only Plus, Dark Sweep. Order is priced at the protected "
    "NBBO midpoint. Can trade against all dark orders on entry, and only "
    "against other Contra Midpoint Only Plus orders once resting.")));
  properties.push_back(make_standard_property(
    "value3", QString("M,Subject to the order's optional limit price.")));
  properties.push_back(make_standard_property(
    "value4", QString("Q,Additional value.")));
  properties.push_back(make_standard_property(
    "prereq1", QString("Prerequisite1,A,B,C,D,E")));
  properties.push_back(make_standard_property(
    "prereq2", QString("Prerequisite2,A,B,C,D,E")));
  properties.push_back(make_standard_property(
    "prereq3", QString("Prerequisite3,A,B,C,D,E")));
  properties.push_back(make_standard_property(
    "prereq4", QString("Prerequisite4,A,B,C,D,E")));
  auto profile = UiProfile("OrderFieldInfoTip", properties, [] (auto& profile) {
    auto label = make_label("Hover me!");
    apply_widget_properties(label, profile.get_properties());
    auto model = OrderFieldInfoTip::Model();
    model.m_tag.m_name =
      get<QString>("name", profile.get_properties()).get().toStdString();
    model.m_tag.m_description = get<QString>("description",
      profile.get_properties()).get().toStdString();
    auto parse_value = [] (const auto& text) {
      auto value = OrderFieldInfoTip::Model::Argument();
      auto list = text.split(",");
      if(!list.isEmpty()) {
        value.m_value = list.front().toStdString();
        if(list.size() > 1) {
          value.m_description = list[1].toStdString();
        }
      }
      return value;
    };
    auto parse_tag = [] (const auto& text) {
      auto tag = OrderFieldInfoTip::Model::Tag();
      auto list = text.split(",");
      for(auto& item : list) {
        if(tag.m_name.empty()) {
          tag.m_name = item.toStdString();
          continue;
        }
        tag.m_arguments.push_back({item.toStdString(), ""});
      }
      return tag;
    };
    for(auto i = 1; i < 5; ++i) {
      auto& value =
        get<QString>(QString("value%1").arg(i), profile.get_properties());
      if(auto text = value.get(); !text.isEmpty()) {
        model.m_tag.m_arguments.push_back(parse_value(text));
      }
      auto& prereq =
        get<QString>(QString("prereq%1").arg(i), profile.get_properties());
      if(auto text = prereq.get(); !text.isEmpty()) {
        model.m_prerequisites.push_back(parse_tag(text));
      }
    }
    auto tip = new OrderFieldInfoTip(std::move(model), label);
    return label;
  });
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
  populate_enum_properties(properties, "current", current_property);
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("OrderTypeBox", properties,
    std::bind_front(setup_enum_box_profile<OrderTypeBox, make_order_type_box>));
  return profile;
}

UiProfile Spire::make_order_type_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property<bool>("Limit"));
  properties.push_back(make_standard_property<bool>("Market"));
  properties.push_back(make_standard_property<bool>("Pegged"));
  properties.push_back(make_standard_property<bool>("Stop"));
  auto profile = UiProfile("OrderTypeFilterPanel", properties, std::bind_front(
    setup_closed_filter_panel_profile<
      OrderType, make_order_type_filter_panel>));
  return profile;
}

UiProfile Spire::make_overlay_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("close-on-focus-out", false));
  properties.push_back(make_standard_property("draggable", true));
  auto positioning_property = define_enum<OverlayPanel::Positioning>(
    {{"NONE", OverlayPanel::Positioning::NONE},
     {"PARENT", OverlayPanel::Positioning::PARENT}});
  properties.push_back(
    make_standard_enum_property("positioning", positioning_property));
  auto profile = UiProfile("OverlayPanel", properties, [=] (auto& profile) {
    auto& close_on_focus_out =
      get<bool>("close-on-focus-out", profile.get_properties());
    auto& draggable = get<bool>("draggable", profile.get_properties());
    auto& positioning =
      get<OverlayPanel::Positioning>("positioning", profile.get_properties());
    auto button = make_label_button("Click me");
    apply_widget_properties(button, profile.get_properties());
    auto panel = QPointer<OverlayPanel>();
    button->connect_click_signal(
      [=, &profile, &close_on_focus_out, &draggable, &positioning]
          () mutable {
        if(panel && !close_on_focus_out.get()) {
          return;
        }
        auto body = create_panel_body("Show child panel");
        panel = new OverlayPanel(*body, *button);
        auto child_button = body->findChild<Button*>();
        child_button->connect_click_signal(
          [=, &close_on_focus_out, &draggable, &positioning] {
            create_child_panel(close_on_focus_out.get(), draggable.get(),
              positioning.get(), child_button);
          });
        panel->setAttribute(Qt::WA_DeleteOnClose);
        panel->set_closed_on_focus_out(close_on_focus_out.get());
        panel->set_is_draggable(draggable.get());
        panel->set_positioning(positioning.get());
        panel->show();
      });
    return button;
  });
  return profile;
}

UiProfile Spire::make_percent_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_decimal_box_with_decimal_properties(
    properties, DecimalBoxProfileProperties(Decimal(1)));
  auto profile = UiProfile("PercentBox",
    properties, setup_decimal_box_with_decimal_profile<PercentBox>);
  return profile;
}

UiProfile Spire::make_popup_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  auto size_policy_property = define_enum<int>(
    {{"Blue", 0}, {"Green", 1}, {"Yellow", 2}});
  properties.push_back(make_standard_enum_property("horizontal_size_policy",
    size_policy_property));
  properties.push_back(make_standard_enum_property("vertical_size_policy",
    size_policy_property));
  auto profile = UiProfile("PopupBox", properties, [] (auto& profile) {
    auto popup_boxes = std::vector<PopupBox*>();
    auto widget = new QWidget();
    auto grid_layout = new QGridLayout(widget);
    grid_layout->setSpacing(0);
    for(auto i = 0; i < 5; ++i) {
      for(auto j = 0; j < 3; ++j) {
        auto widget = [&] () {
          if(i == 1 && j == 1) {
            auto region_box = new RegionBox(populate_region_box_model());
            region_box->set_placeholder("RegionBox");
            return new PopupBox(*region_box);
          } else if(i == 3 && j == 1) {
            auto tag_box = new TagBox(populate_tag_box_model(),
              std::make_shared<LocalTextModel>());
            tag_box->set_placeholder("TagBox");
            tag_box->connect_submit_signal([=] (const auto& value) {
              if(!value.isEmpty()) {
                tag_box->get_tags()->push(value);
              }
            });
            return new PopupBox(*tag_box);
          } else if(i == 4 && j == 2) {
            auto tag_combo_box =
              new TagComboBox(populate_tag_combo_box_model());
            tag_combo_box->set_placeholder("TagComboBox");
            return new PopupBox(*tag_combo_box);
          }
          auto text_box = new TextBox(QString("%1").arg(i));
          return new PopupBox(*text_box);
        }();
        popup_boxes.push_back(widget);
        grid_layout->addWidget(widget, i, j);
        if(j != 0) {
          grid_layout->setColumnStretch(j, 1);
        }
      }
    }
    grid_layout->setColumnMinimumWidth(0, scale_width(30));
    grid_layout->setColumnMinimumWidth(1, scale_width(120));
    grid_layout->setColumnMinimumWidth(2, scale_width(100));
    auto& horizontal_size_policy = get<int>("horizontal_size_policy",
      profile.get_properties());
    horizontal_size_policy.connect_changed_signal([=] (auto value) {
      for(auto box : popup_boxes) {
        auto policy = box->sizePolicy();
        if(value == 0) {
          policy.setHorizontalPolicy(QSizePolicy::Expanding);
        } else if(value == 1) {
          policy.setHorizontalPolicy(QSizePolicy::Preferred);
        } else {
          policy.setHorizontalPolicy(QSizePolicy::Fixed);
        }
        box->setSizePolicy(policy);
      }
    });
    auto& vertical_size_policy = get<int>("vertical_size_policy",
      profile.get_properties());
    vertical_size_policy.connect_changed_signal([=] (auto value) {
      for(auto box : popup_boxes) {
        auto policy = box->sizePolicy();
        if(value == 0) {
          policy.setVerticalPolicy(QSizePolicy::Expanding);
        } else if(value == 1) {
          policy.setVerticalPolicy(QSizePolicy::Preferred);
        } else {
          policy.setVerticalPolicy(QSizePolicy::Fixed);
        }
        box->setSizePolicy(policy);
      }
    });
    return widget;
  });
  return profile;
}

UiProfile Spire::make_progress_bar_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  get<int>("width", properties).set(scale_width(224));
  properties.push_back(make_standard_property("current", 0));
  auto profile = UiProfile("ProgressBar", properties, [] (auto& profile) {
    auto& current = get<int>("current", profile.get_properties());
    auto current_model = std::make_shared<LocalProgressModel>(current.get());
    auto progress_bar = new ProgressBar(current_model);
    apply_widget_properties(progress_bar, profile.get_properties());
    current.connect_changed_signal([=] (const auto& current) {
      current_model->set(std::clamp(current, 0, 100));
    });
    progress_bar->get_current()->connect_update_signal(
      profile.make_event_slot<int>("Current"));
    return progress_bar;
  });
  return profile;
}

UiProfile Spire::make_quantity_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto box_properties = DecimalBoxProfileProperties(Quantity(1));
  box_properties.m_minimum = Quantity(0);
  populate_decimal_box_properties<Quantity>(properties, box_properties);
  auto profile = UiProfile(
    "QuantityBox", properties, setup_decimal_box_profile<QuantityBox>);
  return profile;
}

UiProfile Spire::make_quantity_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_scalar_filter_panel_properties(properties, "Filter Quantity");
  auto profile = UiProfile("QuantityFilterPanel", properties,
    setup_scalar_filter_panel_profile<QuantityBox>);
  return profile;
}

UiProfile Spire::make_radio_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  populate_check_box_properties(properties);
  return UiProfile("RadioButton", properties, [=] (auto& profile) {
    return setup_checkable_profile(profile, make_radio_button());
  });
}

UiProfile Spire::make_region_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current",
    "TSX,USA,CAN"));
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("RegionBox", properties, [] (auto& profile) {
    auto query_model = populate_region_box_model();
    auto to_region = [=] (const auto& text) {
      auto region = Region();
      for(auto& value : text.split(",")) {
        if(auto parse_value = query_model->parse(value)) {
          region += *parse_value;
        }
      }
      return region;
    };
    auto to_string = [] (const auto& region) {
      auto text = QString();
      for(auto& country : region.GetCountries()) {
        text = text % to_text(country) % ",";
      }
      for(auto& market : region.GetMarkets()) {
        text = text % to_text(MarketToken(market)) % ",";
      }
      for(auto& security : region.GetSecurities()) {
        text = text % to_text(security) % ",";
      }
      text.remove(text.length() - 1, 1);
      return text;
    };
    auto& current = get<QString>("current", profile.get_properties());
    auto current_model = std::make_shared<LocalValueModel<Region>>(
      to_region(current.get()));
    current.connect_changed_signal([=] (const auto& value) {
      auto region = to_region(value);
      if(current_model->get() != region) {
        current_model->set(region);
      }
    });
    auto box = new RegionBox(query_model, current_model);
    box->setMinimumWidth(scale_width(112));
    apply_widget_properties(box, profile.get_properties());
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& placeholder) {
      box->set_placeholder(placeholder);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&RegionBox::set_read_only, box));
    auto print_region = [] (const Region& region) {
      auto result = QString();
      result += "Region{Countries{";
      for(auto& country : region.GetCountries()) {
        result += GetDefaultCountryDatabase().FromCode(country).
          m_threeLetterCode.GetData();
        result += " ";
      }
      result += "} Markets{";
      for(auto& market : region.GetMarkets()) {
        result += to_text(MarketToken(market));
        result += " ";
      }
      result += "} Securities{";
      for(auto& security : region.GetSecurities()) {
        result += to_text(security);
        result += " ";
      }
      result += "}}";
      return result;
    };
    auto current_slot = profile.make_event_slot<QString>("Current");
    box->get_current()->connect_update_signal(
      [=, &current] (const Region& region) {
        current_slot(print_region(region));
        if(to_region(current.get()) != region) {
          current.set(to_string(region));
        }
      });
    auto submit_slot = profile.make_event_slot<QString>("Submit");
    box->connect_submit_signal([=] (const Region& region) {
      submit_slot(print_region(region));
    });
    return box;
  });
  return profile;
}

UiProfile Spire::make_region_drop_down_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_region = define_enum<Region>(
    {{"NSEX", GetDefaultMarketDatabase().FromCode(DefaultMarkets::NSEX())},
     {"ISE", GetDefaultMarketDatabase().FromCode(DefaultMarkets::ISE())},
     {"TSX", GetDefaultMarketDatabase().FromCode(DefaultMarkets::TSX())},
     {"USA", Region(DefaultCountries::US())},
     {"CAN", Region(DefaultCountries::CA())}});
  properties.push_back(make_standard_enum_property("current", current_region));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("RegionDropDownBox", properties, [] (auto& profile) {
    auto markets = std::vector<MarketCode>{DefaultMarkets::NSEX(),
      DefaultMarkets::ISE(), DefaultMarkets::TSX()};
    auto countries = std::vector<CountryCode>{DefaultCountries::US(),
      DefaultCountries::CA()};
    auto regions = std::make_shared<ArrayListModel<Region>>();
    for(auto& market_code : markets) {
      auto market = GetDefaultMarketDatabase().FromCode(market_code);
      auto region = Region(market);
      region.SetName(market.m_description);
      regions->push(region);
    }
    for(auto& country : countries) {
      auto region = Region(country);
      region.SetName(GetDefaultCountryDatabase().FromCode(country).m_name);
      regions->push(region);
    }
    auto box = make_region_drop_down_box(std::move(regions));
    box->setFixedWidth(scale_width(150));
    apply_widget_properties(box, profile.get_properties());
    auto& current = get<Region>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      box->get_current()->set(value);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&RegionDropDownBox::set_read_only, box));
    box->get_current()->connect_update_signal(
      profile.make_event_slot<Region>("Current"));
    box->connect_submit_signal(profile.make_event_slot<Region>("Submit"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_region_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto type_property = define_enum<int>(
    {{"SECURITY", 0}, {"MARKET", 1}, {"COUNTRY", 2}});
  properties.push_back(make_standard_enum_property("type", type_property));
  auto profile = UiProfile("RegionListItem", properties, [] (auto& profile) {
    auto& type = get<int>("type", profile.get_properties());
    auto region = [&] {
      if(type.get() == 0) {
        auto security = ParseSecurity("MSFT.NSDQ");
        auto region = Region(security);
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

UiProfile Spire::make_responsive_label_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_property("label1", QString("Hello world")));
  properties.push_back(make_standard_property("label2", QString("Hello w.")));
  properties.push_back(make_standard_property("label3", QString("Hello")));
  properties.push_back(make_standard_property("label4", QString("HW")));
  properties.push_back(make_standard_property("font-size", 12));
  auto profile = UiProfile("ResponsiveLabel", properties, [] (auto& profile) {
    auto labels = std::make_shared<ArrayListModel<QString>>();
    auto& label1 = get<QString>("label1", profile.get_properties());
    labels->push(label1.get());
    auto& label2 = get<QString>("label2", profile.get_properties());
    labels->push(label2.get());
    auto& label3 = get<QString>("label3", profile.get_properties());
    labels->push(label3.get());
    auto& label4 = get<QString>("label4", profile.get_properties());
    labels->push(label4.get());
    auto label = new ResponsiveLabel(labels);
    apply_widget_properties(label, profile.get_properties());
    auto label_map =
      std::make_shared<std::unordered_map<int, int>>();
    auto connect_label_changed_signal =
      [=] (auto& property, auto id, auto label_list) {
        label_map->insert_or_assign(id, id);
        property.connect_changed_signal([=] (const auto& value) {
          if(value.isEmpty()) {
            if(auto index = label_map->find(id); index != label_map->end()) {
              labels->remove(label_map->at(index->first));
              for(auto& label_property : *label_map) {
                if(label_property.second > index->second) {
                  label_property.second -= 1;
                }
              }
            }
            label_map->erase(id);
            return;
          } else if(auto index = label_map->find(id);
              index == label_map->end()) {
            labels->push(value);
            label_map->insert_or_assign(id, labels->get_size() - 1);
            return;
          }
          labels->set(label_map->at(id), value);
        });
      };
    connect_label_changed_signal(label1, 0, labels);
    connect_label_changed_signal(label2, 1, labels);
    connect_label_changed_signal(label3, 2, labels);
    connect_label_changed_signal(label4, 3, labels);
    auto& font_size = get<int>("font-size", profile.get_properties());
    font_size.connect_changed_signal([=] (auto size) {
      update_style(*label, [&] (auto& style) {
        style.get(Any()).set(FontSize(scale_height(size)));
      });
    });
    return label;
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
  auto profile = UiProfile("ScrollBar", properties, [] (auto& profile) {
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
      profile.make_event_slot<int>("Position"));
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
  auto default_style = R"(
    any {
      horizontal_padding: 10px;
      vertical_padding: 10px;
      border: 1px 0xC8C8C8;
    }
  )";
  properties.push_back(make_style_property("style_sheet",
    std::move(default_style)));
  properties.push_back(make_standard_property("rows", 3));
  properties.push_back(make_standard_property("columns", 3));
  auto profile = UiProfile("ScrollBox", properties, [] (auto& profile) {
    auto label = new QLabel();
    auto& columns = get<int>("columns", profile.get_properties());
    auto& rows = get<int>("rows", profile.get_properties());
    label->setPixmap(QPixmap::fromImage(
      make_grid_image(scale(100, 100), columns.get(), rows.get())));
    auto scroll_box = new ScrollBox(label);
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
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*scroll_box, styles);
    });
    return scroll_box;
  });
  return profile;
}

UiProfile Spire::make_scrollable_list_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(
    make_standard_enum_property("direction", get_orientation_property()));
  auto overflow_property = define_enum<Overflow>(
    {{"NONE", Overflow::NONE}, {"WRAP", Overflow::WRAP}});
  properties.push_back(
    make_standard_enum_property("overflow", overflow_property));
  auto display_policy_property = define_enum<ScrollBox::DisplayPolicy>(
    {{"ON_OVERFLOW", ScrollBox::DisplayPolicy::ON_OVERFLOW},
     {"ON_ENGAGE", ScrollBox::DisplayPolicy::ON_ENGAGE}});
  properties.push_back(make_standard_enum_property(
    "horizontal_display_policy", display_policy_property));
  properties.push_back(make_standard_enum_property(
    "vertical_display_policy", display_policy_property));
  auto profile = UiProfile("ScrollableListBox", properties, [] (auto& profile) {
    auto list_model = std::make_shared<ArrayListModel<QString>>();
    for(auto i = 0; i < 15; ++i) {
      list_model->push(QString("Item%1").arg(i));
    }
    auto list_view = new ListView(list_model,
      [] (const std::shared_ptr<ListModel<QString>>& model, auto index) {
        return make_label(model->get(index));
      });
    auto scrollable_list_box = new ScrollableListBox(*list_view);
    apply_widget_properties(scrollable_list_box, profile.get_properties());
    auto& direction =
      get<Qt::Orientation>("direction", profile.get_properties());
    direction.connect_changed_signal([=] (auto value) {
      update_style(*list_view, [&] (auto& style) {
        style.get(Any()).set(value);
      });
    });
    auto& overflow = get<Overflow>("overflow", profile.get_properties());
    overflow.connect_changed_signal([=] (auto value) {
      update_style(*list_view, [&] (auto& style) {
        style.get(Any()).set(value);
      });
    });
    auto& horizontal_display_policy = get<ScrollBox::DisplayPolicy>(
      "horizontal_display_policy", profile.get_properties());
    horizontal_display_policy.connect_changed_signal([=] (auto value) {
      scrollable_list_box->get_scroll_box().set_horizontal(value);
    });
    auto& vertical_display_policy = get<ScrollBox::DisplayPolicy>(
      "vertical_display_policy", profile.get_properties());
    vertical_display_policy.connect_changed_signal([=] (auto value) {
      scrollable_list_box->get_scroll_box().set_vertical(value);
    });
    return scrollable_list_box;
  });
  return profile;
}

UiProfile Spire::make_search_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("placeholder"));
  auto profile = UiProfile("SearchBox", properties, [] (auto& profile) {
    auto search_box = new SearchBox();
    search_box->setFixedWidth(scale_width(180));
    apply_widget_properties(search_box, profile.get_properties());
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& text) {
      search_box->set_placeholder(text);
    });
    search_box->get_current()->connect_update_signal(
      profile.make_event_slot<QString>("Current"));
    search_box->connect_submit_signal(
      profile.make_event_slot<QString>("Submit"));
    return search_box;
  });
  return profile;
}

UiProfile Spire::make_security_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", "MX.TSX"));
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("SecurityBox", properties, [] (auto& profile) {
    auto model = populate_security_query_model();
    auto& current = get<QString>("current", profile.get_properties());
    auto current_model = std::make_shared<LocalValueModel<Security>>(
      model->parse(current.get())->m_security);
    auto box = new SecurityBox(model, current_model);
    box->setFixedWidth(scale_width(112));
    apply_widget_properties(box, profile.get_properties());
    auto current_connection = box->get_current()->connect_update_signal(
      profile.make_event_slot<Security>("Current"));
    current.connect_changed_signal([=] (const auto& current) {
      if(auto value = model->parse(current)) {
        box->get_current()->set(value->m_security);
      } else {
        auto current_blocker = shared_connection_block(current_connection);
        box->get_current()->set(Security());
      }
    });
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& placeholder) {
      box->set_placeholder(placeholder);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&SecurityBox::set_read_only, box));
    box->connect_submit_signal(profile.make_event_slot<Security>("Submit"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_security_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("SecurityListItem", properties, [] (auto& profile) {
    auto security = ParseSecurity("AB.NYSE");
    auto security_info =
      SecurityInfo(security, "Alliancebernstein Holding LP", "", 0);
    auto item = new SecurityListItem(security_info);
    apply_widget_properties(item, profile.get_properties());
    return item;
  });
  return profile;
}

UiProfile Spire::make_security_view_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property<int>("width", 266));
  properties.push_back(make_standard_property<int>("height", 361));
  auto profile = UiProfile("SecurityView", properties, [] (auto& profile) {
    auto model = populate_security_query_model();
    auto label = make_label("");
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto security_view = new SecurityView(model, *label);
    auto box = new Box(security_view);
    update_style(*box, [] (auto& style) {
      style.get(Hover() || Focus()).
        set(border(scale_width(1), QColor(0x4B23A0)));
    });
    security_view->get_current()->connect_update_signal(
      profile.make_event_slot<Security>("Current", [=] (const auto& security) {
        label->get_current()->set(to_text(security));
        return security;
      }));
    auto& width = get<int>("width", profile.get_properties());
    width.connect_changed_signal([=] (auto value) {
      if(value != 0) {
        if(unscale_width(security_view->width()) != value) {
          security_view->setFixedWidth(scale_width(value));
        }
      }
    });
    auto& height = get<int>("height", profile.get_properties());
    height.connect_changed_signal([=] (auto value) {
      if(value != 0) {
        if(unscale_height(security_view->height()) != value) {
          security_view->setFixedHeight(scale_height(value));
        }
      }
    });
    return box;
  });
  return profile;
}

UiProfile Spire::make_side_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_property = define_enum<Side>(
    {{"Buy", Side::BID}, {"Sell", Side::ASK}});
  populate_enum_properties(properties, "current", current_property);
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("SideBox", properties, std::bind_front(
    setup_enum_box_profile<SideBox, make_side_box>));
  return profile;
}

UiProfile Spire::make_side_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property<bool>("Buy"));
  properties.push_back(make_standard_property<bool>("Sell"));
  auto profile = UiProfile("SideFilterPanel", properties, std::bind_front(
    setup_closed_filter_panel_profile<Side, make_side_filter_panel>));
  return profile;
}

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
          imageFromSvg(":/Icons/color-thumb.svg", scale(14, 14));
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
      imageFromSvg(":/Icons/color-thumb.svg", scale(14, 14));
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

UiProfile Spire::make_split_view_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_enum_property(
    "orientation", Qt::Orientation::Horizontal, get_orientation_property()));
  properties.push_back(make_standard_property("primary_minimum_width", 222));
  properties.push_back(make_standard_property("primary_maximum_width", 774));
  properties.push_back(make_standard_property("primary_minimum_height", 100));
  properties.push_back(make_standard_property("primary_maximum_height", -1));
  properties.push_back(make_standard_property("secondary_minimum_width", 224));
  properties.push_back(make_standard_property("secondary_maximum_width", -1));
  properties.push_back(make_standard_property("secondary_minimum_height", 100));
  properties.push_back(make_standard_property("secondary_maximum_height", -1));
  auto profile = UiProfile("SplitView", properties, [] (auto& profile) {
    auto primary_box = new Box();
    update_style(*primary_box, [] (auto& style) {
      style.get(Any()).set(BackgroundColor(QColor(0xFFDFBF)));
    });
    auto secondary_box = new Box();
    update_style(*secondary_box, [] (auto& style) {
      style.get(Any()).set(BackgroundColor(QColor(0xFFFFBF)));
    });
    auto view = new SplitView(*primary_box, *secondary_box);
    auto& width = get<int>("width", profile.get_properties());
    width.set(1000);
    apply_widget_properties(view, profile.get_properties());
    auto& orientation =
      get<Qt::Orientation>("orientation", profile.get_properties());
    orientation.connect_changed_signal([=, &profile] (auto orientation) {
      update_style(*view, [&] (auto& style) {
        auto& width = get<int>("width", profile.get_properties());
        auto& height = get<int>("height", profile.get_properties());
        if(orientation == Qt::Orientation::Horizontal) {
          width.set(1000);
          height.set(100);
        } else {
          width.set(100);
          height.set(1000);
        }
        get<int>("primary_minimum_width", profile.get_properties()).set(222);
        get<int>("primary_maximum_width", profile.get_properties()).set(774);
        get<int>("primary_minimum_height", profile.get_properties()).set(100);
        get<int>("primary_maximum_height", profile.get_properties()).set(-1);
        get<int>("secondary_minimum_width", profile.get_properties()).set(224);
        get<int>("secondary_maximum_width", profile.get_properties()).set(-1);
        get<int>("secondary_minimum_height", profile.get_properties()).set(100);
        get<int>("secondary_maximum_height", profile.get_properties()).set(-1);
        style.get(Any()).set(orientation);
      });
    });
    auto& primary_minimum_width =
      get<int>("primary_minimum_width", profile.get_properties());
    primary_minimum_width.connect_changed_signal([=] (auto width) {
      primary_box->setMinimumWidth(scale_width(width));
    });
    auto& primary_maximum_width =
      get<int>("primary_maximum_width", profile.get_properties());
    primary_maximum_width.connect_changed_signal([=] (auto width) {
      if(width < 0) {
        primary_box->setMaximumWidth(QWIDGETSIZE_MAX);
      } else {
        primary_box->setMaximumWidth(scale_width(width));
      }
    });
    auto& primary_minimum_height =
      get<int>("primary_minimum_height", profile.get_properties());
    primary_minimum_height.connect_changed_signal([=] (auto height) {
      primary_box->setMinimumHeight(scale_height(height));
    });
    auto& primary_maximum_height =
      get<int>("primary_maximum_height", profile.get_properties());
    primary_maximum_height.connect_changed_signal([=] (auto height) {
      if(height < 0) {
        primary_box->setMaximumHeight(QWIDGETSIZE_MAX);
      } else {
        primary_box->setMaximumHeight(scale_height(height));
      }
    });
    auto& secondary_minimum_width =
      get<int>("secondary_minimum_width", profile.get_properties());
    secondary_minimum_width.connect_changed_signal([=] (auto width) {
      secondary_box->setMinimumWidth(scale_width(width));
    });
    auto& secondary_maximum_width =
      get<int>("secondary_maximum_width", profile.get_properties());
    secondary_maximum_width.connect_changed_signal([=] (auto width) {
      if(width < 0) {
        secondary_box->setMaximumWidth(QWIDGETSIZE_MAX);
      } else {
        secondary_box->setMaximumWidth(scale_width(width));
      }
    });
    auto& secondary_minimum_height =
      get<int>("secondary_minimum_height", profile.get_properties());
    secondary_minimum_height.connect_changed_signal([=] (auto height) {
      secondary_box->setMinimumHeight(scale_height(height));
    });
    auto& secondary_maximum_height =
      get<int>("secondary_maximum_height", profile.get_properties());
    secondary_maximum_height.connect_changed_signal([=] (auto height) {
      if(height < 0) {
        secondary_box->setMaximumHeight(QWIDGETSIZE_MAX);
      } else {
        secondary_box->setMaximumHeight(scale_height(height));
      }
    });
    return view;
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

UiProfile Spire::make_table_header_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("TableHeader", properties, [] (auto& profile) {
    auto items = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    auto item = TableHeaderItem::Model();
    item.m_name = "Security";
    items->push(item);
    item = TableHeaderItem::Model();
    item.m_name = "Quantity";
    item.m_order = TableHeaderItem::Order::ASCENDING;
    items->push(item);
    item = TableHeaderItem::Model();
    item.m_name = "Side";
    items->push(item);
    item = TableHeaderItem::Model();
    item.m_name = "Date";
    items->push(item);
    auto header = new TableHeader(items);
    apply_widget_properties(header, profile.get_properties());
    header->connect_sort_signal(
      profile.make_event_slot<int, TableHeaderItem::Order>(
        "Sort", to_string_converter(get_order_property())));
    header->connect_filter_signal(profile.make_event_slot<int>("Filter"));
    return header;
  });
  return profile;
}

UiProfile Spire::make_table_header_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto default_style = R"(
    any {
      padding_bottom: 0;
    }
    any > :label {
      text_align: left;
    }
  )";
  properties.push_back(make_style_property("style_sheet",
    std::move(default_style)));
  properties.push_back(
    make_standard_enum_property("order", get_order_property()));
  properties.push_back(
    make_standard_enum_property("filter", get_filter_property()));
  properties.push_back(make_standard_property<bool>("is_resizeable", true));
  auto profile = UiProfile("TableHeaderItem", properties, [=] (auto& profile) {
    auto item_model = TableHeaderItem::Model();
    item_model.m_name = "Security";
    item_model.m_order = TableHeaderItem::Order::ASCENDING;
    auto model =
      std::make_shared<LocalValueModel<TableHeaderItem::Model>>(item_model);
    auto item = new TableHeaderItem(model);
    apply_widget_properties(item, profile.get_properties());
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*item, styles);
    });
    link(make_field_value_model(model, &TableHeaderItem::Model::m_order),
      get<TableHeaderItem::Order>("order", profile.get_properties()));
    link(make_field_value_model(model, &TableHeaderItem::Model::m_filter),
      get<TableFilter::Filter>("filter", profile.get_properties()));
    link(&TableHeaderItem::is_resizeable, &TableHeaderItem::set_is_resizeable,
      *item, get<bool>("is_resizeable", profile.get_properties()));
    item->connect_sort_signal(profile.make_event_slot<TableHeaderItem::Order>(
      "Sort", to_string_converter(get_order_property())));
    item->connect_filter_signal(profile.make_event_slot("Filter"));
    item->connect_start_resize_signal(profile.make_event_slot("StartResize"));
    item->connect_end_resize_signal(profile.make_event_slot("EndResize"));
    return item;
  });
  return profile;
}

UiProfile Spire::make_table_view_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto default_style = R"(
    any > TableBody {
      padding_top: 0px;
      padding_right: 0px;
      padding_bottom: 0px;
      padding_left: 0px;
      horizontal_spacing: 1px;
      vertical_spacing: 1px;
      horizontal_grid_color: 0xE0E0E0;
      vertical_grid_color: 0xE0E0E0;
    }
    any > TableBody > :row > :hover_item {
      border_color: 0xA0A0A0;
    }
  )";
  properties.push_back(make_style_property("style_sheet",
    std::move(default_style)));
  properties.push_back(make_standard_property("row_count", 50));
  properties.push_back(make_standard_property("current_row", 0));
  properties.push_back(make_standard_property("update", 0));
  properties.push_back(std::make_shared<
    StandardUiProperty<TableModel::AddOperation>>(
      "insert", TableModel::AddOperation(-1), [] (auto parent, auto& property) {
        auto form = new QWidget();
        auto layout = make_hbox_layout(form);
        auto row = new QSpinBox();
        auto button = new QPushButton("Insert");
        layout->addWidget(row);
        layout->addWidget(button);
        QObject::connect(button, &QPushButton::pressed, [=, &property] {
          property.set(TableModel::AddOperation(row->value()));
        });
        return form;
      }));
  properties.push_back(std::make_shared<
    StandardUiProperty<TableModel::RemoveOperation>>(
      "remove", TableModel::RemoveOperation(-1), [] (
          auto parent, auto& property) {
        auto form = new QWidget();
        auto layout = make_hbox_layout(form);
        auto row = new QSpinBox();
        auto button = new QPushButton("Remove");
        layout->addWidget(row);
        layout->addWidget(button);
        QObject::connect(button, &QPushButton::pressed, [=, &property] {
          property.set(TableModel::RemoveOperation(row->value()));
        });
        return form;
      }));
  properties.push_back(std::make_shared<StandardUiProperty<int>>("hide-column",
    -2, [] (auto parent, auto& property) {
      auto form = new QWidget();
      auto layout = make_hbox_layout(form);
      auto column = new QSpinBox();
      column->setMinimum(-1);
      column->setMaximum(2);
      auto button = new QPushButton("Apply");
      layout->addWidget(column);
      layout->addWidget(button);
      QObject::connect(button, &QPushButton::pressed, [=, &property] {
        property.set(column->value());
      });
      return form;
    }));
  auto profile = UiProfile("TableView", properties, [] (auto& profile) {
    auto model = std::make_shared<ArrayTableModel>();
    auto& row_count = get<int>("row_count", profile.get_properties());
    for(auto row = 0; row != row_count.get(); ++row) {
      auto values = std::vector<std::any>();
      for(auto column = 0; column != 4; ++column) {
        values.push_back(row * 4 + column);
      }
      model->push(values);
    }
    auto header = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    auto item = TableHeaderItem::Model();
    item.m_name = "Security";
    item.m_short_name = "Sec";
    header->push(item);
    item = TableHeaderItem::Model();
    item.m_name = "Quantity";
    item.m_short_name = "Qty";
    item.m_order = TableHeaderItem::Order::ASCENDING;
    header->push(item);
    item = TableHeaderItem::Model();
    item.m_name = "Side";
    item.m_short_name = "Sd";
    header->push(item);
    item = TableHeaderItem::Model();
    item.m_name = "Date";
    item.m_short_name = "Date";
    header->push(item);
    auto view = TableViewBuilder(model).
      set_header(header).
      set_standard_filter().
      set_item_builder(
        [] (const std::shared_ptr<TableModel>& table, int row, int column) {
          return make_label(make_to_text_model(
            make_table_value_model<int>(table, row, column)));
        }).
      make();
    apply_widget_properties(view, profile.get_properties());
    view->get_current()->connect_update_signal(
      profile.make_event_slot<TableView::CurrentModel::Type>(
        "Current", [] (auto index) {
          return to_string(index);
        }));
    view->connect_sort_signal(
      profile.make_event_slot<int, TableHeaderItem::Order>(
        "Sort", to_string_converter(get_order_property())));
    auto& height = get<int>("height", profile.get_properties());
    height.set(300);
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*view, styles);
    });
    auto& current_row = get<int>("current_row", profile.get_properties());
    current_row.connect_changed_signal([=] (auto value) {
      view->get_current()->set(TableView::Index(value, 0));
    });
    auto& update_operation = get<int>("update", profile.get_properties());
    update_operation.connect_changed_signal([&, view] (auto value) {
      if(auto current = view->get_current()->get()) {
        view->get_table()->set(current->m_row, current->m_column, value);
      }
    });
    auto& insert_operation =
      get<TableModel::AddOperation>("insert", profile.get_properties());
    insert_operation.connect_changed_signal([=] (const auto& operation) {
      if(operation.m_index >= 0 &&
          operation.m_index <= model->get_row_size()) {
        auto value = model->get_row_size() * 4;
        model->insert(
          std::vector<std::any>{value, value + 1, value + 2, value + 3},
          operation.m_index);
      }
    });
    auto& remove_operation =
      get<TableModel::RemoveOperation>("remove", profile.get_properties());
    remove_operation.connect_changed_signal([=] (const auto& operation) {
      if(operation.m_index >= 0 &&
          operation.m_index < model->get_row_size()) {
        model->remove(operation.m_index);
      }
    });
    auto& hide_column = get<int>("hide-column", profile.get_properties());
    hide_column.connect_changed_signal([=] (int column) {
      auto& widths = view->get_header().get_widths();
      if(column >= 0 && column < widths->get_size()) {
        view->get_header().get_item(column)->setVisible(false);
      } else if(column == -1) {
        for(auto i = 0; i < widths->get_size(); ++i) {
          if(!view->get_header().get_item(i)->isVisible()) {
            view->get_header().get_item(i)->setVisible(true);
          }
        }
      }
    });
    return view;
  });
  return profile;
}

UiProfile Spire::make_tag_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("label", "CAN"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("Tag", properties, [] (auto& profile) {
    auto& label = get<QString>("label", profile.get_properties());
    auto tag = new Tag(label.get());
    label.connect_changed_signal([=] (const auto& value) {
      if(tag->get_label()->get() != value) {
        tag->get_label()->set(value);
      }
    });
    tag->get_label()->connect_update_signal([&] (const auto& value) {
      if(label.get() != value) {
        label.set(value);
      }
    });
    apply_widget_properties(tag, profile.get_properties());
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      tag->set_read_only(is_read_only);
    });
    tag->connect_delete_signal(profile.make_event_slot("DeleteSignal"));
    tag->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    tag->setMinimumSize(0, 0);
    return tag;
  });
  return profile;
}

UiProfile Spire::make_tag_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property("read_only", false));
  properties.push_back(make_standard_property<QString>("add_tag"));
  auto profile = UiProfile("TagBox", properties, [] (auto& profile) {
    auto tag_box = new TagBox(populate_tag_box_model(),
      std::make_shared<LocalTextModel>());
    apply_widget_properties(tag_box, profile.get_properties());
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& text) {
      tag_box->set_placeholder(text);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      tag_box->set_read_only(is_read_only);
    });
    auto& add_tag = get<QString>("add_tag", profile.get_properties());
    add_tag.connect_changed_signal([=] (const auto& value) {
      if(!value.isEmpty()) {
        tag_box->get_tags()->push(value);
      }
    });
    tag_box->connect_submit_signal(profile.make_event_slot<QString>("Submit"));
    return tag_box;
  });
  return profile;
}

UiProfile Spire::make_tag_combo_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("TagComboBox", properties, [] (auto& profile) {
    auto box = new TagComboBox(populate_tag_combo_box_model());
    box->setMinimumWidth(scale_width(112));
    apply_widget_properties(box, profile.get_properties());
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& placeholder) {
      box->set_placeholder(placeholder);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&TagComboBox<QString>::set_read_only, box));
    auto current_filter_slot =
      profile.make_event_slot<QString>(QString::fromUtf8("Current"));
    auto print_current = [=] {
      auto result = QString();
      for(auto i = 0; i < box->get_current()->get_size(); ++i) {
        result += box->get_current()->get(i) + " ";
      }
      current_filter_slot(result);
    };
    box->get_current()->connect_operation_signal([=] (const auto& operation) {
      visit(operation,
        [=] (const ListModel<QString>::AddOperation& operation) {
          print_current();
        },
        [=] (const ListModel<QString>::RemoveOperation& operation) {
          print_current();
        });
    });
    auto submit_filter_slot =
      profile.make_event_slot<QString>(QString::fromUtf8("Submit"));
    box->connect_submit_signal([=] (const auto& submission) {
      auto result = QString();
      for(auto i = 0; i < submission->get_size(); ++i) {
        result += submission->get(i) + " ";
      }
      submit_filter_slot(result);
    });
    return box;
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
  auto profile = UiProfile("TextAreaBox", properties, [] (auto& profile) {
    auto text_area_box = new TextAreaBox();
    apply_widget_properties(text_area_box, profile.get_properties());
    auto& current = get<QString>("current", profile.get_properties());
    current.connect_changed_signal([=] (const auto& value) {
      if(text_area_box->get_current()->get() != value) {
        text_area_box->get_current()->set(value);
      }
    });
    text_area_box->get_current()->connect_update_signal(
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
        update_style(*text_area_box, [&] (auto& style) {
          style.get(Any()).set(LineHeight(
            static_cast<double>(line_height) / 100));
        });
      });
    auto& horizontal_alignment = get<Qt::Alignment>("horizontal-align",
      profile.get_properties());
    horizontal_alignment.connect_changed_signal(
      [&, text_area_box] (auto alignment) {
        update_style(*text_area_box, [&] (auto& style) {
          style.get(Any()).
            set(TextAlign(
              Qt::Alignment(alignment) | Qt::AlignTop));
        });
      });
    text_area_box->connect_submit_signal(
      profile.make_event_slot<QString>("Submit"));
    text_area_box->get_current()->connect_update_signal(
      profile.make_event_slot<QString>("Current"));
    return text_area_box;
  });
  return profile;
}

UiProfile Spire::make_text_area_label_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property<int>("width", 100));
  properties.push_back(make_standard_property("current", QString("Label")));
  auto profile = UiProfile("TextAreaLabel", properties, [] (auto& profile) {
    auto text_area_label = make_text_area_label("");
    auto& width = get<int>("width", profile.get_properties());
    width.connect_changed_signal([=] (auto value) {
      if(value != 0) {
        if(unscale_width(text_area_label->width()) != value) {
          text_area_label->setFixedWidth(scale_width(value));
        }
      }
    });
    auto& current = get<QString>("current", profile.get_properties());
    current.connect_changed_signal([=] (const auto& value) {
      text_area_label->get_current()->set(value);
    });
    return text_area_label;
  });
  return profile;
}

UiProfile Spire::make_text_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<bool>("read_only"));
  properties.push_back(make_standard_property<QString>("current"));
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property<QString>("rejected", "deny"));
  auto default_style = R"(
    any {
      horizontal_padding: 8px;
      border_size: 1px;
      text_align: left;
    }
    :rejected {
      background_color: chain(timeout(0xFFF1F1, 250ms),
        linear(0xFFF1F1, revert, 300ms));
      border_color: chain(timeout(0xB71C1C, 550ms), revert);
    }
  )";
  properties.push_back(make_style_property("style_sheet",
    std::move(default_style)));
  auto profile = UiProfile("TextBox", properties, [] (auto& profile) {
    auto& rejected = get<QString>("rejected", profile.get_properties());
    auto model = make_validated_value_model<QString>(
      [&] (const QString& value) {
        if(value == rejected.get()) {
          return QValidator::State::Intermediate;
        }
        return QValidator::State::Acceptable;
      });
    auto text_box = new TextBox(model);
    text_box->setFixedWidth(scale_width(100));
    apply_widget_properties(text_box, profile.get_properties());
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      text_box->set_read_only(is_read_only);
    });
    link(text_box->get_current(),
      get<QString>("current", profile.get_properties()));
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& text) {
      text_box->set_placeholder(text);
    });
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*text_box, styles);
    });
    text_box->get_current()->connect_update_signal(
      profile.make_event_slot<QString>("Current"));
    text_box->connect_submit_signal(profile.make_event_slot<QString>("Submit"));
    text_box->connect_reject_signal(profile.make_event_slot<QString>("Reject"));
    return text_box;
  });
  return profile;
}

UiProfile Spire::make_time_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", ""));
  properties.push_back(make_standard_property<bool>("read_only"));
  auto profile = UiProfile("TimeBox", properties, [] (auto& profile) {
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
        if(time_box->get_current()->get() != *current_value) {
          time_box->get_current()->set(*current_value);
        }
      }
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      time_box->set_read_only(is_read_only);
    });
    time_box->get_current()->connect_update_signal(
      profile.make_event_slot<optional<time_duration>>("Current"));
    time_box->connect_submit_signal(
      profile.make_event_slot<optional<time_duration>>("Submit"));
    time_box->connect_reject_signal(
      profile.make_event_slot<optional<time_duration>>("Reject"));
    return time_box;
  });
  return profile;
}

UiProfile Spire::make_time_in_force_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_property = define_enum<TimeInForce>(
    {{"Day", TimeInForce(TimeInForce::Type::DAY)},
     {"GTC", TimeInForce(TimeInForce::Type::GTC)},
     {"OPG", TimeInForce(TimeInForce::Type::OPG)},
     {"IOC", TimeInForce(TimeInForce::Type::IOC)},
     {"FOK", TimeInForce(TimeInForce::Type::FOK)},
     {"GTX", TimeInForce(TimeInForce::Type::GTX)},
     {"GTD", TimeInForce(TimeInForce::Type::GTD)},
     {"MOC", TimeInForce(TimeInForce::Type::MOC)}});
  populate_enum_properties(properties, "current", current_property);
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("TimeInForceBox", properties, std::bind_front(
    setup_enum_box_profile<TimeInForceBox, make_time_in_force_box>));
  return profile;
}

UiProfile Spire::make_time_in_force_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property<bool>("DAY"));
  properties.push_back(make_standard_property<bool>("GTC"));
  properties.push_back(make_standard_property<bool>("OPG"));
  properties.push_back(make_standard_property<bool>("IOC"));
  properties.push_back(make_standard_property<bool>("FOK"));
  properties.push_back(make_standard_property<bool>("GTX"));
  properties.push_back(make_standard_property<bool>("GTD"));
  properties.push_back(make_standard_property<bool>("MOC"));
  auto profile = UiProfile("TimeInForceFilterPanel", properties,
    std::bind_front(setup_closed_filter_panel_profile<
      TimeInForce, make_time_in_force_filter_panel>));
  return profile;
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
  auto profile = UiProfile("TitleBar", properties, [] (auto& profile) {
    auto& title = get<QString>("title", profile.get_properties());
    auto& icon =
      get<std::tuple<QString, QString>>("icon", profile.get_properties());
    auto button = make_label_button("Click me");
    button->connect_click_signal([&title, &icon] {
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
      window->show();
    });
    return button;
  });
  return profile;
}

UiProfile Spire::make_tooltip_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(
    make_standard_property("tooltip-text", QString("Tooltip Text")));
  auto profile = UiProfile("Tooltip", properties, [] (auto& profile) {
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
