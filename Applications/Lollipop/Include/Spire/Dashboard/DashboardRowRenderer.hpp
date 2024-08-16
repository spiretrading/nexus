#ifndef SPIRE_DASHBOARDROWRENDERER_HPP
#define SPIRE_DASHBOARDROWRENDERER_HPP
#include <functional>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <boost/noncopyable.hpp>
#include <boost/signals2/signal.hpp>
#include <QPaintDevice>
#include <QRect>
#include <QSize>
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellRenderer.hpp"

namespace Spire {

  /*! \class DashboardRowRenderer
      \brief Draws a DashboardRow.
   */
  class DashboardRowRenderer : private boost::noncopyable {
    public:

      //! Signals that this renderer has a draw operation available.
      using DrawSignal = boost::signals2::signal<void ()>;

      //! Function signature used to build a DashboardCellRenderer.
      /*!
        \param cell The DashboardCell to render.
      */
      using DashboardCellRendererBuilder = std::function<
        std::unique_ptr<DashboardCellRenderer> (const DashboardCell& cell)>;

      //! Constructs a DashboardRowRenderer.
      /*!
        \param row The DashboardRow to render.
        \param cellRendererBuilder Constructs the default DashboardCellRenderer
               used.
      */
      DashboardRowRenderer(Beam::Ref<const DashboardRow> row,
        const DashboardCellRendererBuilder& cellRendererBuilder);

      //! Returns the row being rendered.
      const DashboardRow& GetRow() const;

      //! Returns a DashboardCellRenderer.
      /*!
        \param index The index of the cell.
        \return The DashboardCellRenderer at the specified <i>index</i>.
      */
      const DashboardCellRenderer& GetCellRenderer(int index) const;

      //! Returns a DashboardCellRenderer.
      /*!
        \param index The index of the cell.
        \return The DashboardCellRenderer at the specified <i>index</i>.
      */
      DashboardCellRenderer& GetCellRenderer(int index);

      //! Returns the width of a cell.
      /*!
        \param index The cell's index/column.
        \return The width of the cell at the specified <i>index</i>.
      */
      int GetCellWidth(int index) const;

      //! Sets the width of a cell.
      /*!
        \param index The cell's index/column.
        \param width The cell's width.
      */
      void SetCellWidth(int index, int width);

      //! Moves a cell from one index to another.
      /*!
        \param sourceIndex The index of the cell to move.
        \param destinationIndex The index to move the cell to.
      */
      void Move(int sourceIndex, int destinationIndex);

      //! Performs a draw operation.
      /*!
        \param device The device to draw to.
        \param region The region within the <i>device</i> to draw to.
      */
      void Draw(QPaintDevice& device, const QRect& region);

      //! Connects a slot to the DrawSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the DrawSignal.
      */
      boost::signals2::connection ConnectDrawSignal(
        const DrawSignal::slot_function_type& slot) const;

    private:
      struct CellEntry {
        const DashboardCell* m_cell;
        int m_width;
        std::unique_ptr<DashboardCellRenderer> m_renderer;
        boost::signals2::scoped_connection m_drawConnection;
      };
      const DashboardRow* m_row;
      DashboardCellRendererBuilder m_cellRendererBuilder;
      int m_defaultCellWidth;
      std::vector<std::unique_ptr<CellEntry>> m_cells;
      boost::signals2::scoped_connection m_cellAddedConnection;
      mutable DrawSignal m_drawSignal;

      void OnCellAddedSignal(const DashboardCell& cell);
      void OnCellDrawSignal();
  };
}

#endif
