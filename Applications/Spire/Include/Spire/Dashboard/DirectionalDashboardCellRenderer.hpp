#ifndef SPIRE_DIRECTIONALDASHBOARDCELLRENDERER_HPP
#define SPIRE_DIRECTIONALDASHBOARDCELLRENDERER_HPP
#include <boost/optional/optional.hpp>
#include <QFlags>
#include <QPen>
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCell.hpp"
#include "Spire/Dashboard/DashboardCellRenderer.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class DirectionalDashboardCellRenderer
      \brief Implements a DashboardCellRenderer that displays colored text
             depending on whether a value is on an uptick or downtick.
   */
  class DirectionalDashboardCellRenderer : public DashboardCellRenderer {
    public:

      //! Constructs a DirectionalDashboardCellRenderer.
      /*!
        \param cell The cell to render.
        \param userProfile The user's profile.
      */
      DirectionalDashboardCellRenderer(Beam::Ref<const DashboardCell> cell,
        Beam::Ref<UserProfile> userProfile);

      //! Returns the text's font.
      const QFont& GetFont() const;

      //! Sets the text's font.
      /*!
        \param font The font to use.
      */
      void SetFont(const QFont& font);

      //! Returns the pen used to draw text.
      const QPen& GetPen() const;

      //! Returns the pen used to draw text.
      QPen& GetPen();

      //! Returns the text alignment.
      QFlags<Qt::AlignmentFlag> GetAlignment() const;

      //! Sets the text alignment.
      /*!
        \param alignment The text alignment to use.
      */
      void SetAlignment(QFlags<Qt::AlignmentFlag> alignment);

      virtual void Draw(QPaintDevice& device, const QRect& region);

      virtual boost::signals2::connection ConnectDrawSignal(
        const DrawSignal::slot_function_type& slot) const;

    private:
      UserProfile* m_userProfile;
      QFont m_font;
      QPen m_pen;
      QFlags<Qt::AlignmentFlag> m_alignment;
      std::unique_ptr<LegacyUI::CustomVariantItemDelegate> m_delegate;
      boost::optional<DashboardCell::Value> m_lastValue;
      int m_direction;
      boost::signals2::scoped_connection m_cellUpdateConnection;
      mutable DrawSignal m_drawSignal;

      void OnCellUpdateSignal(const DashboardCell::Value& value);
  };
}

#endif
