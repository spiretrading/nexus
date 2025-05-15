#ifndef SPIRE_TIME_AND_SALES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_HPP
#include <boost/signals2/connection.hpp>
#include "Nexus/Definitions/Market.hpp"
#include "Spire/LegacyUI/PersistentWindow.hpp"
#include "Spire/LegacyUI/SecurityContext.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/TimeAndSales/TimeAndSalesPropertiesWindowFactory.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/Window.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Display the time and sales window for a security. */
  class TimeAndSalesWindow : public Window, public LegacyUI::PersistentWindow,
      public LegacyUI::SecurityContext {
    public:

      /**
       * The type of function used to build a TimeAndSalesModel based on
       * the security.
       * @param security The security that the window is representing.
       * @return A TimeAndSalesModel.
       */
      using ModelBuilder = std::function<
        std::shared_ptr<TimeAndSalesModel>(const Nexus::Security& security)>;

      /**
       * Constructs a TimeAndSalesWindow.
       * @param securities The set of securities to use.
       * @param markets The database of markets.
       * @param factory The factory used to create a
       *        TimeAndSalesPropertiesWindow.
       * @param model_builder The ModelBuilder to use.
       * @param parent The parent widget.
       */
      TimeAndSalesWindow(std::shared_ptr<SecurityInfoQueryModel> securities,
        Nexus::MarketDatabase markets,
        std::shared_ptr<TimeAndSalesPropertiesWindowFactory> factory,
        ModelBuilder model_builder, QWidget* parent = nullptr);

      /**
       * Constructs a TimeAndSalesWindow.
       * @param securities The set of securities to use.
       * @param markets The database of markets.
       * @param factory The factory used to create a
       *        TimeAndSalesPropertiesWindow.
       * @param model_builder The ModelBuilder to use.
       * @param identifier The SecurityContext identifier.
       * @param parent The parent widget.
       */
      TimeAndSalesWindow(std::shared_ptr<SecurityInfoQueryModel> securities,
        Nexus::MarketDatabase markets,
        std::shared_ptr<TimeAndSalesPropertiesWindowFactory> factory,
        ModelBuilder model_builder, std::string identifier,
        QWidget* parent = nullptr);

      /** Returns the currently displayed security. */
      const std::shared_ptr<SecurityModel>& get_current() const;

      std::unique_ptr<LegacyUI::WindowSettings>
        GetWindowSettings() const override;

    protected:
      void showEvent(QShowEvent* event) override;
      void HandleLink(SecurityContext& context) override;
      void HandleUnlink() override;

    private:
      friend class LegacyTimeAndSalesWindowSettings;
      friend class TimeAndSalesWindowSettings;
      Nexus::MarketDatabase m_markets;
      std::shared_ptr<TimeAndSalesPropertiesWindowFactory> m_factory;
      ModelBuilder m_model_builder;
      std::shared_ptr<TimeAndSalesTableModel> m_table_model;
      TableView* m_table_view;
      TransitionView* m_transition_view;
      std::string m_link_identifier;
      boost::signals2::scoped_connection m_link_connection;
      SecurityView* m_security_view;

      void on_context_menu(QWidget* parent, const QPoint& pos);
      void on_export_menu();
      void on_properties_menu();
      void on_begin_loading();
      void on_end_loading();
      void on_current(const Nexus::Security& security);
  };
}

#endif
