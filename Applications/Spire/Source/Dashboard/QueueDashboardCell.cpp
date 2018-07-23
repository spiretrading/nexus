#include "Spire/Dashboard/QueueDashboardCell.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace std;

namespace {
  const unsigned int UPDATE_INTERVAL = 500;
}

QueueDashboardCell::QueueDashboardCell(
    std::shared_ptr<QueueReader<Value>> queue)
    : m_queue{std::move(queue)},
      m_values(30) {
  QObject::connect(&m_updateTimer, &QTimer::timeout,
    std::bind(&QueueDashboardCell::OnUpdateTimer, this));
  m_updateTimer.start(UPDATE_INTERVAL);
}

void QueueDashboardCell::SetBufferSize(int size) {
  m_values.set_capacity(size);
}

const circular_buffer<QueueDashboardCell::Value>&
    QueueDashboardCell::GetValues() const {
  return m_values;
}

connection QueueDashboardCell::ConnectUpdateSignal(
    const UpdateSignal::slot_function_type& slot) const {
  return m_updateSignal.connect(slot);
}

void QueueDashboardCell::OnUpdateTimer() {
  while(!m_queue->IsEmpty()) {
    m_values.push_back(std::move(m_queue->Top()));
    m_queue->Pop();
    m_updateSignal(m_values.back());
  }
}
