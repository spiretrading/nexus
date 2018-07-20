#include "spire/book_view/book_view_window.hpp"
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

book_view_window::book_view_window(const book_view_properties& properties,
    security_input_model& input_model, QWidget* parent)
    : QWidget(parent) {
  auto body = new QWidget(this);
  body->setMinimumSize(scale(232, 280));
  resize(scale(232, 410));
  body->setStyleSheet("background-color: #FFFFFF;");
  auto window_layout = new QVBoxLayout(this);
  window_layout->setContentsMargins({});
  auto window = new spire::window(body, this);
  setWindowTitle(tr("Book View"));
  window->set_svg_icon(":/icons/bookview-black.svg",
    ":/icons/bookview-grey.svg");
  window_layout->addWidget(window);
  auto layout = new QVBoxLayout(body);
  layout->setContentsMargins({});
  layout->setSpacing(0);
}

void book_view_window::set_model(std::shared_ptr<book_view_model> model) {
}

const book_view_properties& book_view_window::get_properties() const {
  return m_properties;
}

void book_view_window::set_properties(const book_view_properties& properties) {
}

connection book_view_window::connect_security_change_signal(
    const change_security_signal::slot_type& slot) const {
  return m_change_security_signal.connect(slot);
}

connection book_view_window::connect_closed_signal(
    const closed_signal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}
