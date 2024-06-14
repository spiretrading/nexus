#include "Spire/KeyBindings/AdditionalTagsWindow.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/EditableTableView.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextAreaBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct ItemBuilder {
    EditableBox* mount(
        const std::shared_ptr<TableModel>& table, int row, int column) {
      return new EditableBox(*new AnyInputBox(*new TextBox()));
    }

    void unmount(QWidget* widget) {
      widget->deleteLater();
    }
  };

  auto make_tags_table(const std::vector<AdditionalTag>& tags) {
    auto table = std::make_shared<ArrayTableModel>();
    for(auto& tag : tags) {
      auto row = std::vector<std::any>();
      row.push_back(tag);
      row.push_back(tag);
      table->push(std::move(row));
    }
    return table;
  }

  auto make_table_view(std::shared_ptr<TableModel> tags) {
    static const auto DELETE_COLUMN = 0;
    static const auto NAME_COLUMN = 1;
    static const auto VALUE_COLUMN = 2;
    auto table_builder = EditableTableViewBuilder(std::move(tags));
    table_builder.set_item_builder(ItemBuilder());
    table_builder.add_header_item(QObject::tr("Name"));
    table_builder.add_header_item(QObject::tr("Value"));
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
      m_current(std::move(current)) {
  setWindowFlags(windowFlags() & ~Qt::WindowMinimizeButtonHint);
  setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
  setWindowTitle(tr("Additional Tags"));
  set_svg_icon(":/Icons/key-bindings.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/key-bindings.png"));
  setFixedSize(scale(272, 384));
  auto body = new QWidget();
  auto layout = make_vbox_layout(body);
  m_tags = make_tags_table(m_current->get());
  auto table_view = make_table_view(m_tags);
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

void AdditionalTagsWindow::commit() {
  auto updated_tags = std::vector<AdditionalTag>();
  for(auto i = 0; i != m_tags->get_row_size(); ++i) {
    auto tag = AdditionalTag(m_tags->get<AdditionalTag>(i, 0).m_key,
      m_tags->get<AdditionalTag>(i, 1).m_value);
    updated_tags.push_back(std::move(tag));
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
