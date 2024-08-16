#ifndef SPIRE_DASHBOARDROW_HPP
#define SPIRE_DASHBOARDROW_HPP
#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/signals2/signal.hpp>
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCell.hpp"

namespace Spire {

  /*! \class DashboardRow
      \brief Represents a row of DashboardCells on a dashboard.
   */
  class DashboardRow : private boost::noncopyable {
    public:

      //! Signals a cell was added to this row.
      /*!
        \param cell The cell that was added.
      */
      using CellAddedSignal = boost::signals2::signal<void (
        const DashboardCell& cell)>;

      //! Constructs a DashboardRow.
      /*!
        \param index The row's index.
      */
      DashboardRow(std::unique_ptr<DashboardCell> index);

      //! Returns the number of cells in this row.
      int GetSize() const;

      //! Returns the cell representing the index.
      const DashboardCell& GetIndex() const;

      //! Returns the cell at a specified column.
      /*!
        \param column The column of the cell to retrieve where a value of 0
               represents the index cell.
        \return The DashboardCell at the specified <i>column</i>.
      */
      const DashboardCell& GetCell(int column) const;

      //! Adds a DashboardCell.
      /*!
        \param cell The DashboardCell to add.
      */
      void Add(std::unique_ptr<DashboardCell> cell);

      //! Removes a DashboardCell.
      /*!
        \param column The column to remove.
      */
      void Remove(int column);

      //! Connects a slot to the CellAddedSignal.
      /*!
        \param slot The slot to connect to the CellAddedSignal.
        \return A connection to the CellAddedSignal.
      */
      boost::signals2::connection ConnectCellAddedSignal(
        const CellAddedSignal::slot_function_type& slot) const;

    private:
      std::vector<std::unique_ptr<DashboardCell>> m_cells;
      mutable CellAddedSignal m_cellAddedSignal;
  };
}

#endif
