#include "Spire/Ui/DateBox.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::gregorian;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto field_style(StyleSheet style, int leading_zeros) {
    style.get(Any()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_size(0)).
      set(horizontal_padding(scale_width(0))).
      set(LeadingZeros(leading_zeros)).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    style.get(Any() > is_a<Button>()).set(Visibility::NONE);
    return style;
  }

  auto make_integer_field(int min, int max, const QString& placeholder,
      int leading_zeros, const QSize& size) {
    auto model = std::make_shared<LocalOptionalIntegerModel>();
    model->set_minimum(min);
    model->set_maximum(max);
    auto field = new IntegerBox(model, {});
    field->set_placeholder(placeholder);
    set_style(*field, field_style(get_style(*field), leading_zeros));
    field->setFixedSize(size);
    return field;
  }

  auto make_dash() {
    auto dash = make_label(QObject::tr("-"));
    dash->setFixedSize(scale(10, 26));
    auto style = get_style(*dash);
    style.get(Disabled() && ReadOnly()).
      set(TextAlign(Qt::AlignCenter));
    set_style(*dash, std::move(style));
    return dash;
  }
}

DateBox::DateBox(date current, QWidget* parent)
  : DateBox(std::make_shared<LocalOptionalDateModel>(current), parent) {}

DateBox::DateBox(std::shared_ptr<OptionalDateModel> model, QWidget* parent)
    : QWidget(parent),
      m_focus_observer(*this),
      m_model(std::move(model)) {
  m_focus_observer.connect_state_signal([=] (auto state) {
    on_focus(state);
  });
  auto body = new QWidget(this);
  auto body_layout = new QHBoxLayout(body);
  body_layout->setContentsMargins({});
  body_layout->setSpacing(0);
  body_layout->addStretch(1);
  auto input_box = make_input_box(body, this);
  auto input_box_style = get_style(*input_box);
  input_box_style.get(Any()).
    set(vertical_padding(0));
  set_style(*input_box, std::move(input_box_style));
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(input_box);
  m_year_field = make_integer_field(0, 9999, tr("YYYY"), 4, scale(38, 26));
  body_layout->addWidget(m_year_field);
  m_year_dash = make_dash();
  body_layout->addWidget(m_year_dash);
  m_month_field = make_integer_field(1, 12, tr("MM"), 2, scale(28, 26));
  body_layout->addWidget(m_month_field);
  body_layout->addWidget(make_dash());
  m_day_field = make_integer_field(1, 31, tr("DD"), 2, scale(28, 26));
  body_layout->addWidget(m_day_field);
  body_layout->addStretch(1);
  auto calendar = new CalendarDatePicker(m_model, this);
  m_panel = new OverlayPanel(*calendar, *this);
  m_panel->set_is_draggable(false);
  calendar->adjustSize();
  setFixedWidth(calendar->width() + 2);
}

const std::shared_ptr<OptionalDateModel>& DateBox::get_model() const {
  return m_model;
}

connection DateBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

connection DateBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}

void DateBox::on_focus(FocusObserver::State state) {
  if(is_set(FocusObserver::State::FOCUS_IN, state)) {
    m_panel->show();
  } else {
    m_panel->hide();
  }
}
