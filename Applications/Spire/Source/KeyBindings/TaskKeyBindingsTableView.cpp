#include "Spire/KeyBindings/TaskKeyBindingsTableView.hpp"
#include <QSet>
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/KeyBindings/DestinationItemDelegate.hpp"
#include "Spire/KeyBindings/KeySequenceEditor.hpp"
#include "Spire/KeyBindings/KeySequenceItemDelegate.hpp"
#include "Spire/KeyBindings/NameItemDelegate.hpp"
#include "Spire/KeyBindings/OrderTypeItemDelegate.hpp"
#include "Spire/KeyBindings/QuantityItemDelegate.hpp"
#include "Spire/KeyBindings/SecurityInputItemDelegate.hpp"
#include "Spire/KeyBindings/SideItemDelegate.hpp"
#include "Spire/KeyBindings/TimeInForceItemDelegate.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using ValidSequence = KeySequenceEditor::ValidKeySequence;

namespace {
  const auto& FUNCTION_KEYS() {
    static auto function_keys = QSet<Qt::Key>({
      Qt::Key_F1,
      Qt::Key_F2,
      Qt::Key_F3,
      Qt::Key_F4,
      Qt::Key_F5,
      Qt::Key_F6,
      Qt::Key_F7,
      Qt::Key_F8,
      Qt::Key_F9,
      Qt::Key_F10,
      Qt::Key_F11,
      Qt::Key_F12,
    });
    return function_keys;
  }
}

TaskKeyBindingsTableView::TaskKeyBindingsTableView(
    std::vector<KeyBindings::OrderActionBinding> bindings,
    Ref<SecurityInputModel> input_model, QWidget* parent)
    : KeyBindingsTableView(make_header(parent), true, parent),
      m_model(nullptr) {
  set_key_bindings(std::move(bindings));
  setFixedWidth(scale_width(871));
  set_width(scale_width(871));
  set_height(scale_height(296));
  set_column_width(0, scale_width(80));
  set_minimum_column_width(0, scale_width(80));
  set_column_width(1, scale_width(100));
  set_minimum_column_width(1, scale_width(100));
  set_column_width(2, scale_width(89));
  set_minimum_column_width(2, scale_width(89));
  set_column_width(3, scale_width(89));
  set_minimum_column_width(3, scale_width(89));
  set_column_width(4, scale_width(59));
  set_minimum_column_width(4, scale_width(59));
  set_column_width(5, scale_width(80));
  set_minimum_column_width(5, scale_width(80));
  set_column_width(6, scale_width(98));
  set_minimum_column_width(6, scale_width(98));
  set_column_width(7, scale_width(124));
  set_minimum_column_width(7, scale_width(124));
  set_column_width(8, scale_width(116));
  set_minimum_column_width(8, scale_width(116));
  set_column_delegate(0, new NameItemDelegate(this));
  set_column_delegate(1, new SecurityInputItemDelegate(input_model, this));
  set_column_delegate(2, new DestinationItemDelegate(this));
  set_column_delegate(3, new OrderTypeItemDelegate(this));
  set_column_delegate(4, new SideItemDelegate(this));
  set_column_delegate(5, new QuantityItemDelegate(this));
  set_column_delegate(6, new TimeInForceItemDelegate(this));
  auto valid_sequences = std::vector<std::vector<QSet<Qt::Key>>>(
    {ValidSequence({FUNCTION_KEYS()}),
    ValidSequence({{Qt::Key_Shift, Qt::Key_Alt, Qt::Key_Control},
    FUNCTION_KEYS()})});
  set_column_delegate(8, new KeySequenceItemDelegate(valid_sequences, this));
}

void TaskKeyBindingsTableView::set_key_bindings(
    const std::vector<KeyBindings::OrderActionBinding>& bindings) {
  m_model = new TaskKeyBindingsTableModel(bindings, this);
  connect(m_model, &QAbstractItemModel::rowsInserted,
    [=] (auto parent, auto first, auto last) {
      on_row_count_changed();
    });
  connect(m_model, &QAbstractItemModel::rowsRemoved,
    [=] (auto parent, auto first, auto last) {
      on_row_count_changed();
    });
  m_model->connect_item_modified_signal([=] (auto index) {
    on_item_modified(index);
  });
  set_model(m_model);
}

bool TaskKeyBindingsTableView::is_valid(int row, int column) const {
  return m_model->index(row, column).data().isValid();
}

void TaskKeyBindingsTableView::on_item_modified(
    const QModelIndex& index) {
  auto row = index.row();
  auto binding = KeyBindings::OrderActionBinding{};
  auto name = m_model->index(row, 0).data().value<QString>();
  if(!name.isEmpty()) {
    binding.m_action.m_name = name.toStdString();
  }
  if(is_valid(row, 1)) {
    binding.m_region = Region(m_model->index(row, 1).data().value<Security>());
  }
  if(is_valid(row, 3)) {
    binding.m_action.m_type = m_model->index(row, 3).data().value<OrderType>();
  }
  if(is_valid(row, 4)) {
    binding.m_action.m_side = m_model->index(row, 4).data().value<Side>();
  }
  if(is_valid(row, 5)) {
    binding.m_action.m_quantity =
      m_model->index(row, 5).data().value<Quantity>();
  }
  if(is_valid(row, 6)) {
    binding.m_action.m_time_in_force =
      m_model->index(row, 6).data().value<TimeInForce>();
  }
  if(is_valid(row, 8)) {
    binding.m_sequence = m_model->index(row, 8).data().value<QKeySequence>();
  }
  m_modified_signal(binding);
  update();
}

void TaskKeyBindingsTableView::on_row_count_changed() {
  if(m_model->rowCount(QModelIndex()) <= 10) {
    set_height(scale_height(296));
    return;
  }
  set_height(m_model->rowCount(QModelIndex()) * scale_height(26) +
    scale_height(30) + scale_height(8));
}
