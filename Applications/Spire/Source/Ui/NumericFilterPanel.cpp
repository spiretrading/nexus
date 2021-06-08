#include "Spire/Ui/NumericFilterPanel.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto LABEL_STYLE(StyleSheet style) {
    style.get(Disabled()).
      set(TextColor(QColor::fromRgb(0, 0, 0)));
    return style;
  }

  auto make_value_field(const optional<Decimal>& value) {
    auto modifiers = QHash<Qt::KeyboardModifier, Decimal>(
      {{Qt::NoModifier, 1}, {Qt::AltModifier, 5}, {Qt::ControlModifier, 10},
       {Qt::ShiftModifier, 20}});
    auto model = std::make_shared<
      LocalScalarValueModel<optional<Decimal>>>(value);
    model->set_minimum(none);
    model->set_maximum(none);
    auto field = new DecimalBox(model, modifiers);
    field->setFixedSize(scale(120, 26));
    return field;
  }
}

NumericFilterPanel::NumericFilterPanel(const QString& title,
  const NumericRange& default_value, QWidget* parent)
  : NumericFilterPanel(std::make_shared<LocalValueModel<NumericRange>>(), title,
    default_value, parent) {}

NumericFilterPanel::NumericFilterPanel(
    std::shared_ptr<NumericFilterModel> model, const QString& title,
    const NumericRange& default_value, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_default_value(default_value) {
  if(m_default_value.m_min && m_default_value.m_max &&
      m_default_value.m_min > m_default_value.m_max) {
    std::swap(m_default_value.m_min, m_default_value.m_max);
  }
  auto layout = new QVBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins({});
  auto min_layout = new QHBoxLayout();
  auto min_label = new TextBox(tr("Min"));
  min_label->setEnabled(false);
  min_label->set_read_only(true);
  set_style(*min_label, LABEL_STYLE(get_style(*min_label)));
  min_layout->addWidget(min_label);
  min_layout->addStretch();
  auto min_field = make_value_field(m_default_value.m_min);
  min_layout->addWidget(min_field);
  layout->addLayout(min_layout);
  layout->addSpacing(scale_height(10));
  auto max_layout = new QHBoxLayout();
  auto max_label = new TextBox(tr("Max"));
  max_label->setEnabled(false);
  max_label->set_read_only(true);
  set_style(*max_label, LABEL_STYLE(get_style(*max_label)));
  max_layout->addWidget(max_label);
  max_layout->addStretch();
  auto max_field = make_value_field(m_default_value.m_max);
  max_layout->addWidget(max_field);
  layout->addLayout(max_layout);
  m_filter_panel = new FilterPanel(title, this, parent);
  m_filter_panel->connect_reset_signal([=] {
    m_model->set_current(m_default_value);
  });
  min_field->connect_submit_signal([=] (const auto& value) {
    auto current = m_model->get_current();
    if(current.m_max && value > *current.m_max) {
      current.m_max = value;
      max_field->get_model()->set_current(value);
    }
    current.m_min = value;
    m_model->set_current(current);
  });
  max_field->connect_submit_signal([=] (const auto& value) {
    auto current = m_model->get_current();
    if(current.m_min && value < *current.m_min) {
      current.m_min = value;
      min_field->get_model()->set_current(value);
    }
    current.m_max = value;
    m_model->set_current(current);
  });
  connect_style_signal(*min_field, [=] { on_style(min_field); });
  connect_style_signal(*max_field, [=] { on_style(max_field); });
  m_current_connection = m_model->connect_current_signal(
    [=] (const auto& value) {
      min_field->get_model()->set_current(value.m_min);
      max_field->get_model()->set_current(value.m_max);
      m_filter_signal();
    });
}

const std::shared_ptr<NumericFilterPanel::NumericFilterModel>&
NumericFilterPanel::get_model() const {
  return m_model;
}

NumericFilterPanel::NumericRange NumericFilterPanel::get_default_value() const {
  return m_default_value;
}

void NumericFilterPanel::set_default_value(const NumericRange& default_value) {
  m_default_value = default_value;
}

connection NumericFilterPanel::connect_filter_signal(
    const FilterSignal::slot_type& slot) const {
  return m_filter_signal.connect(slot);
}

bool NumericFilterPanel::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_model->set_current(m_default_value);
    m_filter_panel->show();
  }
  return QWidget::event(event);
}

void NumericFilterPanel::on_style(DecimalBox* field) {
  auto& stylist = find_stylist(*field);
  auto block = stylist.get_computed_block();
  if(auto trailing_zeros_property = Styles::find<TrailingZeros>(block)) {
    stylist.evaluate(*trailing_zeros_property, [=] (auto trailing_zeros) {
      auto exp = -trailing_zeros;
      std::dynamic_pointer_cast<
        LocalScalarValueModel<optional<Decimal>>>(
          field->get_model())->set_increment(pow(Decimal(10), exp));
    });
  }
}
