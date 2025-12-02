#include "Spire/KeyBindings/InteractionsKeyBindingsForm.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/OptionalScalarValueModelDecorator.hpp"
#include "Spire/Spire/ProxyScalarValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
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
  auto make_region_header(std::shared_ptr<RegionModel> region) {
    auto label = make_label(make_to_text_model(std::move(region)));
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

  auto make_description(std::shared_ptr<RegionModel> region) {
    auto description = make_to_text_model(std::move(region),
      [] (const auto& region) {
        if(region.is_global()) {
          return QObject::tr("Customize the default interactions for all "
            "regions to suit your trading style.");
        }
        return QObject::tr(
          "Customize interactions on %1 to suit your trading style.").
            arg(Spire::to_text(region));
      });
    auto label = make_text_area_label(std::move(description));
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
        std::move(model)));
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
      std::array<T, InteractionsKeyBindingsModel::MODIFIER_COUNT> increments) {
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
    for(auto i = 0; i != InteractionsKeyBindingsModel::MODIFIER_COUNT; ++i) {
      layout->addWidget(make_slot(to_text(to_modifier(i)), increments[i]));
    }
    return widget;
  }

  template<typename ProxyModel>
  struct InteractionsProxyModel : ProxyModel {
    scoped_connection m_connection;

    template<typename F>
    InteractionsProxyModel(std::shared_ptr<KeyBindingsModel> key_bindings,
        std::shared_ptr<RegionModel> region, F accessor)
        : ProxyModel(accessor(
            *key_bindings->get_interactions_key_bindings(region->get()))) {
      m_connection = region->connect_update_signal(
        [=, accessor = std::move(accessor)] (const auto& region) {
          this->set_source(
            accessor(*key_bindings->get_interactions_key_bindings(region)));
        });
    }
  };

  template<typename ProxyModel, typename F>
  auto make_interactions_proxy_model(
      std::shared_ptr<KeyBindingsModel> key_bindings,
      std::shared_ptr<RegionModel> region, F accessor) {
    return std::make_shared<InteractionsProxyModel<ProxyModel>>(
      std::move(key_bindings), std::move(region), std::move(accessor));
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
  layout->addWidget(make_region_header(m_region));
  layout->addSpacing(scale_height(-18));
  layout->addWidget(make_description(m_region));
  auto default_quantity = make_interactions_proxy_model<
    ProxyScalarValueModel<Quantity>>(
      m_key_bindings, m_region, [] (const auto& interactions) {
        return interactions.get_default_quantity();
      });
  layout->addWidget(
    make_slot(tr("Default Quantity"), std::move(default_quantity)));
  auto quantity_increments = std::array<std::shared_ptr<QuantityModel>,
    InteractionsKeyBindingsModel::MODIFIER_COUNT>();
  auto price_increments = std::array<std::shared_ptr<MoneyModel>,
    InteractionsKeyBindingsModel::MODIFIER_COUNT>();
  for(auto i = 0; i != InteractionsKeyBindingsModel::MODIFIER_COUNT; ++i) {
    quantity_increments[i] = make_interactions_proxy_model<
      ProxyScalarValueModel<Quantity>>(
        m_key_bindings, m_region, [=] (const auto& interactions) {
          return interactions.get_quantity_increment(to_modifier(i));
        });
    price_increments[i] = make_interactions_proxy_model<
      ProxyScalarValueModel<Money>>(
        m_key_bindings, m_region, [=] (const auto& interactions) {
          return interactions.get_price_increment(to_modifier(i));
        });
  }
  layout->addWidget(
    make_field_set(tr("Quantity Increments"), std::move(quantity_increments)));
  layout->addWidget(
    make_field_set(tr("Price Increments"), std::move(price_increments)));
  auto is_cancel_on_fill = make_interactions_proxy_model<ProxyValueModel<bool>>(
    m_key_bindings, m_region, [] (const auto& interactions) {
      return interactions.is_cancel_on_fill();
    });
  auto cancel_on_fill = new CheckBox(std::move(is_cancel_on_fill));
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
