#include "Spire/Spire/RealSpinBoxModel.hpp"

using namespace Spire;

namespace {
  auto to_real(const QVariant& value) {
    static auto locale = [] {
      auto locale = QLocale();
      locale.setNumberOptions(locale.numberOptions().setFlag(
        QLocale::OmitGroupSeparator, true));
      return locale;
    }();
    return CustomVariantItemDelegate().displayText(value,
      locale).toStdString().c_str();
  }
}

RealSpinBoxModel::Real Spire::to_real(std::int64_t value) {
  return value;
}

RealSpinBoxModel::Real Spire::to_real(double value) {
  return static_cast<long double>(value);
}

RealSpinBoxModel::Real Spire::to_real(Nexus::Quantity value) {
  return ::to_real(QVariant::fromValue<Nexus::Quantity>(value));
}

RealSpinBoxModel::Real Spire::to_real(Nexus::Money value) {
  return ::to_real(QVariant::fromValue<Nexus::Money>(value));
}
