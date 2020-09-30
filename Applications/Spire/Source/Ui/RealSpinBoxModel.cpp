#include "Spire/Spire/RealSpinBoxModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  auto to_real(const QVariant& value) {
    static auto locale = [] {
      auto locale = QLocale();
      locale.setNumberOptions(locale.numberOptions().setFlag(
        QLocale::OmitGroupSeparator, true));
      return locale;
    }();
    static auto item_delegate = CustomVariantItemDelegate();
    return RealSpinBoxModel::Real(
      CustomVariantItemDelegate().displayText(value,
        locale).toStdString().c_str());
  }
}

RealSpinBoxModel::Real Spire::to_real(std::int64_t value) {
  return value;
}

RealSpinBoxModel::Real Spire::to_real(double value) {
  return static_cast<long double>(value);
}

RealSpinBoxModel::Real Spire::to_real(Quantity value) {
  return ::to_real(QVariant::fromValue(value));
}

RealSpinBoxModel::Real Spire::to_real(Money value) {
  return ::to_real(QVariant::fromValue(value));
}
