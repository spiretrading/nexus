#include "Spire/Blotter/BlotterTaskView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ArrayTableModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  enum class CommandItem {
    ACTIVATE,
    PIN,
    SEPARATOR,
    RUN,
    CANCEL
  };

  using Separator = StateSelector<void, struct SeparatorTag>;
}

BlotterTaskView::BlotterTaskView(std::shared_ptr<BooleanModel> is_active,
    std::shared_ptr<BooleanModel> is_pinned,
    std::shared_ptr<TaskListModel> tasks, QWidget* parent)
    : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto command_list = std::make_shared<ArrayListModel<CommandItem>>();
  command_list->push(CommandItem::ACTIVATE);
  command_list->push(CommandItem::PIN);
  command_list->push(CommandItem::SEPARATOR);
  command_list->push(CommandItem::RUN);
  command_list->push(CommandItem::CANCEL);
  auto commands = new ListView(command_list,
    [] (const std::shared_ptr<ListModel<CommandItem>>& model, auto index) ->
        QWidget* {
      if(model->get(index) == CommandItem::SEPARATOR) {
        auto separator = new Box(nullptr, nullptr);
        match(*separator, Separator());
        auto style = StyleSheet();
        style.get(Any()).set(BackgroundColor(QColor(0xC8C8C8)));
        set_style(*separator, std::move(style));
        separator->setFixedSize(scale(1, 14));
        return separator;
      }
      auto [path, shortcut] = [&] {
        if(model->get(index) == CommandItem::ACTIVATE) {
          return std::tuple(
            ":/Icons/blotter/tasks/active.svg", tr("Set as active blotter"));
        } else if(model->get(index) == CommandItem::PIN) {
          return std::tuple(
            ":/Icons/blotter/tasks/pin.svg", tr("Pin blotter"));
        } else if(model->get(index) == CommandItem::RUN) {
          return std::tuple(":/Icons/blotter/tasks/run.svg", tr("Run task"));
        }
        return std::tuple(
          ":/Icons/blotter/tasks/cancel.svg", tr("Cancel task"));
      }();
      return make_icon_button(
        imageFromSvg(path, scale(26, 26)), std::move(shortcut));
    });
  update_style(*commands, [] (auto& style) {
    style.get(Any()).
      set(Qt::Orientation::Horizontal).
      set(EdgeNavigation::CONTAIN);
    style.get(Any() > is_a<ListItem>()).
      set(padding(0)).
      set(border(0, QColor(Qt::transparent))).
      set(BackgroundColor(QColor(Qt::transparent)));
    style.get(Any() > (+is_a<ListItem>() > Separator())).
      set(horizontal_padding(scale_width(8)));
  });
  auto command_bar = new Box(commands, nullptr);
  command_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  update_style(*command_bar, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(BorderBottomSize(scale_height(1))).
      set(BorderBottomColor(QColor(0xC8C8C8))).
      set(padding(scale_width(8)));
  });
  auto layout = make_vbox_layout(this);
  layout->addWidget(command_bar);
  auto table_view_builder =
    TableViewBuilder(std::make_shared<ArrayTableModel>());
  table_view_builder.add_header_item(tr("ID"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(tr("Name"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(tr("Pinned"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(tr("State"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(tr("Side"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(tr("Price"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(tr("Quantity"), TableFilter::Filter::NONE);
  table_view_builder.add_header_item(tr("Volume"), TableFilter::Filter::NONE);
  auto table = table_view_builder.make();
  table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto scroll_box = new ScrollBox(table);
  scroll_box->set(
    ScrollBox::DisplayPolicy::ON_OVERFLOW, ScrollBox::DisplayPolicy::NEVER);
  layout->addWidget(scroll_box);
}
