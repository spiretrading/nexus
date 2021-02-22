#include "Spire/Ui/DecimalBox.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Spire;

DecimalBox::DecimalBox(Decimal initial, Decimal minimum, Decimal maximum,
    QHash<Qt::KeyboardModifier, Decimal> modifiers,
    QWidget* parent)
    : QWidget(parent),
      m_modifiers(std::move(modifiers)) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  auto m_text_box = new TextBox(this);
  setFocusProxy(m_text_box);
  layout->addWidget(m_text_box);
}

DecimalBox::Decimal DecimalBox::get_current() const {
  // TODO: get current from text box.
  return Decimal();
}

void DecimalBox::set_current(Decimal current) {
  // TODO: set current in text box.
}

DecimalBox::Decimal DecimalBox::get_minimum() const {
  return m_minimum;
}

void DecimalBox::set_minimum(Decimal minimum) {
  m_minimum = minimum;
  // TODO: update
}

DecimalBox::Decimal DecimalBox::get_maximum() const {
  return m_maximum;
}

void DecimalBox::set_maximum(Decimal maximum) {
  m_maximum = maximum;
  // TODO: update;
}

DecimalBox::Decimal DecimalBox::get_increment(
    Qt::KeyboardModifier modifier) const {
  return m_modifiers[modifier];
}

void DecimalBox::set_increment(Qt::KeyboardModifier modifier,
    Decimal increment) {
  m_modifiers[modifier] = increment;
}

int DecimalBox::get_decimal_places() const {
  return m_decimal_places;
}

void DecimalBox::set_decimal_places(int decimal_places) {
  m_decimal_places = decimal_places;
  // TODO: update
}

bool DecimalBox::has_trailing_zeros() const {
  return m_has_trailing_zeros;
}

void DecimalBox::set_trailing_zeros(bool has_trailing_zeros) {
  m_has_trailing_zeros = has_trailing_zeros;
  // TODO: update
}

void DecimalBox::set_read_only(bool is_read_only) {
  m_text_box->setReadOnly(is_read_only);
  // TODO: what to do with this?
}

connection DecimalBox::connect_current_signal(
    const CurrentSignal::slot_type& slot) const {
  return m_current_signal.connect(slot);
}

connection DecimalBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}
