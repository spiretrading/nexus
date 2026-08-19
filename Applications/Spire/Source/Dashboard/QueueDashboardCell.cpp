#include "Spire/Dashboard/QueueDashboardCell.hpp"
#include <vector>
#include <QTimer>

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace std;

namespace {
  const auto UPDATE_INTERVAL = 250;
}

class QueueDashboardCell::Updater {
  public:
    static Updater& GetInstance() {
      static auto updater = Updater();
      return updater;
    }

    void Add(QueueDashboardCell& cell) {
      cell.m_updateIndex = static_cast<int>(m_cells.size());
      m_cells.push_back(&cell);
      if(m_cells.size() == 1) {
        m_timer.start(UPDATE_INTERVAL);
      }
    }

    void Remove(QueueDashboardCell& cell) {
      m_cells.back()->m_updateIndex = cell.m_updateIndex;
      m_cells[cell.m_updateIndex] = m_cells.back();
      m_cells.pop_back();
      if(m_cells.empty()) {
        m_timer.stop();
      }
    }

  private:
    QTimer m_timer;
    std::vector<QueueDashboardCell*> m_cells;

    Updater() {
      QObject::connect(&m_timer, &QTimer::timeout, [=, this] {
        OnUpdateTimer();
      });
    }

    void OnUpdateTimer() {
      for(auto i = std::size_t(0); i < m_cells.size(); ++i) {
        m_cells[i]->Update();
      }
    }
};

QueueDashboardCell::QueueDashboardCell(
    std::shared_ptr<QueueReader<Value>> queue)
    : m_queue{std::move(queue)},
      m_values(1) {
  Updater::GetInstance().Add(*this);
}

QueueDashboardCell::~QueueDashboardCell() {
  Updater::GetInstance().Remove(*this);
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

void QueueDashboardCell::Update() {
  while(auto value = m_queue->try_pop()) {
    m_values.push_back(std::move(*value));
    m_updateSignal(m_values.back());
  }
}
