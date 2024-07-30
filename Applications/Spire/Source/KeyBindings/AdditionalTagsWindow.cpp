#include "Spire/KeyBindings/AdditionalTagsWindow.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/KeyBindings/AdditionalTagKeyBox.hpp"
#include "Spire/KeyBindings/AdditionalTagValueBox.hpp"
#include "Spire/KeyBindings/NoneAdditionalTagSchema.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/TableValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/EditableTableView.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextAreaBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct AppendableTableModel : TableModel {
    static const auto KEY_COLUMN = 0;
    static const auto VALUE_COLUMN = 1;
    static const auto COLUMN_COUNT = 2;
    static const auto NONE_KEY = -1;
    static const auto& NONE_VALUE() {
      static auto NONE = optional<Nexus::Tag::Type>();
      return NONE;
    }

    std::shared_ptr<ArrayTableModel> m_source;
    AdditionalTagDatabase m_additional_tags;
    std::shared_ptr<DestinationModel> m_destination;
    std::shared_ptr<RegionModel> m_region;
    std::vector<std::shared_ptr<ListModel<int>>> m_available_tags;
    scoped_connection m_connection;
    TableModelTransactionLog m_transaction;

    AppendableTableModel(std::shared_ptr<ArrayTableModel> source,
        AdditionalTagDatabase additional_tags,
        std::shared_ptr<DestinationModel> destination,
        std::shared_ptr<RegionModel> region)
        : m_source(std::move(source)),
          m_additional_tags(std::move(additional_tags)),
          m_destination(std::move(destination)),
          m_region(std::move(region)) {
      for(auto i = 0; i != get_row_size(); ++i) {
        add_available_tags(i);
      }
      m_connection = m_source->connect_operation_signal(
        std::bind_front(&AppendableTableModel::on_operation, this));
    }

    int get_row_size() const {
      return m_source->get_row_size() + 1;
    }

    int get_column_size() const {
      return COLUMN_COUNT;
    }

    AnyRef at(int row, int column) const {
      if(row < m_source->get_row_size()) {
        return m_source->at(row, column);
      } else if(row == m_source->get_row_size()) {
        if(column == KEY_COLUMN) {
          return NONE_KEY;
        } else if(column == VALUE_COLUMN) {
          static auto NONE = optional<Nexus::Tag::Type>();
          return NONE;
        }
      }
      throw std::out_of_range("The row or column is out of range.");
    }

    QValidator::State set(int row, int column, const std::any& value) {
      if(row < m_source->get_row_size()) {
        return m_source->set(row, column, value);
      } else if(row == m_source->get_row_size()) {
        if(column == VALUE_COLUMN) {
          if(std::any_cast<const optional<Nexus::Tag::Type>&>(value) ==
              NONE_VALUE()) {
            return QValidator::Acceptable;
          }
        }
        auto blocker = shared_connection_block(m_connection);
        add_available_tags(get_row_size());
        m_source->push({value, NONE_VALUE()});
        reduce_available_tags(row);
        m_transaction.transact([&] {
          m_transaction.push(UpdateOperation(row, column, NONE_KEY, value));
          m_transaction.push(AddOperation(row + 1));
        });
        return QValidator::Acceptable;
      }
      throw std::out_of_range("The row is out of range.");
    }

    QValidator::State remove(int row) {
      if(row < m_source->get_row_size()) {
        return m_source->remove(row);
      } else if(row == m_source->get_row_size()) {
        return QValidator::Invalid;
      }
      throw std::out_of_range("The row is out of range.");
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const {
      return m_transaction.connect_operation_signal(slot);
    }

    void add_available_tags(int row) {
      auto available_tags = std::make_shared<ArrayListModel<int>>();
      auto keys =
        find(m_additional_tags, m_destination->get(), m_region->get());
      for(auto& key : keys) {
        available_tags->push(key->get_key());
      }
      for(auto i = 0; i != get_row_size(); ++i) {
        if(i == row) {
          continue;
        }
        auto j = std::find(available_tags->begin(), available_tags->end(),
          get<int>(i, KEY_COLUMN));
        if(j != available_tags->end()) {
          available_tags->remove(j);
        }
      }
      m_available_tags.insert(
        m_available_tags.begin() + row, std::move(available_tags));
    }

    void reduce_available_tags(int row, int key) {
      if(key == -1) {
        return;
      }
      for(auto i = 0; i != get_row_size(); ++i) {
        if(i == row) {
          continue;
        }
        auto& available_tags = *m_available_tags[i];
        auto j = std::find(available_tags.begin(), available_tags.end(), key);
        if(j != available_tags.end()) {
          available_tags.remove(j);
        }
      }
    }

    void reduce_available_tags(int row) {
      reduce_available_tags(row, get<int>(row, KEY_COLUMN));
    }

    void augment_available_tags(int row, int key) {
      if(key == -1) {
        return;
      }
      for(auto i = 0; i != get_row_size(); ++i) {
        if(i == row) {
          continue;
        }
        auto& available_tags = *m_available_tags[i];
        auto j = std::find(available_tags.begin(), available_tags.end(), key);
        if(j == available_tags.end()) {
          available_tags.push(key);
        }
      }
    }

    void augment_available_tags(int row) {
      augment_available_tags(row, get<int>(row, KEY_COLUMN));
    }

    void on_operation(const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::AddOperation& operation) {
          add_available_tags(operation.m_index);
          reduce_available_tags(operation.m_index);
        },
        [&] (const TableModel::PreRemoveOperation& operation) {
          augment_available_tags(operation.m_index);
        },
        [&] (const TableModel::RemoveOperation& operation) {
          m_available_tags.erase(m_available_tags.begin() + operation.m_index);
        },
        [&] (const TableModel::MoveOperation& operation) {
          auto i = m_available_tags.begin() + operation.m_source;
          auto source = std::move(*i);
          m_available_tags.erase(i);
          m_available_tags.insert(m_available_tags.begin() +
            operation.m_destination, std::move(source));
        },
        [&] (const TableModel::UpdateOperation& operation) {
          if(operation.m_column != KEY_COLUMN) {
            return;
          }
          reduce_available_tags(operation.m_row);
          augment_available_tags(
            operation.m_row, std::any_cast<int>(operation.m_previous));
        });
      m_transaction.push(operation);
    }
  };

  struct KeyToSchemaModel : AdditionalTagSchemaModel {
    std::shared_ptr<AdditionalTagKeyModel> m_key;
    AdditionalTagDatabase m_additional_tags;
    std::shared_ptr<DestinationModel> m_destination;
    std::shared_ptr<RegionModel> m_region;
    scoped_connection m_connection;
    LocalAdditionalTagSchemaModel m_schema;

    KeyToSchemaModel(std::shared_ptr<AdditionalTagKeyModel> key,
        AdditionalTagDatabase additional_tags,
        std::shared_ptr<DestinationModel> destination,
        std::shared_ptr<RegionModel> region)
        : m_key(std::move(key)),
          m_additional_tags(std::move(additional_tags)),
          m_destination(std::move(destination)),
          m_region(std::move(region)) {
      m_connection = m_key->connect_update_signal(
        std::bind_front(&KeyToSchemaModel::on_update, this));
      on_update(m_key->get());
    }

    QValidator::State get_state() const override {
      return m_schema.get_state();
    }

    const Type& get() const override {
      return m_schema.get();
    }

    QValidator::State test(const Type& value) const override {
      return m_schema.test(value);
    }

    QValidator::State set(const Type& value) override {
      return m_schema.set(value);
    }

    connection
        connect_update_signal(const UpdateSignal::slot_type& slot) const {
      return m_schema.connect_update_signal(slot);
    }

    void on_update(int key) {
      if(key == -1) {
        m_schema.set(NoneAdditionalTagSchema::get_instance());
      } else {
        auto schema =
          find(m_additional_tags, m_destination->get(), m_region->get(), key);
        if(schema) {
          m_schema.set(schema);
        } else {
          m_schema.set(NoneAdditionalTagSchema::get_instance());
        }
      }
    }
  };

  auto make_tags_table(const std::vector<AdditionalTag>& tags,
      AdditionalTagDatabase additional_tags,
      std::shared_ptr<DestinationModel> destination,
      std::shared_ptr<RegionModel> region) {
    auto table = std::make_shared<ArrayTableModel>();
    for(auto& tag : tags) {
      auto row = std::vector<std::any>();
      row.push_back(tag.m_key);
      row.push_back(tag.m_value);
      table->push(std::move(row));
    }
    return std::make_shared<AppendableTableModel>(std::move(table),
      std::move(additional_tags), std::move(destination), std::move(region));
  }

  auto make_table_view(
      std::shared_ptr<TableModel> tags, TableViewItemBuilder view_builder) {
    static const auto DELETE_COLUMN = 0;
    static const auto NAME_COLUMN = 1;
    static const auto VALUE_COLUMN = 2;
    auto table_builder = EditableTableViewBuilder(tags);
    table_builder.set_item_builder(std::move(view_builder));
    table_builder.add_header_item(QObject::tr("Name"));
    table_builder.add_header_item(
      QObject::tr("Value"), TableHeaderItem::Order::UNORDERED);
    table_builder.set_comparator([=] (const AnyRef& left, int left_row,
          const AnyRef& right, int right_row, int column) {
        if(left_row == tags->get_row_size() - 1) {
          return false;
        }
        return compare(left, right);
      });
    auto table_view = table_builder.make();
    table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    table_view->get_header().get_widths()->set(DELETE_COLUMN, scale_width(26));
    table_view->get_header().get_widths()->set(NAME_COLUMN, scale_width(130));
    table_view->get_header().get_widths()->set(VALUE_COLUMN, scale_width(90));
    return table_view;
  }

  auto make_error_box(std::shared_ptr<TextModel> error_message) {
    auto body = new QWidget();
    auto outer_layout = make_hbox_layout(body);
    auto error_layout = make_vbox_layout();
    auto error_icon =
      new Icon(imageFromSvg(":Icons/key_bindings/error.svg", scale(14, 14)));
    update_style(*error_icon, [] (auto& style) {
      style.get(Any()).set(Fill(QColor(0xE63F44)));
    });
    error_layout->addWidget(error_icon);
    error_layout->addStretch(1);
    outer_layout->addLayout(error_layout);
    outer_layout->addSpacing(scale_width(4));
    auto error_message_box = new TextAreaBox(error_message);
    error_message_box->setSizePolicy(
      QSizePolicy::Expanding, QSizePolicy::Preferred);
    update_style(*error_message_box, [] (auto& style) {
      style.get(Any()).
        set(TextColor(QColor(0xE63F44))).
        set(TextAlign(Qt::AlignLeft));
    });
    auto box = new Box(body);
    update_style(*box, [] (auto& style) {
      style.get(Any()).
        set(Visibility(Visibility::NONE)).
        set(horizontal_padding(scale_width(8))).
        set(PaddingTop(scale_height(8))).
        set(PaddingBottom(scale_height(18)));
    });
    return box;
  }

  auto make_action_box(auto on_cancel, auto on_done) {
    auto body = new QWidget();
    auto layout = make_hbox_layout(body);
    layout->addStretch(1);
    auto cancel_button = make_label_button(QObject::tr("Cancel"));
    cancel_button->setFixedWidth(scale_width(100));
    cancel_button->connect_click_signal(std::move(on_cancel));
    layout->addWidget(cancel_button);
    layout->addSpacing(scale_width(8));
    auto done_button = make_label_button(QObject::tr("Done"));
    done_button->setFixedWidth(scale_width(100));
    done_button->connect_click_signal(std::move(on_done));
    layout->addWidget(done_button);
    auto box = new Box(body);
    update_style(*box, [] (auto& style) {
      style.get(Any()).
        set(BorderTopSize(scale_height(1))).
        set(BorderTopColor(QColor(0xE0E0E0))).
        set(horizontal_padding(scale_width(8))).
        set(PaddingBottom(scale_height(8))).
        set(PaddingTop(scale_height(29)));
    });
    return box;
  }
}

