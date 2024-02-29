#include "Spire/KeyBindings/InteractionsKeyBindingsForm.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/OptionalScalarValueModelDecorator.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TextAreaBox.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto& to_text(Qt::KeyboardModifier modifier) {
    if(modifier == Qt::NoModifier) {
      static const auto value = QObject::tr("Default");
      return value;
    } else if(modifier == Qt::ShiftModifier) {
      static const auto value = QObject::tr("Shift");
      return value;
    } else if(modifier == Qt::ControlModifier) {
      static const auto value = QObject::tr("Ctrl");
      return value;
    } else if(modifier == Qt::AltModifier) {
      static const auto value = QObject::tr("Alt");
      return value;
    } else {
      static const auto value = QObject::tr("None");
      return value;
    }
  }

  auto make_region_header(const Region& region) {
    auto label = make_label(Spire::to_text(region));
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    update_style(*label, [] (auto& style) {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Medium);
      font.setPixelSize(scale_width(14));
      style.get(Any()).
        set(Font(font)).
        set(PaddingBottom(scale_height(8)));
    });
    return label;
  }

  auto make_description(const Region& region) {
    auto description = [&] {
      if(region.IsGlobal()) {
        return QString("Customize the default interactions for all regions to "
          "suit your trading style.");
      }
      return
        QString("Customize interactions on %1 to suit your trading style.").
          arg(Spire::to_text(region));
    }();
    auto label = make_text_area_label(description);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    update_style(*label, [] (auto& style) {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Normal);
      font.setPixelSize(scale_width(10));
      style.get(Any()).set(text_style(font, QColor(0x808080)));
    });
    return label;
  }

  template<typename T>
  DecimalBoxAdaptor<T>* make_decimal_box(
      std::shared_ptr<ScalarValueModel<T>> model) {
    return new DecimalBoxAdaptor<T>(
      std::make_shared<OptionalScalarValueModelDecorator<T>>(std::move(model)));
  }

  template<>
  DecimalBoxAdaptor<Quantity>* make_decimal_box(
      std::shared_ptr<ScalarValueModel<Quantity>> model) {
    return new QuantityBox(
      std::make_shared<OptionalScalarValueModelDecorator<Quantity>>(
        std::move(model)), QHash<Qt::KeyboardModifier, Quantity>(
          {{Qt::NoModifier, 1}, {Qt::AltModifier, 5}, {Qt::ControlModifier, 10},
          {Qt::ShiftModifier, 20}}));
  }

  template<>
  DecimalBoxAdaptor<Money>* make_decimal_box(
      std::shared_ptr<ScalarValueModel<Money>> model) {
    return new MoneyBox(
      std::make_shared<OptionalScalarValueModelDecorator<Money>>(
        std::move(model)));
  }

  template<typename T>
  std::shared_ptr<ScalarValueModel<T>> get_increment(
      const std::shared_ptr<InteractionsKeyBindingsModel>& bindings,
      Qt::KeyboardModifier modifier) {
    return std::shared_ptr<ScalarValueModel<T>>();
  }

  template<>
  std::shared_ptr<ScalarValueModel<Quantity>> get_increment(
      const std::shared_ptr<InteractionsKeyBindingsModel>& bindings,
      Qt::KeyboardModifier modifier) {
    return bindings->get_quantity_increment(modifier);
  }

  template<>
  std::shared_ptr<ScalarValueModel<Money>> get_increment(
      const std::shared_ptr<InteractionsKeyBindingsModel>& bindings,
      Qt::KeyboardModifier modifier) {
    return bindings->get_price_increment(modifier);
  }

  template<typename T>
  auto make_slot(const QString& name,
      std::shared_ptr<ScalarValueModel<T>> model) {
    auto label = make_label(name);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto box = make_decimal_box(std::move(model));
    box->setFixedWidth(scale_width(120));
    auto widget = new QWidget();
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto layout = make_hbox_layout(widget);
    layout->addWidget(label);
    layout->addWidget(box);
    return widget;
  }
  
  template<typename T>
  auto make_field_set(const QString& name,
      const std::shared_ptr<InteractionsKeyBindingsModel>& bindings) {
    using Type = T;
    auto header = make_label(name);
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    update_style(*header, [] (auto& style) {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Medium);
      font.setPixelSize(scale_width(12));
      style.get(Any()).
        set(Font(font)).
        set(PaddingBottom(scale_height(8)));
    });
    auto widget = new QWidget();
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto layout = make_vbox_layout(widget);
    layout->setSpacing(scale_height(4));
    layout->addWidget(header);
    layout->addSpacing(scale_height(-4));
    layout->addWidget(make_slot<Type>(to_text(Qt::NoModifier),
      get_increment<Type>(bindings, Qt::NoModifier)));
    layout->addWidget(make_slot<Type>(to_text(Qt::ShiftModifier),
      get_increment<Type>(bindings, Qt::ShiftModifier)));
    layout->addWidget(make_slot<Type>(to_text(Qt::ControlModifier),
      get_increment<Type>(bindings, Qt::ControlModifier)));
    layout->addWidget(make_slot<Type>(to_text(Qt::AltModifier),
      get_increment<Type>(bindings, Qt::AltModifier)));
    return widget;
  }
}

InteractionsKeyBindingsForm::InteractionsKeyBindingsForm(Nexus::Region region,
    std::shared_ptr<InteractionsKeyBindingsModel> bindings, QWidget* parent)
    : QWidget(parent),
      m_bindings(std::move(bindings)) {
  auto body = new QWidget();
  body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto layout = make_vbox_layout(body);
  layout->setSpacing(scale_height(18));
  layout->addWidget(make_region_header(region));
  layout->addSpacing(scale_height(-18));
  layout->addWidget(make_description(region));
  layout->addWidget(make_slot<Quantity>(tr("Default Quantity"),
    m_bindings->get_default_quantity()));
  layout->addWidget(
    make_field_set<Quantity>(tr("Quantity Increments"), m_bindings));
  layout->addWidget(
    make_field_set<Money>(tr("Price Increments"), m_bindings));
  auto cancel_on_fill = new CheckBox(m_bindings->is_cancel_on_fill());
  cancel_on_fill->set_label(tr("Cancel on Fill"));
  layout->addWidget(cancel_on_fill, 0, Qt::AlignLeft);
  auto box = new Box(body);
  box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*box, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(horizontal_padding(scale_width(8))).
      set(PaddingTop(scale_height(18))).
      set(PaddingBottom(scale_height(8)));
  });
  enclose(*this, *box);
}

const std::shared_ptr<InteractionsKeyBindingsModel>&
    InteractionsKeyBindingsForm::get_bindings() const {
  return m_bindings;
}
