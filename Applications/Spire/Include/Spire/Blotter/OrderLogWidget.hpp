#ifndef SPIRE_ORDERLOGWIDGET_HPP
#define SPIRE_ORDERLOGWIDGET_HPP
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <boost/signals2/connection.hpp>
#include <QWidget>
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/OrderLogModel.hpp"
#include "Spire/LegacyUI/LegacyUI.hpp"
#include "Spire/Spire/Spire.hpp"

class Ui_OrderLogWidget;

namespace Spire {

  /** Displays a blotter's Order log. */
  class OrderLogWidget : public QWidget {
    public:

      /** Stores the user interface state of this widget. */
      struct UIState {

        /** The geometry of the order log table header. */
        QByteArray m_tableGeometry;

        /** The state of the order log table header. */
        QByteArray m_tableState;

        template<typename Shuttler>
        void Shuttle(Shuttler& shuttle, unsigned int version);
      };

      /**
       * Constructs an OrderLogWidget.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      explicit OrderLogWidget(
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      /** Returns the current UI state. */
      UIState GetUIState() const;

      /**
       * Sets the UI state.
       * @param state The UIState to set.
       */
      void SetUIState(const UIState& state);

      /**
       * Sets the BlotterModel this widget is representing.
       * @param userProfile The user's profile.
       * @param model The model to represent.
       */
      void SetModel(
        Beam::Ref<UserProfile> userProfile, Beam::Ref<BlotterModel> model);

    protected:
      bool eventFilter(QObject* object, QEvent* event) override;

    private:
      std::unique_ptr<Ui_OrderLogWidget> m_ui;
      UserProfile* m_userProfile;
      BlotterModel* m_model;
      std::unique_ptr<LegacyUI::CustomVariantSortFilterProxyModel> m_proxyModel;
      std::vector<OrderLogModel::OrderEntry> m_orderEntries;
      boost::signals2::scoped_connection m_orderAddedConnection;
      boost::signals2::scoped_connection m_orderRemovedConnection;
      EventHandler m_eventHandler;

      void OnOrderAdded(const OrderLogModel::OrderEntry& entry);
      void OnOrderRemoved(const OrderLogModel::OrderEntry& entry);
      void OnProxyOrderAdded(const QModelIndex& parent, int first, int last);
      void OnProxyOrderRemoved(const QModelIndex& parent, int first, int last);
      void OnCancel();
      void OnExecutionReport(const Nexus::OrderExecutionService::Order* order,
        const Nexus::OrderExecutionService::ExecutionReport& executionReport);
      void OnContextMenu(const QPoint& point);
  };

  template<typename Shuttler>
  void OrderLogWidget::UIState::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    shuttle.Shuttle("table_geometry", m_tableGeometry);
    shuttle.Shuttle("table_state", m_tableState);
  }
}

#endif
