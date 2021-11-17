#include "Spire/Ui/DateBox.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/ScalarValueModelDecorator.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/IntegerBox.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
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
    return new IntegerBox(year_model, {});
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
    return new IntegerBox(month_model, {});
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
    return new IntegerBox(day_model, {});
  }

  auto make_body(const std::shared_ptr<OptionalDateModel>& model) {
    auto body = new QWidget();
    auto layout = new QHBoxLayout(body);
    layout->setContentsMargins({});
    layout->addWidget(make_year_box(model));
    layout->addWidget(make_month_box(model));
    layout->addWidget(make_day_box(model));
    return body;
  }
}

DateBox::DateBox(const optional<date>& current, QWidget* parent)
  : DateBox(std::make_shared<LocalOptionalDateModel>(current), parent) {}

DateBox::DateBox(std::shared_ptr<OptionalDateModel> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)) {
  auto input_box = make_input_box(make_body(m_model));
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(input_box);
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
