#ifndef SPIRE_BLOTTER_POSITIONS_VIEW_HPP
#define SPIRE_BLOTTER_POSITIONS_VIEW_HPP
#include <vector>
#include <QKeySequence>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays the blotter's positions. */
  class BlotterPositionsView : public QWidget {
    public:

      /** The key sequence to flatten selected position. */
      const static QKeySequence FLATTEN_SELECTED_KEY_SEQUENCE;

      /** The key sequence to flatten all positions. */
      const static QKeySequence FLATTEN_ALL_KEY_SEQUENCE;

      /**
       * Signals to flatten a list of positions identified by the position's
       * security.
       * @param securities The list of securities used as indexes into the
       *        positions to flatten.
       */
      using FlattenSignal =
        Signal<void (const std::vector<Nexus::Security>& securities)>;

      /**
       * Signals to reverse a list of positions identified by the position's
       * security.
       * @param securities The list of securities used as indexes into the
       *        positions to reverse.
       */
      using ReverseSignal =
        Signal<void (const std::vector<Nexus::Security>& securities)>;

      /**
       * Constructs a BlotterPositionsView.
       * @param portfolio The set of portfolio whose positions are displayed.
       * @param parent The parent widget.
       */
      explicit BlotterPositionsView(std::shared_ptr<PortfolioModel> portfolio,
        QWidget* parent = nullptr);

      /** Returns the portfolio whose positions are displayed. */
      const std::shared_ptr<PortfolioModel>& get_portfolio() const;

      /** Connects a slot to the FlattenSignal. */
      boost::signals2::connection connect_flatten_signal(
        const FlattenSignal::slot_type& slot) const;

      /** Connects a slot to the ReverseSignal. */
      boost::signals2::connection connect_reverse_signal(
        const ReverseSignal::slot_type& slot) const;

    private:
      enum class Command {
        FLATTEN,
        FLATTEN_ALL
      };
      mutable FlattenSignal m_flatten_signal;
      mutable ReverseSignal m_reverse_signal;
      std::shared_ptr<PortfolioModel> m_portfolio;
  };
}

#endif
