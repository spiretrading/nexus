#include "Spire/KeyBindings/OrderFieldAdditionalTag.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/CalendarDatePicker.hpp"
#include "Spire/Ui/DateBox.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_quantity_modifiers() {
    return QHash<Qt::KeyboardModifier, Quantity>(
      {{Qt::NoModifier, 1}, {Qt::AltModifier, 5}, {Qt::ControlModifier, 10},
       {Qt::ShiftModifier, 20}});
  }

  auto make_drop_down_box(const OrderFieldInfoTip::Model& model,
      const QString& value) {
    auto list = std::make_shared<ArrayListModel<QString>>();
    auto current = std::make_shared<LocalValueModel<optional<int>>>();
    for(auto i = 0; i < std::ssize(model.m_tag.m_arguments); ++i) {
      auto argument_value =
        QString::fromStdString(model.m_tag.m_arguments[i].m_value);
      list->push(argument_value);
      if(argument_value == value) {
        current->set(i);
      }
    }
    return new DropDownBox(std::move(list), std::move(current),
      ListView::default_view_builder);
  }
}

OrderFieldAdditionalTag::OrderFieldAdditionalTag(Nexus::Tag tag,
  OrderFieldInfoTip::Model model)
  : AdditionalTag(model.m_tag.m_name.c_str(), tag.GetKey()),
    m_model(std::move(model)),
    m_value(std::make_shared<LocalValueModel<optional<Nexus::Tag::Type>>>(
      tag.GetValue())) {}

const OrderFieldInfoTip::Model& OrderFieldAdditionalTag::get_model() const {
  return m_model;
}

const std::shared_ptr<AdditionalTagValueModel>&
    OrderFieldAdditionalTag::make_value() const {
  return m_value;
}

QWidget* OrderFieldAdditionalTag::make_view(
    const std::shared_ptr<AdditionalTagValueModel>& value) const {
  if(!value->get()) {
    return nullptr;
  }
  if(get_key() == Nexus::Tag::INT_INDEX) {
    return new IntegerBox(
      std::make_shared<LocalOptionalIntegerModel>(get<int>(*value->get())));
  } else if(get_key() == Nexus::Tag::DOUBLE_INDEX) {
    return new DecimalBox(std::make_shared<LocalOptionalDecimalModel>(
      Decimal(get<double>(*value->get()))));
  } else if(get_key() == Nexus::Tag::QUANTITY_INDEX) {
    return new QuantityBox(std::make_shared<LocalOptionalQuantityModel>(
      get<Quantity>(*value->get())), make_quantity_modifiers());
  } else if(get_key() == Nexus::Tag::MONEY_INDEX) {
    return new MoneyBox(
      std::make_shared<LocalOptionalMoneyModel>(get<Money>(*value->get())));
  } else if(get_key() == Nexus::Tag::CHAR_INDEX) {
    return make_drop_down_box(m_model, QString(get<char>(*value->get())));
  } else if(get_key() == Nexus::Tag::STRING_INDEX) {
    if(m_model.m_tag.m_arguments.empty()) {
      return new TextBox(std::make_shared<LocalTextModel>(
        QString::fromStdString(get<std::string>(*value->get()))));
    }
    return make_drop_down_box(m_model,
      QString::fromStdString(get<std::string>(*value->get())));
  } else if(get_key() == Nexus::Tag::DATE_INDEX) {
    return new DateBox(get<date>(*value->get()));
  } else if(get_key() == Nexus::Tag::TIME_INDEX) {
    return make_time_box(get<time_duration>(*value->get()));
  }
  return nullptr;
}

