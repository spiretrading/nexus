#include "Spire/Blotter/BlotterStatusBar.hpp"
#include "Spire/Blotter/BlotterStatusModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/ResponsiveLabel.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  QLayout* make_status(std::vector<QString> labels,
      std::shared_ptr<MoneyModel> current) {
    auto layout = make_hbox_layout();
    auto label = new ResponsiveLabel(labels);
    label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    update_style(*label, [] (auto& style) {
      style.get(Any()).set(FontSize(scale_height(10)));
    });
    layout->addWidget(label);
    auto value = make_money_label(current);
    value->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    update_style(*value, [] (auto& style) {
      style.get(Any()).
        set(FontSize(scale_height(10))).
        set(PaddingLeft(scale_width(5))).
        set(TrailingZeros(2)).
        set(LeadingZeros(1));
    });
    layout->addWidget(value);
    layout->addSpacerItem(new QSpacerItem(1, 0, QSizePolicy::Expanding));
    return layout;
  }
}

BlotterStatusBar::BlotterStatusBar(
    std::shared_ptr<BlotterStatusModel> status, QWidget* parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setFixedHeight(scale_height(26));
  auto body = new QWidget();
  body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto layout = make_hbox_layout(body);
  layout->setSpacing(scale_width(12));
  layout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  layout->addLayout(make_status({tr("Total P/L"), tr("Tot P/L"), tr("P/L")},
    status->get_total_profit_and_loss()));
  layout->addLayout(make_status(
    {tr("Unrealized P/L"), tr("Unreal P/L"), tr("U P/L")},
    status->get_unrealized_profit_and_loss()));
  layout->addLayout(make_status(
    {tr("Realized P/L"), tr("Real P/L"), tr("R P/L")},
    status->get_realized_profit_and_loss()));
  layout->addLayout(make_status({tr("Fees")}, status->get_fees()));
  layout->addLayout(
    make_status({tr("Cost Basis"), tr("CB")}, status->get_cost_basis()));
  m_box = new Box(body);
  enclose(*this, *m_box);
  proxy_style(*this, *m_box);
  update_style(*this, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(BorderTopSize(scale_height(1))).
      set(BorderTopColor(QColor(0xC8C8C8))).
      set(PaddingLeft(scale_width(8)));
  });
}
