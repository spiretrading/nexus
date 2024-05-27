#ifndef SPIRE_TIME_AND_SALES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_HPP
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesPropertiesWindowFactory.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Display the time and sales window for a security. */
  class TimeAndSalesWindow : public Window {
    public:

      /**
       * The type of function used to build a TimeAndSalesModel based on
       * the security.
       * @param security The security that the window is representing.
       * @return A TimeAndSalesModel.
       */
      using ModelBuilder = std::function<std::shared_ptr<TimeAndSalesModel>(
        const Nexus::Security& security)>;

      /**
       * Constructs a TimeAndSalesWindow.
       * @param securities The set of securities to use.
       * @param factory The factory used to create a
       *        TimeAndSalesPropertiesWindow.
       * @param model_builder The ModelBuilder to use.
       * @param parent The parent widget.
       */
      TimeAndSalesWindow(std::shared_ptr<ComboBox::QueryModel> securities,
        std::shared_ptr<TimeAndSalesPropertiesWindowFactory> factory,
        ModelBuilder model_builder, QWidget* parent = nullptr);

    private:
      TransitionView* m_transition_view;

      void on_current(const Nexus::Security& security);
  };
}

#endif
