#include "Spire/AccountViewer/ComplianceValueWidget.hpp"
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include "Spire/InputWidgets/CurrencyInputWidget.hpp"
#include "Spire/InputWidgets/MoneySpinBox.hpp"
#include "Spire/InputWidgets/ScopeInputWidget.hpp"
#include "Spire/InputWidgets/TickerInputWidget.hpp"
#include "Spire/InputWidgets/TimeInputWidget.hpp"
#include "Spire/InputWidgets/ValueListInputWidget.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  struct ComplianceValueConverter {
    template<typename T>
    ComplianceValue operator ()(const T& value) const {
      return value;
    }
  };

  struct ValueConverter {
    template<typename T>
    ValueListInputDialog::Value operator ()(const T& value) const {
      return {};
    }

    ValueListInputDialog::Value operator ()(const Ticker& value) const {
      return value;
    }
  };

  struct ComplianceWidgetVisitor {
    bool m_isReadOnly;
    UserProfile* m_userProfile;
    QWidget* m_parent;
    Qt::WindowFlags m_flags;

    ComplianceWidgetVisitor(bool isReadOnly, Ref<UserProfile> userProfile,
        QWidget* parent, Qt::WindowFlags flags)
        : m_isReadOnly{isReadOnly},
          m_userProfile{userProfile.get()},
          m_parent{parent},
          m_flags(flags) {}

    template<typename T>
    QWidget* operator ()(const T& value) const {
      return new QWidget{m_parent, m_flags};
    }

    QWidget* operator ()(bool enabled) const {
      auto widget = new QCheckBox{m_parent};
      if(m_isReadOnly) {
        widget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        widget->setFocusPolicy(Qt::NoFocus);
      }
      return widget;
    }

    QWidget* operator ()(Quantity value) const {
      auto widget = new QSpinBox{m_parent};
      widget->setReadOnly(m_isReadOnly);
      return widget;
    }

    QWidget* operator ()(double value) const {
      auto widget = new QDoubleSpinBox{m_parent};
      widget->setReadOnly(m_isReadOnly);
      return widget;
    }

    QWidget* operator ()(const std::string& value) const {
      auto widget = new QLineEdit{QString::fromStdString(value), m_parent};
      widget->setReadOnly(m_isReadOnly);
      return widget;
    }

    QWidget* operator ()(time_duration value) const {
      auto widget = new TimeInputWidget{Ref(*m_userProfile), m_parent,
        m_flags};
      widget->SetReadOnly(m_isReadOnly);
      return widget;
    }

    QWidget* operator ()(CurrencyId currency) const {
      auto widget = new CurrencyInputWidget{
        Ref(*m_userProfile), m_parent, m_flags};
      widget->SetReadOnly(m_isReadOnly);
      return widget;
    }

    QWidget* operator ()(Money value) const {
      auto widget = new MoneySpinBox{Ref(*m_userProfile), m_parent, m_flags};
      widget->SetDecimals(2);
      widget->SetReadOnly(m_isReadOnly);
      return widget;
    }

    QWidget* operator ()(const Scope& value) const {
      auto widget = new ScopeInputWidget{Ref(*m_userProfile), m_parent};
      widget->SetReadOnly(m_isReadOnly);
      return widget;
    }

    QWidget* operator ()(const Ticker& value) const {
      auto widget = new TickerInputWidget{Ref(*m_userProfile), m_parent,
        m_flags};
      widget->SetReadOnly(m_isReadOnly);
      return widget;
    }

    QWidget* operator ()(const vector<ComplianceValue>& value) const {
      auto widget = new ValueListInputWidget{m_parent, m_flags};
      widget->Initialize(Ref(*m_userProfile), value.front().type());
      widget->SetReadOnly(m_isReadOnly);
      return widget;
    }
  };

  struct SetValueVisitor {
    QWidget* m_widget;

    SetValueVisitor(QWidget* widget)
        : m_widget(widget) {}

    template<typename T>
    void operator ()(const T& value) const {}

    void operator ()(bool isChecked) const {
      static_cast<QCheckBox*>(m_widget)->setChecked(isChecked);
    }

    void operator ()(Quantity value) const {
      static_cast<QSpinBox*>(m_widget)->setValue(static_cast<int>(value));
    }

    void operator ()(double value) const {
      static_cast<QDoubleSpinBox*>(m_widget)->setValue(value);
    }

    void operator ()(const std::string& value) const {
      static_cast<QLineEdit*>(m_widget)->setText(QString::fromStdString(value));
    }

    void operator ()(const time_duration& value) const {
      static_cast<TimeInputWidget*>(m_widget)->SetTime(value);
    }

    void operator ()(CurrencyId currency) const {
      static_cast<CurrencyInputWidget*>(m_widget)->SetCurrency(currency);
    }

    void operator ()(Money value) const {
      static_cast<MoneySpinBox*>(m_widget)->SetValue(value);
    }

    void operator ()(const Scope& value) const {
      static_cast<ScopeInputWidget*>(m_widget)->SetScope(value);
    }

    void operator ()(const Ticker& value) const {
      static_cast<TickerInputWidget*>(m_widget)->SetTicker(value);
    }

    void operator ()(const vector<ComplianceValue>& value) const {
      vector<ValueListInputDialog::Value> values;
      std::transform(value.begin(), value.end(), back_inserter(values),
        [] (const ComplianceValue& value) {
          return apply_visitor(ValueConverter{}, value);
        });
      static_cast<ValueListInputWidget*>(m_widget)->SetValues(values);
    }
  };
}

