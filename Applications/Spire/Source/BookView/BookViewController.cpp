#include "Spire/BookView/BookViewController.hpp"
#include "Spire/BookView/ServicesBookViewModel.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

BookViewController::BookViewController(Definitions definitions,
    Ref<SecurityInputModel> security_input_model,
    Ref<VirtualServiceClients> service_clients)
    : m_definitions(std::move(definitions)),
      m_security_input_model(security_input_model.Get()),
      m_service_clients(service_clients.Get()),
      m_window(nullptr) {}

BookViewController::~BookViewController() {
  close();
}

void BookViewController::open() {
  if(m_window != nullptr) {
    return;
  }
  m_window = new BookViewWindow(BookViewProperties(),
    Ref(*m_security_input_model));
  m_security_change_connection = m_window->connect_security_change_signal(
    [=] (const auto& security) { on_change_security(security); });
  // TODO
  //m_window->connect_closed_signal([=] { on_closed(); });
  m_window->show();
}

void BookViewController::close() {
  if(m_window == nullptr) {
    return;
  }
  deleteLater(m_window);
  m_closed_signal();
}

connection BookViewController::connect_closed_signal(
    const ClosedSignal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void BookViewController::on_change_security(const Security& security) {
  auto model = std::make_shared<ServicesBookViewModel>(security,
    m_definitions, Ref(*m_service_clients));
  m_window->set_model(model);
}

void BookViewController::on_closed() {
  close();
}
