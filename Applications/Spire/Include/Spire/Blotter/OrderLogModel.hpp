#ifndef SPIRE_ORDER_LOG_MODEL_HPP
#define SPIRE_ORDER_LOG_MODEL_HPP
#include <optional>
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/signal.hpp>
#include <QAbstractItemModel>
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/OrderLogProperties.hpp"

namespace Spire {

  /** Keeps track of the state of all Orders. */
  class OrderLogModel : public QAbstractTableModel {
    public:

      /** Enumerates the model's columns. */
      enum Columns {

        /** The time that the Order was submitted. */
        TIME_COLUMN,

        /** The order id. */
        ID_COLUMN,

        /** The Order status. */
        STATUS_COLUMN,

        /** The Security acted upon. */
        SECURITY_COLUMN,

        /** The Currency used. */
        CURRENCY_COLUMN,

        /** The Order's type. */
        ORDER_TYPE_COLUMN,

        /** The Order's Side. */
        SIDE_COLUMN,

        /** The Order's destination. */
        DESTINATION_COLUMN,

        /** The quantity ordered. */
        QUANTITY_COLUMN,

        /** The Order's price. */
        PRICE_COLUMN,

        /** The Order's TimeInForce. */
        TIME_IN_FORCE_COLUMN,
      };

      /** The number of columns. */
      static const auto COLUMN_COUNT = 11;

      /** Stores a single entry in this model. */
      struct OrderEntry {

        /** The Order represented. */
        std::shared_ptr<Nexus::Order> m_order;

        /** The current Order's status. */
        Nexus::OrderStatus m_status;

        /**
         * Constructs an OrderEntry.
         * @param order The Order to represent.
         */
        OrderEntry(std::shared_ptr<Nexus::Order> order);
      };

      /**
       * Signals an Order was added to this model.
       * @param entry The OrderEntry that was added.
       */
      using OrderAddedSignal =
        boost::signals2::signal<void (const OrderEntry& entry)>;

      /**
       * Signals an Order was removed from this model.
       * @param entry The OrderEntry that was removed.
       */
      using OrderRemovedSignal =
        boost::signals2::signal<void (const OrderEntry& entry)>;

      /**
       * Constructs an empty OrderLogModel.
       * @param properties The properties used to display this model.
       */
      explicit OrderLogModel(const OrderLogProperties& properties);

      /** Returns the properties used to display this model. */
      const OrderLogProperties& GetProperties() const;

      /** Sets the properties used to display this model. */
      void SetProperties(const OrderLogProperties& properties);

      /**
       * Returns the OrderEntry at a specified index.
       * @param index The index of the OrderEntry to return.
       * @return The OrderEntry at the specified <i>index</i>.
       */
      const OrderEntry& GetEntry(const QModelIndex& index) const;

      /**
       * Sets the OrderExecutionPublisher whose Orders are to be logged.
       * @param orderExecutionPublisher The OrderExecutionPublisher whose Orders
       *        are to be logged.
       */
      void SetOrderExecutionPublisher(
        Beam::Ref<const Beam::Publisher<std::shared_ptr<Nexus::Order>>>
          orderExecutionPublisher);

      /**
       * Connects a slot to the OrderAddedSignal.
       * @param slot The slot to connect to the OrderAddedSignal.
       * @return A connection to the specified signal.
       */
      boost::signals2::connection ConnectOrderAddedSignal(
        const OrderAddedSignal::slot_function_type& slot) const;

      /**
       * Connects a slot to the OrderRemovedSignal.
       * @param slot The slot to connect to the OrderRemovedSignal.
       * @return A connection to the specified signal.
       */
      boost::signals2::connection ConnectOrderRemovedSignal(
        const OrderRemovedSignal::slot_function_type& slot) const;

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      QVariant headerData(
        int section, Qt::Orientation orientation, int role) const override;

    private:
      OrderLogProperties m_properties;
      const Beam::Publisher<std::shared_ptr<Nexus::Order>>*
        m_orderExecutionPublisher;
      std::vector<OrderEntry> m_entries;
      mutable OrderAddedSignal m_orderAddedSignal;
      mutable OrderRemovedSignal m_orderRemovedSignal;
      std::optional<EventHandler> m_eventHandler;

      void OnOrderExecuted(const std::shared_ptr<Nexus::Order>& order);
      void OnExecutionReport(
        std::size_t entryIndex, const Nexus::ExecutionReport& report);
  };
}

#endif
