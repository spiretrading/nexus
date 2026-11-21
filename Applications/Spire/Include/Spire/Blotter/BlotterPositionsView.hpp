#ifndef SPIRE_BLOTTER_POSITIONS_VIEW_HPP
#define SPIRE_BLOTTER_POSITIONS_VIEW_HPP
#include "Spire/Blotter/BlotterPositionsCommandBar.hpp"
#include "Spire/Blotter/PortfolioModel.hpp"

namespace Spire {
  class TableView;

  /** Displays a blotter's positions. */
  class BlotterPositionsView : public QWidget {
    public:

      /** Signals a request to flatten a set of open positions. */
      using FlattenSignal =
        Signal<void (const std::vector<Nexus::Position>& positions)>;

      /**
       * Constructs a BlotterPositionsView.
       * @param portfolio The portfolio whose open positions are displayed.
       * @param bindings The user's position key bindings.
       * @param parent The parent widget.
       */
      BlotterPositionsView(std::shared_ptr<PortfolioModel> portfolio,
        std::shared_ptr<PositionKeyBindingsModel> bindings,
        QWidget* parent = nullptr);

      /** Returns the portfolio displayed. */
      const std::shared_ptr<PortfolioModel>& get_portfolio() const;

      /** Connects a slot to the FlattenSignal. */
      boost::signals2::connection connect_flatten_signal(
        const FlattenSignal::slot_type& slot) const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable FlattenSignal m_flatten_signal;
      std::shared_ptr<PortfolioModel> m_portfolio;
      std::shared_ptr<PortfolioModel> m_positions;
      std::shared_ptr<PositionKeyBindingsModel> m_bindings;
      std::shared_ptr<ValueModel<BlotterPositionsCommandBar::FlattenState>>
        m_can_flatten;
      TableView* m_table;
      Beam::ConnectionGroup m_connections;

      void on_update();
      void on_flatten();
      void on_flatten_all();
      void on_context_menu(const QPoint& position);
      void on_header_menu(const QPoint& position);
  };
}

#endif
