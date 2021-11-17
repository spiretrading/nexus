#include "Spire/Ui/DateBox.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ScalarValueModelDecorator.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).set(DateFormat::YYYYMMDD);
    return style;
  }

  auto make_year_box(const std::shared_ptr<OptionalDateModel>& model) {
    auto year_model =
      std::make_shared<ScalarValueModelDecorator<optional<int>>>(
        make_transform_value_model(model,
          [] (const auto& current) -> optional<int> {
            if(current) {
              return static_cast<int>(current->year());
            }
            return none;
          },
          [] (const auto& current, auto value) {
            if(current && value) {
              return make_optional(
                date(*value, current->month(), current->day()));
            }
            return current;
          }));
    auto box = new IntegerBox(year_model, {});
    box->setFixedSize(scale(38, 26));
    return box;
  }

  auto make_month_box(const std::shared_ptr<OptionalDateModel>& model) {
    auto month_model =
      std::make_shared<ScalarValueModelDecorator<optional<int>>>(
        make_transform_value_model(model,
          [] (const auto& current) -> optional<int> {
            if(current) {
              return static_cast<int>(current->month());
            }
            return none;
          },
          [] (const auto& current, auto value) {
            if(current && value) {
              return make_optional(
                date(current->year(), *value, current->day()));
            }
            return current;
          }));
    auto box = new IntegerBox(month_model, {});
    box->setFixedSize(scale(28, 26));
    return box;
  }

  auto make_day_box(const std::shared_ptr<OptionalDateModel>& model) {
    auto day_model =
      std::make_shared<ScalarValueModelDecorator<optional<int>>>(
        make_transform_value_model(model,
          [] (const auto& current) -> optional<int> {
            if(current) {
              return static_cast<int>(current->day());
            }
            return none;
          },
          [] (const auto& current, auto value) {
            if(current && value) {
              return make_optional(
                date(current->year(), current->month(), *value));
            }
            return current;
          }));
    auto box = new IntegerBox(day_model, {});
    box->setFixedSize(scale(28, 26));
    return box;
  }

  auto make_dash() {
    auto label = make_label("-");
    label->setFixedSize(scale(10, 26));
    return label;
  }

  auto make_body(const std::shared_ptr<OptionalDateModel>& model) {
    auto body = new QWidget();
    auto layout = new QHBoxLayout(body);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addSpacerItem(
      new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
    auto year_box = make_year_box(model);
    layout->addWidget(year_box);
    auto year_dash = make_dash();
    layout->addWidget(year_dash);
    auto month_box = make_month_box(model);
    layout->addWidget(month_box);
    layout->addWidget(make_dash());
    auto day_box = make_day_box(model);
    layout->addWidget(day_box);
    layout->addSpacerItem(
      new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
    return std::tuple(year_box, year_dash, month_box, day_box, body);
  }

  auto make_date_picker(
      const std::shared_ptr<OptionalDateModel>& model, QWidget* parent) {
    auto date_picker = new CalendarDatePicker(model);
    auto panel = new OverlayPanel(*date_picker, *parent);
    return panel;
  }
}

DateBox::DateBox(const optional<date>& current, QWidget* parent)
  : DateBox(std::make_shared<LocalOptionalDateModel>(current), parent) {}

DateBox::DateBox(std::shared_ptr<OptionalDateModel> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_focus_observer(*this),
      m_is_read_only(false),
      m_format(DateFormat::YYYYMMDD) {
  std::tie(m_year_box, m_year_dash, m_month_box, m_day_box, m_body) =
    make_body(m_model);
  auto input_box = make_input_box(m_body);
  auto style = get_style(*input_box);
  style.get(Any()).set(vertical_padding(0));
  set_style(*input_box, std::move(style));
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(input_box);
  set_style(*this, DEFAULT_STYLE());
  m_date_picker = make_date_picker(m_model, this);
  m_style_connection = connect_style_signal(*this, [=] { on_style(); });
  m_focus_observer.connect_state_signal([=] (auto state) { on_focus(state); });
}

const std::shared_ptr<OptionalDateModel>& DateBox::get_model() const {
  return m_model;
}

bool DateBox::is_read_only() const {
  return m_is_read_only;
}

void DateBox::set_read_only(bool read_only) {
  if(m_is_read_only == read_only) {
    return;
  }
  m_is_read_only = read_only;
  m_year_box->set_read_only(m_is_read_only);
  m_month_box->set_read_only(m_is_read_only);
  m_day_box->set_read_only(m_is_read_only);
  auto& layout = *static_cast<QHBoxLayout*>(m_body->layout());
  if(m_is_read_only) {
    layout.removeItem(layout.itemAt(0));
    match(*this, ReadOnly());
  } else {
    layout.insertItem(
      0, new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
    unmatch(*this, ReadOnly());
  }
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
  if(is_set(state, FocusObserver::State::FOCUS_IN)) {
    m_date_picker->show();
  } else {
    m_date_picker->hide();
  }
}

void DateBox::on_style() {
  auto& stylist = find_stylist(*this);
  auto& block = stylist.get_computed_block();
  for(auto& property : block) {
    property.visit(
      [&] (const EnumProperty<DateFormat>& format) {
        stylist.evaluate(format, [=] (auto format) {
          if(format == m_format) {
            return;
          }
          m_format = format;
          m_year_box->setVisible(m_format == DateFormat::YYYYMMDD);
          m_year_dash->setVisible(m_format == DateFormat::YYYYMMDD);
        });
      });
  }
}