AdditionalTagsWindow::AdditionalTagsWindow(
    std::shared_ptr<AdditionalTagsModel> current,
    AdditionalTagDatabase additional_tags,
    std::shared_ptr<DestinationModel> destination,
    std::shared_ptr<RegionModel> region, QWidget* parent)
    : Window(parent),
      m_current(std::move(current)),
      m_additional_tags(std::move(additional_tags)),
      m_destination(std::move(destination)),
      m_region(std::move(region)) {
  setWindowFlags(windowFlags() & ~Qt::WindowMinimizeButtonHint);
  setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
  setWindowTitle(tr("Additional Tags"));
  set_svg_icon(":/Icons/key-bindings.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/key-bindings.png"));
  setFixedSize(scale(272, 384));
  m_tags = make_tags_table(
    m_current->get(), m_additional_tags, m_destination, m_region);
  auto body = new QWidget();
  auto layout = make_vbox_layout(body);
  auto table_view = make_table_view(
    m_tags, std::bind_front(&AdditionalTagsWindow::make_item, this));
  layout->addWidget(table_view);
  m_error_message = std::make_shared<LocalTextModel>();
  auto error_box = make_error_box(m_error_message);
  layout->addWidget(error_box);
  auto actions_box = make_action_box(
    std::bind_front(&AdditionalTagsWindow::on_cancel, this),
    std::bind_front(&AdditionalTagsWindow::on_done, this));
  layout->addWidget(actions_box);
  set_body(body);
}

