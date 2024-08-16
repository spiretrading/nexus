#ifndef SPIRE_DASHBOARDSELECTIONMODEL_HPP
#define SPIRE_DASHBOARDSELECTIONMODEL_HPP
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/signals2/signal.hpp>
#include "Spire/Dashboard/Dashboard.hpp"

namespace Spire {

  /*! \class DashboardSelectionModel
      \brief Represents the selected items in a DashboardModel.
   */
  class DashboardSelectionModel : private boost::noncopyable {
    public:

      //! Signals a change in the selected rows.
      using SelectedRowsUpdatedSignal = boost::signals2::signal<void ()>;

      //! Signals a change in the active row.
      /*!
        \param activeRow The current active row.
      */
      using ActiveRowUpdatedSignal = boost::signals2::signal<
        void (boost::optional<int> activeRow)>;

      //! Constructs a DashboardSelectionModel.
      DashboardSelectionModel() = default;

      //! Returns the current active row.
      boost::optional<int> GetActiveRow() const;

      //! Returns the list of selected rows.
      const std::vector<int>& GetSelectedRows() const;

      //! Returns <code>true</code> iff a row is selected.
      /*!
        \param row The row to test.
        \return <code>true</code> iff the <i>row</i> is selected.
      */
      bool IsRowSelected(int row) const;

      //! Adds a row to the list of selected rows.
      /*!
        \param index The index of the row to add.
      */
      void AddRow(int index);

      //! Removes a row from the list of selected rows.
      /*!
        \param index The index of the row to remove.
      */
      void RemoveRow(int index);

      //! Sets a row as the active row.
      /*!
        \param index The index of the active row.
      */
      void SetActiveRow(const boost::optional<int>& index);

      //! Resets this model.
      void Reset();

      //! Connects a slot to the SelectedRowsUpdatedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the SelectedRowsUpdatedSignal.
      */
      boost::signals2::connection ConnectSelectedRowsUpdatedSignal(
        const SelectedRowsUpdatedSignal::slot_function_type& slot) const;

      //! Connects a slot to the ActiveRowUpdatedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the ActiveRowUpdatedSignal.
      */
      boost::signals2::connection ConnectActiveRowUpdatedSignal(
        const ActiveRowUpdatedSignal::slot_function_type& slot) const;

    private:
      std::vector<int> m_selectedRows;
      boost::optional<int> m_activeRow;
      mutable SelectedRowsUpdatedSignal m_selectedRowsUpdatedSignal;
      mutable ActiveRowUpdatedSignal m_activeRowUpdatedSignal;
  };
}

#endif
