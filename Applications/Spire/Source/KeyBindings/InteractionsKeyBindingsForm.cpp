#include "Spire/KeyBindings/InteractionsKeyBindingsForm.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/OptionalScalarValueModelDecorator.hpp"
#include "Spire/Spire/ProxyScalarValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TextAreaBox.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
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

  auto to_index(Qt::KeyboardModifier modifier) {
    if(modifier == Qt::NoModifier) {
      return 0;
    } else if(modifier == Qt::ShiftModifier) {
      return 1;
    } else if(modifier == Qt::ControlModifier) {
      return 2;
    } else if(modifier == Qt::AltModifier) {
      return 3;
    }
    throw std::out_of_range("Invalid keyboard modifier.");
  }

  auto to_modifier(int index) {
    if(index == 0) {
      return Qt::NoModifier;
    } else if(index == 1) {
      return Qt::ShiftModifier;
    } else if(index == 2) {
      return Qt::ControlModifier;
    } else if(index == 3) {
      return Qt::AltModifier;
    }
    throw std::out_of_range("Invalid keyboard modifier.");
  }

  auto make_region_header() {
    auto label = make_label("");
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

  auto make_description() {
    auto label = make_text_area_label("");
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
  DecimalBoxAdaptor<T>* make_scalar_box(
      std::shared_ptr<ScalarValueModel<T>> model) {
    return new DecimalBoxAdaptor<T>(
      std::make_shared<OptionalScalarValueModelDecorator<T>>(std::move(model)));
  }

  template<>
  DecimalBoxAdaptor<Quantity>* make_scalar_box(
      std::shared_ptr<ScalarValueModel<Quantity>> model) {
    return new QuantityBox(
      std::make_shared<OptionalScalarValueModelDecorator<Quantity>>(
        std::move(model)), QHash<Qt::KeyboardModifier, Quantity>(
          {{Qt::NoModifier, 1}, {Qt::AltModifier, 5}, {Qt::ControlModifier, 10},
          {Qt::ShiftModifier, 20}}));
  }

  template<>
  DecimalBoxAdaptor<Money>* make_scalar_box(
      std::shared_ptr<ScalarValueModel<Money>> model) {
    return new MoneyBox(
      std::make_shared<OptionalScalarValueModelDecorator<Money>>(
        std::move(model)));
  }

  template<typename T>
  auto make_slot(QString name, std::shared_ptr<T> model) {
    auto label = make_label(std::move(name));
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto box = make_scalar_box(std::move(model));
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
      const std::array<T, InteractionsKeyBindingsModel::MODIFIER_COUNT>&
        increments) {
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
    layout->addWidget(
      make_slot(to_text(Qt::NoModifier), increments[to_index(Qt::NoModifier)]));
    layout->addWidget(make_slot(
      to_text(Qt::ShiftModifier), increments[to_index(Qt::ShiftModifier)]));
    layout->addWidget(make_slot(
      to_text(Qt::ControlModifier), increments[to_index(Qt::ControlModifier)]));
    layout->addWidget(make_slot(
      to_text(Qt::AltModifier), increments[to_index(Qt::AltModifier)]));
    return widget;
  }
}

InteractionsKeyBindingsForm::InteractionsKeyBindingsForm(
    std::shared_ptr<KeyBindingsModel> key_bindings,
    std::shared_ptr<RegionModel> region, QWidget* parent)
    : QWidget(parent),
      m_key_bindings(std::move(key_bindings)),
      m_region(std::move(region)) {
  auto body = new QWidget();
  body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto layout = make_vbox_layout(body);
  layout->setSpacing(scale_height(18));
  m_header = make_region_header();
  layout->addWidget(m_header);
  layout->addSpacing(scale_height(-18));
  m_description = make_description();
  auto interactions =
    m_key_bindings->get_interactions_key_bindings(m_region->get());
  m_default_quantity =
    make_proxy_scalar_value_model(interactions->get_default_quantity());
  for(auto i = 0; i != InteractionsKeyBindingsModel::MODIFIER_COUNT; ++i) {
    m_quantity_increments[i] = make_proxy_scalar_value_model(
      interactions->get_quantity_increment(::to_modifier(i)));
    m_price_increments[i] = make_proxy_scalar_value_model(
      interactions->get_price_increment(::to_modifier(i)));
  }
  m_is_cancel_on_fill =
    make_proxy_value_model(interactions->is_cancel_on_fill());
  m_region_connection = m_region->connect_update_signal(
    std::bind_front(&InteractionsKeyBindingsForm::on_region, this));
  on_region(m_region->get());
  layout->addWidget(m_description);
  layout->addWidget(make_slot(tr("Default Quantity"), m_default_quantity));
  layout->addWidget(
    make_field_set(tr("Quantity Increments"), m_quantity_increments));
  layout->addWidget(make_field_set(tr("Price Increments"), m_price_increments));
  auto cancel_on_fill = new CheckBox(m_is_cancel_on_fill);
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

void InteractionsKeyBindingsForm::on_region(const Region& region) {
  auto description = [&] {
    if(region.IsGlobal()) {
      return QString("Customize the default interactions for all regions to "
        "suit your trading style.");
    }
    return QString("Customize interactions on %1 to suit your trading style.").
      arg(Spire::to_text(region));
  }();
  auto interactions = m_key_bindings->get_interactions_key_bindings(region);
  m_default_quantity->set_source(interactions->get_default_quantity());
  for(auto i = 0; i != InteractionsKeyBindingsModel::MODIFIER_COUNT; ++i) {
    m_quantity_increments[i]->set_source(
      interactions->get_quantity_increment(::to_modifier(i)));
    m_price_increments[i]->set_source(
      interactions->get_price_increment(::to_modifier(i)));
  }
  m_is_cancel_on_fill->set_source(interactions->is_cancel_on_fill());
  m_description->get_current()->set(description);
  m_header->get_current()->set(to_text(region));
}
