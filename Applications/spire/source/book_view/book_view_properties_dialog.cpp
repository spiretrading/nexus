#include "spire/book_view/book_view_properties_dialog.hpp"
#include <QHBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

book_view_properties_dialog::book_view_properties_dialog(
    const book_view_properties& properties, const Security& security,
    QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags | Qt::Window | Qt::FramelessWindowHint |
        Qt::WindowCloseButtonHint) {
  auto body = new QWidget(this);
  body->setFixedSize(scale(492, 394));
  body->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  body->setStyleSheet("background-color: #F5F5F5;");
  auto window_layout = new QHBoxLayout(this);
  window_layout->setContentsMargins({});
  auto window = new spire::window(body, this);
  setWindowTitle(tr("Properties"));
  window->set_svg_icon(":/icons/bookview-black.svg",
    ":/icons/bookview-grey.svg");
  window_layout->addWidget(window);
  auto layout = new QHBoxLayout(body);
}

book_view_properties book_view_properties_dialog::get_properties() const {
  return m_properties;
}

connection book_view_properties_dialog::connect_apply_signal(
    const apply_signal::slot_type& slot) const {
  return m_apply_signal.connect(slot);
}

connection book_view_properties_dialog::connect_apply_all_signal(
    const apply_all_signal::slot_type& slot) const {
  return m_apply_all_signal.connect(slot);
}

connection book_view_properties_dialog::connect_save_default_signal(
    const save_default_signal::slot_type& slot) const {
  return m_save_default_signal.connect(slot);
}
