#include "Spire/UiViewer/DecimalSpinBoxTestWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  optional<double> get_value(const QString& text) {
    auto ok = false;
    auto value = text.toDouble(&ok);
    if(ok) {
      return value;
    }
    return none;
  }
}

DecimalSpinBoxTestWidget::DecimalSpinBoxTestWidget(QWidget* parent)
    : SpinBoxAdapter(parent),
      m_spin_box(nullptr) {
  m_layout = new QHBoxLayout(this);
  m_layout->setContentsMargins({});
}

bool DecimalSpinBoxTestWidget::reset(const QString& initial,
    const QString& minimum, const QString& maximum, const QString& increment) {
  auto initial_value = get_value(initial);
  auto min_value = get_value(minimum);
  auto max_value = get_value(maximum);
  auto increment_value = get_value(increment);
  if(initial_value && min_value && max_value && increment_value &&
      *min_value < *max_value && *min_value <= *initial_value &&
      *initial_value <= *max_value) {
    m_model = std::make_shared<DecimalSpinBoxModel>(*initial_value, *min_value,
      *max_value, *increment_value);
    delete_later(m_spin_box);
    m_spin_box = new DecimalSpinBox(m_model, this);
    setFocusProxy(m_spin_box);
    m_spin_box->setFixedSize(scale(100, 26));
    m_spin_box->connect_change_signal([=] (auto value) {
      m_change_signal(QString::number(value, 'g', 10));
    });
    m_layout->addWidget(m_spin_box);
    return true;
  }
  return false;
}

QString DecimalSpinBoxTestWidget::get_initial() const {
  return QString::number(m_model->get_initial());
}

bool DecimalSpinBoxTestWidget::set_initial(const QString& initial) {
  if(auto initial_value = get_value(initial);
      m_model->get_minimum() <= *initial_value &&
      *initial_value <= m_model->get_maximum()) {
    m_model->set_initial(*initial_value);
    return true;
  }
  return false;
}

QString DecimalSpinBoxTestWidget::get_minimum() const {
  return QString::number(m_model->get_minimum());
}

bool DecimalSpinBoxTestWidget::set_minimum(const QString& minimum) {
  if(auto min_value = get_value(minimum);
      *min_value <= m_model->get_maximum()) {
    m_model->set_minimum(*min_value);
    return true;
  }
  return false;
}

QString DecimalSpinBoxTestWidget::get_maximum() const {
  return QString::number(m_model->get_maximum());
}

bool DecimalSpinBoxTestWidget::set_maximum(const QString& maximum) {
  if(auto max_value = get_value(maximum);
      *max_value >= m_model->get_minimum()) {
    m_model->set_maximum(*max_value);
    return true;
  }
  return false;
}

QString DecimalSpinBoxTestWidget::get_increment(
    Qt::KeyboardModifiers modifiers) const {
  return QString::number(m_model->get_increment(modifiers));
}

bool DecimalSpinBoxTestWidget::set_increment(Qt::KeyboardModifiers modifiers,
    const QString& increment) {
  if(auto increment_value = get_value(increment); increment_value) {
    m_model->set_increment(modifiers, *increment_value);
    return true;
  }
  return false;
}

connection DecimalSpinBoxTestWidget::connect_change_signal(
    const ChangeSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}
