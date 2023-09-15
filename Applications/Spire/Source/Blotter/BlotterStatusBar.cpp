#include "Spire/Blotter/BlotterStatusBar.hpp"
#include "Spire/Blotter/BlotterStatusMessageIndicator.hpp"
#include "Spire/Blotter/BlotterStatusModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/ResponsiveLabel.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct StatusLabel : QWidget {
    ResponsiveLabel* m_label;
    MoneyBox* m_value;
    QSize m_size_hint;

    StatusLabel(std::vector<QString> labels,
        std::shared_ptr<MoneyModel> current) {
      m_label = new ResponsiveLabel(std::move(labels));
      m_label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
      update_style(*m_label, [] (auto& style) {
        style.get(Any()).set(FontSize(scale_height(10)));
      });
      m_label->get_current()->connect_update_signal(
        std::bind_front(&StatusLabel::on_update, this));
      auto min_label = std::numeric_limits<int>::max();
      auto metrics = QFontMetrics(m_label->font());
      for(auto i = 0; i != m_label->get_labels()->get_size(); ++i) {

        /**
         * TODO: Looks like ResponsiveLabel has an off by one error. We need
         *       compensate for it here temporarily.
         */
        min_label = std::min(min_label,
          metrics.horizontalAdvance(m_label->get_labels()->get(i)) + 1);
      }
      m_label->setMinimumWidth(min_label);
      auto layout = make_hbox_layout(this);
      layout->addWidget(m_label);
      m_value = make_money_label(current);
      m_value->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
      update_style(*m_value, [] (auto& style) {
        style.get(Any()).
          set(FontSize(scale_height(10))).
          set(PaddingLeft(scale_width(5))).
          set(TrailingZeros(2)).
          set(LeadingZeros(1));
      });
      m_value->get_text()->connect_update_signal(
        std::bind_front(&StatusLabel::on_update, this));
      layout->addWidget(m_value);
      layout->addSpacerItem(new QSpacerItem(1, 0, QSizePolicy::Expanding));
      update_size_hint();
    }

    QSize sizeHint() const override {
      return m_size_hint;
    }

    void update_size_hint() {
      auto metrics = QFontMetrics(m_value->font());
      auto width = metrics.horizontalAdvance(
        m_label->get_current()->get() + m_value->get_text()->get()) +
        scale_width(5);
      m_size_hint = QSize(width, layout()->sizeHint().height());
      setMaximumWidth(m_label->sizeHint().width() + scale_width(5) +
        metrics.horizontalAdvance(m_value->get_text()->get()));
      updateGeometry();
    }

    void on_update(const QString& label) {
      update_size_hint();
    }
  };
}

BlotterStatusBar::BlotterStatusBar(
    std::shared_ptr<BlotterStatusModel> status, QWidget* parent)
    : m_status(std::move(status)) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setFixedHeight(scale_height(26));
  auto body = new QWidget();
  body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto layout = make_hbox_layout(body);
  layout->setSpacing(scale_width(12));
  layout->setAlignment(Qt::AlignVCenter);
  layout->addWidget(new StatusLabel({tr("Total P/L"), tr("Tot P/L"), tr("P/L")},
    m_status->get_total_profit_and_loss()));
  layout->addWidget(new StatusLabel(
    {tr("Unrealized P/L"), tr("Unreal P/L"), tr("U P/L")},
    m_status->get_unrealized_profit_and_loss()));
  layout->addWidget(new StatusLabel(
    {tr("Realized P/L"), tr("Real P/L"), tr("R P/L")},
    m_status->get_realized_profit_and_loss()));
  layout->addWidget(new StatusLabel({tr("Fees")}, m_status->get_fees()));
  layout->addWidget(
    new StatusLabel({tr("Cost Basis"), tr("CB")}, m_status->get_cost_basis()));
  layout->addSpacerItem(new QSpacerItem(1, 0, QSizePolicy::Expanding));
  layout->addWidget(new StatusLabel({tr("Buying Power"), tr("Buy Pwr"),
    tr("BP")}, m_status->get_buying_power()));
  layout->addWidget(
    new StatusLabel({tr("Net Loss"), tr("NL")}, m_status->get_net_loss()));
  layout->addWidget(new BlotterStatusMessageIndicator());
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
