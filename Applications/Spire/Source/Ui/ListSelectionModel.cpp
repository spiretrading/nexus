#include "Spire/Ui/ListSelectionModel.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/MultiSelectionModel.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

ListSelectionModel::ListSelectionModel()
    : m_mode(Mode::SINGLE),
      m_model(std::make_shared<ListSingleSelectionModel>()) {
  m_model->connect_operation_signal(m_operation_signal);
}

ListSelectionModel::Mode ListSelectionModel::get_mode() const {
  return m_mode;
}

void ListSelectionModel::set_mode(Mode mode) {
  if(mode == m_mode) {
    return;
  }
  m_mode = mode;
  auto previous = m_model;
  if(m_mode == Mode::NONE) {
    m_model->transact([&] {
      while(m_model->get_size() != 0) {
        m_model->remove(m_model->end() - 1);
      }
    });
    m_model = std::make_shared<ListEmptySelectionModel>();
  } else if(m_mode == Mode::SINGLE) {
    m_model->transact([&] {
      while(m_model->get_size() > 1) {
        m_model->remove(m_model->end() - 1);
      }
    });
    m_model = std::make_shared<ListSingleSelectionModel>();
    if(previous->get_size() >= 1) {
      m_model->push(previous->get(previous->get_size() - 1));
    }
  } else {
    m_model = std::make_shared<ListMultiSelectionModel>();
    for(auto i = 0; i != previous->get_size(); ++i) {
      m_model->push(previous->get(i));
    }
  }
  m_model->connect_operation_signal(m_operation_signal);
}

int ListSelectionModel::get_size() const {
  return m_model->get_size();
}

const int& ListSelectionModel::get(int index) const {
  return m_model->get(index);
}

QValidator::State ListSelectionModel::set(int index, const int& value) {
  return m_model->set(index, value);
}

QValidator::State ListSelectionModel::insert(const int& value, int index) {
  return m_model->insert(value, index);
}

QValidator::State ListSelectionModel::move(int source, int destination) {
  return m_model->move(source, destination);
}

QValidator::State ListSelectionModel::remove(int index) {
  return m_model->remove(index);
}

connection ListSelectionModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_operation_signal.connect(slot);
}

void ListSelectionModel::transact(const std::function<void ()>& transaction) {
  m_model->transact([&] { transaction(); });
}
