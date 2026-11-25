#include "Spire/Blotter/BlotterStatusBar.hpp"
#include <vector>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ScalarValueModelDecorator.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/AdaptiveBox.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Tooltip.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_summary_label(const QString& name, const QFont& font) {
    auto label = make_label(name);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    update_style(*label, [&] (auto& style) {
      style.get(Any()).
        set(Font(font)).
        set(PaddingRight(scale_width(4)));
    });
    return label;
  }

  auto make_summary_layout(const std::vector<TextBox*>& labels,
      const std::vector<MoneyBox*>& values, QWidget& messages) {
    auto layout = make_hbox_layout();
    constexpr auto right_group_start = 4;
    for(auto i = 0; i < std::ssize(labels); ++i) {
      if(i == right_group_start) {
        layout->addSpacing(scale_width(12));
        layout->addStretch();
      } else if(i != 0) {
        layout->addSpacing(scale_width(12));
      }
      layout->addWidget(labels[i]);
      layout->addWidget(values[i]);
    }
    layout->addSpacing(scale_width(8));
    layout->addWidget(&messages);
    return layout;
  }
}

BlotterStatusBar::BlotterStatusBar(
    std::shared_ptr<ValueModel<PortfolioUpdateEntry>> summary,
    std::shared_ptr<ValueModel<RiskParameters>> risk_parameters,
    QWidget* parent)
    : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setFixedHeight(scale_height(22));
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(10));
  font.setWeight(QFont::Medium);
  auto labels = std::vector<TextBox*>();
  auto short_labels = std::vector<TextBox*>();
  auto values = std::vector<MoneyBox*>();
  auto models = std::vector<std::shared_ptr<ValueModel<Money>>>{
    make_transform_value_model(summary, [] (const auto& value) {
      return get_realized_profit_and_loss(value.m_currency_inventory) +
        value.m_currency_unrealized;
    }),
    make_transform_value_model(summary, [] (const auto& value) {
      return value.m_currency_unrealized;
    }),
    make_transform_value_model(summary, [] (const auto& value) {
      return get_realized_profit_and_loss(value.m_currency_inventory);
    }),
    make_transform_value_model(summary, [] (const auto& value) {
      return value.m_currency_inventory.m_fees;
    }),
    make_transform_value_model(summary, [] (const auto& value) {
      return value.m_currency_inventory.m_position.m_cost_basis;
    }),
    make_transform_value_model(risk_parameters, [] (const auto& value) {
      return value.m_buying_power;
    }),
    make_transform_value_model(risk_parameters, [] (const auto& value) {
      return value.m_net_loss;
    })};
  for(auto& [name, short_name] :
      std::initializer_list<std::pair<QString, QString>>{
        {tr("Total P/L"), tr("Total P/L")},
        {tr("Unrealized P/L"), tr("Unreal P/L")},
        {tr("Realized P/L"), tr("Real P/L")},
        {tr("Fees"), tr("Fees")},
        {tr("Cost Basis"), tr("CB")},
        {tr("Buying Power"), tr("BP")},
        {tr("Net Loss"), tr("NL")}}) {
    labels.push_back(make_summary_label(name, font));
    short_labels.push_back(make_summary_label(short_name, font));
    auto tooltip = [&] {
      if(name == tr("Net Loss")) {
        return tr("Net Loss Allowance");
      }
      return name;
    }();
    add_tooltip(tooltip, *labels.back());
    add_tooltip(tooltip, *short_labels.back());
    auto value = make_money_label(
      make_scalar_value_model_decorator(models[values.size()]));
    value->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    update_style(*value, [&] (auto& style) {
      style.get(Any()).set(Font(font));
    });
    values.push_back(value);
  }
  auto messages =
    make_icon_button(image_from_svg(":/Icons/blotter/bell.svg", scale(22, 22)),
      tr("No New Messages"));
  messages->setFixedWidth(scale_width(22));
  messages->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  auto content = new AdaptiveBox();
  content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  content->add(*make_summary_layout(labels, values, *messages));
  m_compact_layout = make_summary_layout(short_labels, values, *messages);
  content->add(*m_compact_layout);
  auto body = new Box(content);
  update_style(*body, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(BorderTopSize(scale_height(1))).
      set(BorderTopColor(QColor(0xC8C8C8))).
      set(PaddingLeft(scale_width(8)));
  });
  enclose(*this, *body);
}

QSize BlotterStatusBar::minimumSizeHint() const {
  return QSize(m_compact_layout->totalMinimumSize().width() + scale_width(8),
    minimumHeight());
}
