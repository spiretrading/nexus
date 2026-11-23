#include "Spire/Blotter/BlotterController.hpp"
#include <QEvent>

using namespace Spire;

class BlotterController::EventFilter : public QObject {
  public:
    explicit EventFilter(BlotterController& controller)
      : m_controller(&controller) {}

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::Close) {
        m_controller->on_close();
      }
      return false;
    }

  private:
    BlotterController* m_controller;
};

BlotterController::BlotterController(std::shared_ptr<BlotterModel> model,
  std::shared_ptr<BlotterCollection> collection,
  std::shared_ptr<CancelKeyBindingsModel> cancel_bindings,
  std::shared_ptr<PositionKeyBindingsModel> position_bindings)
  : m_model(std::move(model)),
    m_collection(std::move(collection)),
    m_cancel_bindings(std::move(cancel_bindings)),
    m_position_bindings(std::move(position_bindings)),
    m_is_open(std::make_shared<LocalBooleanModel>(false)),
    m_read_only_is_open(m_is_open) {}

BlotterController::~BlotterController() {
  if(m_window) {
    m_window->removeEventFilter(m_event_filter.get());
    m_window.reset();
  }
  if(m_event_filter) {
    m_event_filter.release()->deleteLater();
  }
  if(m_is_open->get()) {
    m_is_open->set(false);
  }
}

const std::shared_ptr<BlotterModel>& BlotterController::get_model() const {
  return m_model;
}

const std::shared_ptr<const BooleanModel>& BlotterController::is_open() const {
  return m_read_only_is_open;
}

BlotterWindow& BlotterController::open() {
  if(!m_window) {
    if(!m_event_filter) {
      m_event_filter = std::make_unique<EventFilter>(*this);
    }
    m_window = std::make_unique<BlotterWindow>(
      m_model, m_collection, m_cancel_bindings, m_position_bindings);
    m_window->installEventFilter(m_event_filter.get());
  }
  m_window->show();
  m_window->raise();
  m_window->activateWindow();
  if(!m_is_open->get()) {
    auto is_open = m_is_open;
    is_open->set(true);
  }
  return *m_window;
}

void BlotterController::close() {
  if(m_window) {
    m_window->close();
  }
}

void BlotterController::on_close() {
  auto window = m_window.release();
  window->removeEventFilter(m_event_filter.get());
  window->deleteLater();
  auto is_open = m_is_open;
  is_open->set(false);
}
