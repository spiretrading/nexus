#ifndef SPIRE_TIME_AND_SALES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_HPP
#include <Beam/Pointers/Ref.hpp>
#include "Spire/LegacyUI/PersistentWindow.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Playback/ReplaySession.hpp"
#include "Spire/Spire/PropertyHubMember.hpp"
#include "Spire/Spire/ProxyValueModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesPropertiesWindowFactory.hpp"
#include "Spire/Ui/TickerBox.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {
  class TableView;
  class TickerView;
  class TransitionView;

  /** Display the time and sales window for a ticker. */
  class TimeAndSalesWindow : public Window, public LegacyUI::PersistentWindow {
    public:

      /**
       * The type of function used to build a TimeAndSalesModel based on
       * the ticker and the ReplaySession the window plays back.
       * @param ticker The ticker that the window is representing.
       * @param session The ReplaySession the window plays back, null if the
       *        window displays time and sales in real time.
       * @return A TimeAndSalesModel.
       */
      using ModelBuilder = std::function<
        std::shared_ptr<TimeAndSalesModel>(const Nexus::Ticker& ticker,
          const std::shared_ptr<ReplaySession>& session)>;

      /**
       * Constructs a TimeAndSalesWindow.
       * @param user_profile The user's profile.
       * @param tickers The set of tickers to use.
       * @param factory The factory used to create a
       *        TimeAndSalesPropertiesWindow.
       * @param model_builder The ModelBuilder to use.
       * @param parent The parent widget.
       */
      TimeAndSalesWindow(Beam::Ref<UserProfile> user_profile,
        std::shared_ptr<TickerInfoQueryModel> tickers,
        std::shared_ptr<TimeAndSalesPropertiesWindowFactory> factory,
        ModelBuilder model_builder, QWidget* parent = nullptr);

      /**
       * Constructs a TimeAndSalesWindow.
       * @param user_profile The user's profile.
       * @param tickers The set of tickers to use.
       * @param factory The factory used to create a
       *        TimeAndSalesPropertiesWindow.
       * @param model_builder The ModelBuilder to use.
       * @param hub The PropertyHub storing the properties shared with the
       *        components this window is linked to.
       * @param parent The parent widget.
       */
      TimeAndSalesWindow(Beam::Ref<UserProfile> user_profile,
        std::shared_ptr<TickerInfoQueryModel> tickers,
        std::shared_ptr<TimeAndSalesPropertiesWindowFactory> factory,
        ModelBuilder model_builder, std::shared_ptr<PropertyHub> hub,
        QWidget* parent = nullptr);

      /** Returns the currently displayed ticker. */
      const std::shared_ptr<TickerModel>& get_current() const;

      std::unique_ptr<LegacyUI::WindowSettings>
        GetWindowSettings() const override;

    private:
      friend class LegacyTimeAndSalesWindowSettings;
      friend class TimeAndSalesWindowSettings;
      UserProfile* m_user_profile;
      std::shared_ptr<TimeAndSalesPropertiesWindowFactory> m_factory;
      ModelBuilder m_model_builder;
      std::shared_ptr<ProxyValueModel<TimeAndSalesProperties>>
        m_properties_proxy;
      PropertyHubMember m_member;
      std::shared_ptr<ReplaySessionModel> m_session;
      Nexus::Ticker m_ticker;
      std::shared_ptr<TimeAndSalesTableModel> m_table_model;
      TableView* m_table_view;
      bool m_is_update_pending;
      TransitionView* m_transition_view;
      TickerView* m_ticker_view;
      boost::signals2::scoped_connection m_session_connection;

      void update_model();
      void schedule_update();
      void on_context_menu(QWidget* parent, const QPoint& pos);
      void on_export_menu();
      void on_properties_menu();
      void on_end_loading();
      void on_current(const Nexus::Ticker& ticker);
      void on_session(const std::shared_ptr<ReplaySession>& session);
  };
}

#endif
