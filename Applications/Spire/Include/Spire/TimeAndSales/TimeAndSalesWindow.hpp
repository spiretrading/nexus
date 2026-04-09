#ifndef SPIRE_TIME_AND_SALES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_HPP
#include <boost/signals2/connection.hpp>
#include "Spire/LegacyUI/PersistentWindow.hpp"
#include "Spire/LegacyUI/TickerContext.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/TimeAndSales/TimeAndSalesPropertiesWindowFactory.hpp"
#include "Spire/Ui/TickerBox.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {
  class TableView;
  class TickerView;
  class TransitionView;

  /** Display the time and sales window for a ticker. */
  class TimeAndSalesWindow : public Window, public LegacyUI::PersistentWindow,
      public LegacyUI::TickerContext {
    public:

      /**
       * The type of function used to build a TimeAndSalesModel based on
       * the ticker.
       * @param ticker The ticker that the window is representing.
       * @return A TimeAndSalesModel.
       */
      using ModelBuilder = std::function<
        std::shared_ptr<TimeAndSalesModel>(const Nexus::Ticker& ticker)>;

      /**
       * Constructs a TimeAndSalesWindow.
       * @param tickers The set of tickers to use.
       * @param factory The factory used to create a
       *        TimeAndSalesPropertiesWindow.
       * @param model_builder The ModelBuilder to use.
       * @param parent The parent widget.
       */
      TimeAndSalesWindow(std::shared_ptr<TickerInfoQueryModel> tickers,
        std::shared_ptr<TimeAndSalesPropertiesWindowFactory> factory,
        ModelBuilder model_builder, QWidget* parent = nullptr);

      /**
       * Constructs a TimeAndSalesWindow.
       * @param tickers The set of tickers to use.
       * @param factory The factory used to create a
       *        TimeAndSalesPropertiesWindow.
       * @param model_builder The ModelBuilder to use.
       * @param identifier The TickerContext identifier.
       * @param parent The parent widget.
       */
      TimeAndSalesWindow(std::shared_ptr<TickerInfoQueryModel> tickers,
        std::shared_ptr<TimeAndSalesPropertiesWindowFactory> factory,
        ModelBuilder model_builder, std::string identifier,
        QWidget* parent = nullptr);

      /** Returns the currently displayed ticker. */
      const std::shared_ptr<TickerModel>& get_current() const;

      std::unique_ptr<LegacyUI::WindowSettings>
        GetWindowSettings() const override;

    protected:
      void showEvent(QShowEvent* event) override;
      void HandleLink(LegacyUI::TickerContext& context) override;
      void HandleUnlink() override;

    private:
      friend class LegacyTimeAndSalesWindowSettings;
      friend class TimeAndSalesWindowSettings;
      std::shared_ptr<TimeAndSalesPropertiesWindowFactory> m_factory;
      ModelBuilder m_model_builder;
      std::shared_ptr<TimeAndSalesTableModel> m_table_model;
      TableView* m_table_view;
      TransitionView* m_transition_view;
      std::string m_link_identifier;
      boost::signals2::scoped_connection m_link_connection;
      TickerView* m_ticker_view;

      void on_context_menu(QWidget* parent, const QPoint& pos);
      void on_export_menu();
      void on_properties_menu();
      void on_begin_loading();
      void on_end_loading();
      void on_current(const Nexus::Ticker& ticker);
  };
}

#endif
