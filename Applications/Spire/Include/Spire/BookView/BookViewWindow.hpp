#ifndef SPIRE_BOOK_VIEW_WINDOW_HPP
#define SPIRE_BOOK_VIEW_WINDOW_HPP
#include <boost/signals2/connection.hpp>
#include <QTimer>
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/BookView/BookViewPropertiesWindowFactory.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/Window.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Display the book view window for a security. */
  class BookViewWindow : public Window {
    public:

      /**
       * The type of function used to build a BookViewModel based on
       * the security.
       * @param security The security that the window is representing.
       * @return A TimeAndSalesModel.
       */
      using ModelBuilder = std::function<
        std::shared_ptr<BookViewModel>(const Nexus::Security& security)>;

      /**
       * Constructs a TimeAndSalesWindow.
       * @param securities The set of securities to use.
       * @param markets The database of markets.
       * @param factory The factory used to create a
       *        TimeAndSalesPropertiesWindow.
       * @param model_builder The ModelBuilder to use.
       * @param parent The parent widget.
       */
      BookViewWindow(std::shared_ptr<SecurityInfoQueryModel> securities,
        std::shared_ptr<KeyBindingsModel> key_bindings,
        std::shared_ptr<BookViewPropertiesWindowFactory> factory,
        Nexus::MarketDatabase markets,
        ModelBuilder model_builder, QWidget* parent = nullptr);

    protected:

    private:
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      std::shared_ptr<BookViewPropertiesWindowFactory> m_factory;
      ModelBuilder m_model_builder;
      Nexus::MarketDatabase m_markets;
      std::shared_ptr<QuantityModel> m_default_bid_quantity;
      std::shared_ptr<QuantityModel> m_default_ask_quantity;
      TransitionView* m_transition_view;
      SecurityView* m_security_view;
      boost::signals2::scoped_connection m_properties_connection;

      void on_current(const Nexus::Security& security);
      //void on_properties(const TimeAndSalesProperties& properties);
  };
}

#endif