QWidget* Spire::MakeComplianceValueWidget(ComplianceValue value,
    bool isReadOnly, Ref<UserProfile> userProfile, QWidget* parent,
    Qt::WindowFlags flags) {
  return apply_visitor(ComplianceWidgetVisitor{isReadOnly, Ref(userProfile),
    parent, flags}, value);
}

ComplianceValue Spire::GetComplianceValue(const QWidget& widget) {
  if(auto boolWidget = dynamic_cast<const QCheckBox*>(&widget)) {
    return boolWidget->isChecked();
  } else if(auto quantityWidget = dynamic_cast<const QSpinBox*>(&widget)) {
    return static_cast<Quantity>(quantityWidget->value());
  } else if(auto doubleWidget = dynamic_cast<const QDoubleSpinBox*>(&widget)) {
    return doubleWidget->value();
  } else if(auto textWidget = dynamic_cast<const QLineEdit*>(&widget)) {
    return textWidget->text().toStdString();
  } else if(auto durationWidget =
      dynamic_cast<const TimeInputWidget*>(&widget)) {
    return durationWidget->GetTime();
  } else if(auto currencyWidget =
      dynamic_cast<const CurrencyInputWidget*>(&widget)) {
    return currencyWidget->GetCurrency();
  } else if(auto moneyWidget = dynamic_cast<const MoneySpinBox*>(&widget)) {
    return moneyWidget->GetValue();
  } else if(auto scopeWidget = dynamic_cast<const ScopeInputWidget*>(&widget)) {
    return scopeWidget->GetScope();
  } else if(auto tickerWidget =
      dynamic_cast<const TickerInputWidget*>(&widget)) {
    return tickerWidget->GetTicker();
  } else if(auto listWidget =
      dynamic_cast<const ValueListInputWidget*>(&widget)) {
    auto& listValue = listWidget->GetValues();
    vector<ComplianceValue> values;
    std::transform(listValue.begin(), listValue.end(), back_inserter(values),
      [] (const ValueListInputDialog::Value& value) {
        return apply_visitor(ComplianceValueConverter{}, value);
      });
    return values;
  }
  return ComplianceValue{};
}

void Spire::SetComplianceValue(QWidget& widget, const ComplianceValue& value) {
  return apply_visitor(SetValueVisitor(&widget), value);
}
