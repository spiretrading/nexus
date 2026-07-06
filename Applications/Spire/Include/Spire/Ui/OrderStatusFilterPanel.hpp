#ifndef SPIRE_ORDER_STATUS_FILTER_PANEL_HPP
#define SPIRE_ORDER_STATUS_FILTER_PANEL_HPP
#include <QWidget>
#include "Spire/Ui/OrderStatusListBox.hpp"

namespace Spire {

  /** Displays a FilterPanel for filtering by OrderStatus using presets. */
  class OrderStatusFilterPanel : public QWidget {
    public:

      /**
       * Signals the submission of the filtered order statuses.
       * @param submission The list of OrderStatus values to be submitted.
       */
      using SubmitSignal =
        Signal<void (const std::shared_ptr<OrderStatusListModel>& submission)>;

      /**
       * Constructs an OrderStatusFilterPanel.
       * @param parent The parent widget.
       */
      explicit OrderStatusFilterPanel(QWidget* parent = nullptr);

      /**
       * Constructs an OrderStatusFilterPanel.
       * @param current The current list of selected order statuses.
       * @param parent The parent widget.
       */
      explicit OrderStatusFilterPanel(
        std::shared_ptr<OrderStatusListModel> current,
        QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<OrderStatusListModel>& get_current() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool focusNextPrevChild(bool next) override;

    private:
      enum class Preset {
        ALL,
        LIVE,
        TERMINAL,
        CUSTOM
      };
      struct PresetButtonContainer;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<OrderStatusListModel> m_current;
      OrderStatusListBox* m_list_box;
      PresetButtonContainer* m_button_container;
      QWidget* m_body;
      boost::signals2::scoped_connection m_connection;

      void submit();
      void on_preset(Preset preset);
      void on_reset();
      void on_operation(const OrderStatusListModel::Operation& operation);
  };
}

#endif
