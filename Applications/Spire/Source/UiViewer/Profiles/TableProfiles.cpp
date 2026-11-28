#include "Spire/UiViewer/Profiles/TableProfiles.hpp"
#include <QPushButton>
#include <QSpinBox>
#include "Nexus/Definitions/StandardCountries.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ScalarValueModelDecorator.hpp"
#include "Spire/Spire/TableValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Spire/ValidatedValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/EditableTableView.hpp"
#include "Spire/Ui/GroupedTableView.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OrderTypeBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/StandardTableFilter.hpp"
#include "Spire/Ui/TableColumnReorderController.hpp"
#include "Spire/Ui/TableHeader.hpp"
#include "Spire/Ui/TableHeaderItem.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/UiViewer/Profiles/ProfileModels.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
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

  QString format_index(const TableView::CurrentModel::Type index) {
    if(index) {
      return QString("(%1, %2)").arg(index->m_row).arg(index->m_column);
    }
    return QString("None");
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

UiProfile Spire::make_grouped_table_view_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>{
    make_standard_property("enabled", true),
    make_standard_property("width", 630),
    make_standard_property("height", 350),
    make_standard_property("AR.TSX shares", 1200)};
  auto default_style = R"(
    any > TableBody > (:aggregate_row and !:selected) {
      background_color: 0xEFEFEF;
    }
  )";
  properties.push_back(make_style_property("style_sheet", default_style));
  return UiProfile("GroupedTableView", properties, [] (auto& profile) {
    auto positions = std::make_shared<ArrayTableModel>();
    auto& shares = get<int>("AR.TSX shares", profile.get_properties());
    positions->push({QString("conglg15"), QString("CAD"), QString("AR.TSX"),
      QString("Long"), shares.get(), Countries::CA});
    positions->push({QString("conglg15"), QString("USD"),
      QString("MSFT.NASDAQ"), QString("Long"), 600, Countries::US});
    positions->push({QString("dichch10"), QString("CAD"), QString("AGRA.CSE"),
      QString("Long"), 800, Countries::CA});
    positions->push({QString("conglg15"), QString("CAD"), QString("BHS.TSXV"),
      QString("Long"), 400, Countries::CA});
    positions->push({QString("dichch10"), QString("CAD"), QString("ATMO.CSE"),
      QString("Short"), 300, Countries::CA});
    auto view = GroupedTableViewBuilder(positions).
      add_column("Account", "Acct").add_column("Currency", "Ccy").
      add_column("Ticker").add_column("Side").add_column("Shares").
      add_column("Country").set_item_builder(
        [] (const std::shared_ptr<TableModel>& table, int row, int column) ->
            QWidget* {
          if(column != 5) {
            return TableView::default_item_builder(table, row, column);
          }
          auto code = to_text(table->get<CountryCode>(row, column));
          auto size = scale(16, 10);
          auto flag = new Icon(image_from_svg(
            QString(":/Icons/flag_icons/%1.svg").arg(code.toLower()), size));
          flag->setFixedSize(size);
          flag->setAccessibleName(code);
          set_style(*flag, StyleSheet());
          return flag;
        }).make();
    new TableColumnReorderController(*view);
    auto& widths = *view->get_header().get_widths();
    widths.set(0, scale_width(180));
    widths.set(1, scale_width(85));
    widths.set(2, scale_width(115));
    widths.set(3, scale_width(70));
    widths.set(4, scale_width(80));
    widths.set(5, scale_width(85));
    apply_widget_properties(view, profile.get_properties());
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      if(styles) {
        set_style(*view, *styles);
      }
    });
    shares.connect_changed_signal([=] (int value) {
      positions->set(0, 4, value);
    });
    return view;
  });
}

UiProfile Spire::make_table_header_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("TableHeader", properties, [] (auto& profile) {
    auto items = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    auto types = std::vector<std::type_index>();
    auto item = TableHeaderItem::Model();
    item.m_name = "Ticker";
    item.m_short_name = "Sec";
    item.m_filter = TableFilter::Filter::UNFILTERED;
    items->push(item);
    types.push_back(typeid(int));
    item = TableHeaderItem::Model();
    item.m_name = "Quantity";
    item.m_order = TableHeaderItem::Order::ASCENDING;
    item.m_filter = TableFilter::Filter::UNFILTERED;
    items->push(item);
    types.push_back(typeid(int));
    item = TableHeaderItem::Model();
    item.m_name = "Side";
    item.m_filter = TableFilter::Filter::UNFILTERED;
    items->push(item);
    types.push_back(typeid(int));
    item = TableHeaderItem::Model();
    item.m_name = "Date";
    item.m_filter = TableFilter::Filter::UNFILTERED;
    items->push(item);
    types.push_back(typeid(date));
    auto header = new TableHeader(items,
      std::make_shared<StandardTableFilter>(types));
    apply_widget_properties(header, profile.get_properties());
    header->connect_sort_signal(
      profile.make_event_slot<int, TableHeaderItem::Order>(
        "Sort", to_string_converter(get_order_property())));
      return header;
    });
  return profile;
}

UiProfile Spire::make_table_header_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto default_style = R"(
    any > :label {
      text_align: left;
    }
    any > :container {
      padding_left: 10px;
    }
  )";
  properties.push_back(make_style_property("style_sheet",
    std::move(default_style)));
  properties.push_back(make_standard_property<QString>("name", "Ticker"));
  properties.push_back(make_standard_property<QString>("short_name", "Sec"));
  properties.push_back(
    make_standard_enum_property("order", get_order_property()));
  properties.push_back(
    make_standard_enum_property("filter", get_filter_property()));
  properties.push_back(make_standard_property<bool>("is_resizeable", true));
  properties.push_back(make_standard_property<bool>("is_filter_open", false));
  auto profile = UiProfile("TableHeaderItem", properties, [=] (auto& profile) {
    auto& name = get<QString>("name", profile.get_properties());
    auto& short_name = get<QString>("short_name", profile.get_properties());
    auto item_model = TableHeaderItem::Model();
    item_model.m_name = name.get();
    item_model.m_short_name = short_name.get();
    auto model = make_local_value_model(item_model);
    auto item = new TableHeaderItem(model);
    apply_widget_properties(item, profile.get_properties());
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*item, styles);
    });
    name.connect_changed_signal([=] (const auto& value) {
      auto item_model = model->get();
      item_model.m_name = value;
      model->set(item_model);
    });
    short_name.connect_changed_signal([=] (const auto& value) {
      auto item_model = model->get();
      item_model.m_short_name = value;
      model->set(item_model);
    });
    link(make_field_value_model(model, &TableHeaderItem::Model::m_order),
      get<TableHeaderItem::Order>("order", profile.get_properties()));
    link(make_field_value_model(model, &TableHeaderItem::Model::m_filter),
      get<TableFilter::Filter>("filter", profile.get_properties()));
    link(&TableHeaderItem::is_resizeable, &TableHeaderItem::set_is_resizeable,
      *item, get<bool>("is_resizeable", profile.get_properties()));
    link(item->is_filter_open(),
      get<bool>("is_filter_open", profile.get_properties()));
    item->connect_sort_signal(profile.make_event_slot<TableHeaderItem::Order>(
      "Sort", to_string_converter(get_order_property())));
    item->is_filter_open()->connect_update_signal(
      profile.make_event_slot<bool>("FilterOpen"));
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
    item.m_name = "Ticker";
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
          return format_index(index);
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
      if(column >= 0) {
        view->hide_column(column);
      } else {
        for(auto i = 0; i < widths->get_size(); ++i) {
          view->show_column(i);
        }
      }
    });
    return view;
  });
  return profile;
}
