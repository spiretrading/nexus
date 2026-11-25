#ifndef SPIRE_BLOTTER_STATUS_BAR_HPP
#define SPIRE_BLOTTER_STATUS_BAR_HPP
#include <QBoxLayout>
#include <QWidget>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /** Displays a blotter's summary values and message status. */
  class BlotterStatusBar : public QWidget {
    public:

      /**
       * Constructs a BlotterStatusBar.
       * @param summary The portfolio totals in the reporting currency.
       * @param risk_parameters The account's risk limits.
       * @param parent The parent widget.
       */
      BlotterStatusBar(
        std::shared_ptr<ValueModel<Nexus::PortfolioUpdateEntry>> summary,
        std::shared_ptr<ValueModel<Nexus::RiskParameters>> risk_parameters,
        QWidget* parent = nullptr);

      QSize minimumSizeHint() const override;

    private:
      QBoxLayout* m_compact_layout;
  };
}

#endif
