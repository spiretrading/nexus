#include "Spire/Blotter/ControllerPortfolioModel.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

ControllerPortfolioModel::ControllerPortfolioModel(
    std::shared_ptr<Controller> controller)
    : m_controller(std::move(controller)) {
  auto& publisher = m_controller->get_publisher();
  publisher.with([&] {
    auto snapshot = optional<Controller::Portfolio*>();
    publisher.monitor(m_events.get_slot<Type>(
      std::bind_front(&ControllerPortfolioModel::on_update, this)),
      out(snapshot));
    if(snapshot) {
      for_each(**snapshot,
        std::bind_front(&ControllerPortfolioModel::on_update, this));
    }
  });
}

ControllerPortfolioModel::~ControllerPortfolioModel() = default;

int ControllerPortfolioModel::get_size() const {
  return m_entries.get_size();
}

const ControllerPortfolioModel::Type&
    ControllerPortfolioModel::get(int index) const {
  return m_entries.get(index);
}

connection ControllerPortfolioModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_entries.connect_operation_signal(slot);
}

void ControllerPortfolioModel::start_transaction() {
  m_entries.start_transaction();
}

void ControllerPortfolioModel::end_transaction() {
  m_entries.end_transaction();
}

void ControllerPortfolioModel::on_update(const PortfolioUpdateEntry& update) {
  auto [entry, is_inserted] = m_indices.try_emplace(
    update.m_inventory.m_position.m_ticker, m_entries.get_size());
  if(is_inserted) {
    m_entries.push(update);
  } else {
    m_entries.set(entry->second, update);
  }
}
