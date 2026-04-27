#include "Spire/Ui/TaskStateBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Spire;

namespace {
  TaskStateBox::Settings setup() {
    static auto settings = [] {
      auto settings = TaskStateBox::Settings();
      auto cases = std::make_shared<ArrayListModel<Task::State>>();
      cases->push(Task::State::READY);
      cases->push(Task::State::INITIALIZING);
      cases->push(Task::State::ACTIVE);
      cases->push(Task::State::PENDING_CANCEL);
      cases->push(Task::State::CANCELED);
      cases->push(Task::State::COMPLETE);
      cases->push(Task::State::FAILED);
      settings.m_cases = std::move(cases);
      return settings;
    }();
    return settings;
  }
}

TaskStateBox* Spire::make_task_state_box(QWidget* parent) {
  return make_task_state_box(Task::State::READY, parent);
}

TaskStateBox* Spire::make_task_state_box(Task::State current, QWidget* parent) {
  return make_task_state_box(
    std::make_shared<LocalTaskStateModel>(current), parent);
}

TaskStateBox* Spire::make_task_state_box(
    std::shared_ptr<TaskStateModel> current, QWidget* parent) {
  auto settings = setup();
  settings.m_current = std::move(current);
  return new TaskStateBox(std::move(settings), parent);
}
