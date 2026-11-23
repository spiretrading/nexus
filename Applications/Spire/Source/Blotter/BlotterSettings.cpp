#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterController.hpp"

using namespace Spire;

BlotterSettings::BlotterSettings()
  : m_is_consolidated(false) {}

BlotterSettings::BlotterSettings(const BlotterController& controller)
    : m_id(controller.m_id),
      m_name(controller.get_model()->get_name()->get()),
      m_account(controller.get_model()->get_account()),
      m_is_consolidated(controller.get_model()->is_consolidated()) {
  for(auto& monitor :
      std::as_const(*controller.get_model()->get_task_monitors())) {
    auto saved = Monitor();
    saved.m_name = monitor.m_name;
    if(monitor.m_expression) {
      saved.m_expression = CanvasNode::Clone(*monitor.m_expression);
    }
    saved.m_default_width = monitor.m_default_width;
    m_monitors.push_back(std::move(saved));
  }
  if(controller.m_window) {
    m_window.emplace(*controller.m_window);
  } else {
    m_window = controller.m_window_settings;
  }
}

void BlotterSettings::apply(BlotterController& controller) const {
  controller.m_id = m_id;
  auto& model = *controller.get_model();
  model.get_name()->set(m_name);
  auto& monitors = *model.get_task_monitors();
  monitors.transact([&] {
    while(monitors.get_size() != 0) {
      monitors.remove(monitors.get_size() - 1);
    }
    for(auto& monitor : m_monitors) {
      auto restored = BlotterTaskMonitor();
      restored.m_name = monitor.m_name;
      if(monitor.m_expression) {
        restored.m_expression = CanvasNode::Clone(*monitor.m_expression);
      }
      restored.m_default_width = monitor.m_default_width;
      monitors.push(restored);
    }
  });
  controller.m_window_settings = m_window;
}
