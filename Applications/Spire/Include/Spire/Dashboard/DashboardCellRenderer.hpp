#ifndef SPIRE_DASHBOARDCELLRENDERER_HPP
#define SPIRE_DASHBOARDCELLRENDERER_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/noncopyable.hpp>
#include <boost/signals2/signal.hpp>
#include <QPaintDevice>
#include <QRect>
#include "Spire/Dashboard/Dashboard.hpp"

namespace Spire {

  /*! \class DashboardCellRenderer
      \brief Draws a single DashboardCell.
   */
  class DashboardCellRenderer : private boost::noncopyable {
    public:

      //! Signals that this renderer has a draw operation available.
      using DrawSignal = boost::signals2::signal<void ()>;

      virtual ~DashboardCellRenderer() = default;

      //! Returns the cell being rendered.
      const DashboardCell& GetCell() const;

      //! Performs a draw operation.
      /*!
        \param device The device to draw to.
        \param region The region within the <i>device</i> to draw to.
      */
      virtual void Draw(QPaintDevice& device, const QRect& region) = 0;

      //! Connects a slot to the DrawSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the DrawSignal.
      */
      virtual boost::signals2::connection ConnectDrawSignal(
        const DrawSignal::slot_function_type& slot) const = 0;

    protected:

      //! Constructs a DashboardCellRenderer.
      /*!
        \param cell The cell to render.
      */
      DashboardCellRenderer(Beam::Ref<const DashboardCell> cell);

    private:
      const DashboardCell* m_cell;
  };
}

#endif
