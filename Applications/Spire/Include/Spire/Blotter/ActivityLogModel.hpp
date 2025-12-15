#ifndef SPIRE_ACTIVITY_LOG_MODEL_HPP
#define SPIRE_ACTIVITY_LOG_MODEL_HPP
#include <optional>
#include <Beam/Pointers/Ref.hpp>
#include <QAbstractItemModel>
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Blotter/Blotter.hpp"

namespace Spire {

  /** Keeps track of Order activity. */
  class ActivityLogModel : public QAbstractTableModel {
    public:

      /** Enumerates the model's columns. */
      enum Columns {

        /** The time of the action. */
        TIME_COLUMN,

        /** The order id. */
        ID_COLUMN,

        /** The side. */
        SIDE_COLUMN,

        /** The Security acted upon. */
        SECURITY_COLUMN,

        /** The Order status. */
        STATUS_COLUMN,

        /** The last quantity. */
        LAST_QUANTITY_COLUMN,

        /** The last price. */
        LAST_PRICE_COLUMN,

        /** The last market. */
        LAST_MARKET_COLUMN,

        /** The liquidity flag. */
        LIQUIDITY_FLAG_COLUMN,

        /** The execution fee paid/received. */
        EXECUTION_FEE_COLUMN,

        /** The processing fee paid/received. */
        PROCESSING_FEE_COLUMN,

        /** The commission paid/received. */
        COMMISSION_COLUMN,

        /** The message. */
        MESSAGE_COLUMN
      };

      /** The number of columns. */
      static const auto COLUMN_COUNT = 13;

      /** Constructs an empty ActivityLogModel. */
      ActivityLogModel();

      ~ActivityLogModel() override = default;

      /**
       * Sets the Publisher whose Orders are to be modeled.
       * @param publisher Publishes the Orders to model.
       */
      void SetOrderExecutionPublisher(
        Beam::Ref<const Beam::Publisher<std::shared_ptr<Nexus::Order>>>
          publisher);

      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& parent) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      QVariant headerData(
        int section, Qt::Orientation orientation, int role) const override;

    private:
      struct UpdateEntry {
        std::shared_ptr<Nexus::Order> m_order;
        Nexus::ExecutionReport m_report;
      };
      const Beam::Publisher<std::shared_ptr<Nexus::Order>>*
        m_orderExecutionPublisher;
      std::vector<UpdateEntry> m_entries;
      std::optional<EventHandler> m_eventHandler;

      void OnOrderExecuted(const std::shared_ptr<Nexus::Order>& order);
      void OnExecutionReport(const std::shared_ptr<Nexus::Order>& order,
        const Nexus::ExecutionReport& report);
  };
}

#endif
