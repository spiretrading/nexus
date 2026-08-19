#ifndef SPIRE_DASHBOARDCELLRENDERER_HPP
#define SPIRE_DASHBOARDCELLRENDERER_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/noncopyable.hpp>
#include <boost/signals2/signal.hpp>
#include <QFont>
#include <QPainter>
#include <QPen>
#include <QRect>
#include "Spire/Dashboard/Dashboard.hpp"

namespace Spire {

  /*! \class DashboardPainter
      rief Draws to a QPainter, tracking the pen and font applied to it so
             that redundant updates are skipped.
   */
  class DashboardPainter {
    public:

      //! Constructs a DashboardPainter.
      /*!
        \param painter The painter to draw to.
      */
      explicit DashboardPainter(QPainter& painter);

      //! Returns the painter being drawn to.
      QPainter& GetPainter();

      //! Sets the pen used to draw.
      /*!
        \param pen The pen to draw with.
      */
      void SetPen(const QPen& pen);

      //! Sets the font used to draw.
      /*!
        \param font The font to draw with.
      */
      void SetFont(const QFont& font);

    private:
      QPainter* m_painter;
      QPen m_pen;
      QFont m_font;
  };

  inline DashboardPainter::DashboardPainter(QPainter& painter)
    : m_painter(&painter),
      m_pen(painter.pen()),
      m_font(painter.font()) {}

  inline QPainter& DashboardPainter::GetPainter() {
    return *m_painter;
  }

  inline void DashboardPainter::SetPen(const QPen& pen) {
    if(m_pen == pen) {
      return;
    }
    m_pen = pen;
    m_painter->setPen(pen);
  }

  inline void DashboardPainter::SetFont(const QFont& font) {
    if(m_font == font) {
      return;
    }
    m_font = font;
    m_painter->setFont(font);
  }

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
        \param painter The painter to draw with.
        \param region The region within the <i>painter</i> to draw to.
      */
      virtual void Draw(DashboardPainter& painter, const QRect& region) = 0;

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
