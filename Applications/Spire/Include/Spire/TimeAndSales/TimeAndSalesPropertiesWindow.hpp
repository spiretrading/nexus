#ifndef SPIRE_TIME_AND_SALES_PROPERTIES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_PROPERTIES_WINDOW_HPP
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {
  class FontBox;

  /** Displays the properties for the time and sales. */
  class TimeAndSalesPropertiesWindow : public Window {
    public:

      /** Signals that the user submitted the current edits. */
      using SubmitSignal = Signal<void ()>;

      /** Signals that the user discarded the current edits. */
      using CancelSignal = Signal<void ()>;

      /**
       * Constructs a TimeAndSalesPropertiesWindow.
       * @param current The initial current properties.
       * @param parent The parent widget.
       */
      explicit TimeAndSalesPropertiesWindow(
        std::shared_ptr<TimeAndSalesPropertiesModel> current,
        QWidget* parent = nullptr);

      /** Returns the current properties. */
      const std::shared_ptr<TimeAndSalesPropertiesModel>&
        get_current() const;

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the CancelSignal. */
      boost::signals2::connection connect_cancel_signal(
        const CancelSignal::slot_type& slot) const;

    protected:
      void showEvent(QShowEvent* event) override;
      void closeEvent(QCloseEvent* event) override;

    private:
      struct PropertiesWindowModel;
      mutable SubmitSignal m_submit_signal;
      mutable CancelSignal m_cancel_signal;
      std::unique_ptr<PropertiesWindowModel> m_model;
      FontBox* m_font_box;
      bool m_is_first_show;
      bool m_is_submitted;

      void on_font(const QFont& font);
      void on_cancel();
      void on_done();
  };
}

#endif
