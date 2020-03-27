#include "Spire/KeyBindings/KeyBindingsWindow.hpp"

using namespace boost::signals2;
using namespace Spire;

KeyBindingsWindow::KeyBindingsWindow(const KeyBindings& key_bindings,
  QWidget* parent)
  : Window(parent),
    m_key_bindings(key_bindings) {}

const KeyBindings& KeyBindingsWindow::get_key_bindings() const {
  return m_key_bindings;
}

connection KeyBindingsWindow::connect_apply_signal(
    const ApplySignal::slot_type& slot) const {
  return scoped_connection();
}