void AdditionalTagsWindow::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    on_cancel();
    return;
  }
  QWidget::keyPressEvent(event);
}

EditableBox* AdditionalTagsWindow::make_key_item(
    const std::shared_ptr<TableModel>& table, int row, int column) const {
  auto current = make_table_value_model<int>(table, row, column);
  auto available_tags =
    static_cast<AppendableTableModel&>(*m_tags).m_available_tags[row];
  return new EditableBox(
    *new AdditionalTagKeyBox(std::move(current), std::move(available_tags),
      m_additional_tags, m_destination, m_region));
}

EditableBox* AdditionalTagsWindow::make_value_item(
    const std::shared_ptr<TableModel>& table, int row, int column) const {
  const auto KEY_COLUMN = 0;
  auto key = make_table_value_model<int>(table, row, KEY_COLUMN);
  auto value =
    make_table_value_model<optional<Nexus::Tag::Type>>(table, row, column);
  auto schema = std::make_shared<KeyToSchemaModel>(
    std::move(key), m_additional_tags, m_destination, m_region);
  return new EditableBox(*new AdditionalTagValueBox(value, schema));
}

EditableBox* AdditionalTagsWindow::make_item(
    const std::shared_ptr<TableModel>& table, int row, int column) const {
  const auto KEY_COLUMN = 0;
  const auto VALUE_COLUMN = 1;
  if(column == KEY_COLUMN) {
    return make_key_item(table, row, column);
  } else if(column == VALUE_COLUMN) {
    return make_value_item(table, row, column);
  }
  throw std::runtime_error("Invalid column.");
}

void AdditionalTagsWindow::commit() {
  auto updated_tags = std::vector<AdditionalTag>();
  for(auto i = 0; i < m_tags->get_row_size() - 1; ++i) {
    auto key = m_tags->get<int>(i, 0);
    if(key != 0) {
      auto tag =
        AdditionalTag(key, m_tags->get<optional<Nexus::Tag::Type>>(i, 1));
      updated_tags.push_back(std::move(tag));
    }
  }
  m_current->set(updated_tags);
}

void AdditionalTagsWindow::on_cancel() {
  close();
}

void AdditionalTagsWindow::on_done() {
  commit();
  close();
}
