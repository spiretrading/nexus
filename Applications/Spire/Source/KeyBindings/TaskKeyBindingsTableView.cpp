#include "Spire/KeyBindings/TaskKeyBindingsTableView.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Spire;

TaskKeyBindingsTableView::TaskKeyBindingsTableView(
    std::vector<KeyBindings::OrderActionBinding> bindings,
    QWidget* parent)
    : KeyBindingsTableView(make_header(parent), parent),
      m_model(nullptr) {
  set_key_bindings(std::move(bindings));
  setFixedWidth(scale_width(853));
  set_width(scale_width(853));
  set_height(scale_height(376));
  set_column_width(0, scale_width(80));
  set_column_width(1, scale_width(80));
  set_column_width(2, scale_width(89));
  set_column_width(3, scale_width(89));
  set_column_width(4, scale_width(59));
  set_column_width(5, scale_width(80));
  set_column_width(6, scale_width(98));
  set_column_width(7, scale_width(144));
}

void TaskKeyBindingsTableView::set_key_bindings(
    const std::vector<KeyBindings::OrderActionBinding>& bindings) {
  m_model = new TaskKeyBindingsTableModel(bindings, this);
  set_model(m_model);
}
