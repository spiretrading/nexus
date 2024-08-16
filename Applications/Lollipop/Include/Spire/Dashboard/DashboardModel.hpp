#ifndef SPIRE_DASHBOARDMODEL_HPP
#define SPIRE_DASHBOARDMODEL_HPP
#include <memory>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCell.hpp"
#include "Spire/Dashboard/DashboardRow.hpp"

namespace Spire {

  /*! \class DashboardModel
      \brief Represents a table of DashboardCells.
   */
  class DashboardModel : private boost::noncopyable {
    public:

      //! Signals a row was added to this model.
      /*!
        \param row The row that was added.
      */
      using RowAddedSignal = boost::signals2::signal<void (
        const DashboardRow& row)>;

      //! Signals a row was removed from this model.
      /*!
        \param row The row that was removed.
      */
      using RowRemovedSignal = boost::signals2::signal<void (
        const DashboardRow& row)>;

      //! Constructs a DashboardModel.
      /*!
        \param columnNames The names of the columns.
      */
      DashboardModel(std::vector<std::string> columnNames);

      //! Returns the number of rows in the dashboard.
      int GetRowCount() const;

      //! Returns the number of columns in the dashboard.
      int GetColumnCount() const;

      //! Returns a column's name.
      /*!
        \param index The index of the column.
      */
      const std::string& GetColumnName(int index) const;

      //! Sets a column's name.
      /*!
        \param index The column's index.
        \param name The name of the column.
      */
      void SetColumnName(int index, std::string name);

      //! Returns the row at a specified index.
      /*!
        \param index The index of the row to retrieve.
        \return The DashboardRow at the specified <i>index</i>.
      */
      const DashboardRow& GetRow(int index) const;

      //! Adds a DashboardRow to the model.
      /*!
        \param row The DashboardRow to add.
      */
      void Add(std::unique_ptr<DashboardRow> row);

      //! Removes a DashboardRow from this model.
      /*!
        \param row The DashboardRow to remove.
      */
      void Remove(const DashboardRow& row);

      //! Connects a slot to the RowAddedSignal.
      /*!
        \param slot The slot to connect to the RowAddedSignal.
        \return A connection to the RowAddedSignal.
      */
      boost::signals2::connection ConnectRowAddedSignal(
        const RowAddedSignal::slot_function_type& slot) const;

      //! Connects a slot to the RowRemovedSignal.
      /*!
        \param slot The slot to connect to the RowRemovedSignal.
        \return A connection to the RowRemovedSignal.
      */
      boost::signals2::connection ConnectRowRemovedSignal(
        const RowRemovedSignal::slot_function_type& slot) const;

    private:
      std::vector<std::string> m_columnNames;
      std::vector<std::unique_ptr<DashboardRow>> m_rows;
      mutable RowAddedSignal m_rowAddedSignal;
      mutable RowRemovedSignal m_rowRemovedSignal;
  };
}

#endif
