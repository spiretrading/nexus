#include "Spire/UI/FunctionalAction.hpp"

using namespace Spire;
using namespace Spire::UI;

FunctionalAction::FunctionalAction(const std::function<void ()>& function,
    QObject* parent)
    : QAction(parent),
      m_function(function) {}

FunctionalAction::FunctionalAction(QObject* parent)
    : QAction(parent),
      m_function([] {}) {}

FunctionalAction::~FunctionalAction() {}

void FunctionalAction::Execute() {
  m_function();
}

void FunctionalAction::SetFunction(const std::function<void ()>& function) {
  m_function = function;
}
