#ifndef SPIRE_SPLIT_VIEW_HPP
#define SPIRE_SPLIT_VIEW_HPP
#include <boost/signals2/connection.hpp>
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays two QWidget's within a common region separated by a horizontal or
   * vertical divider that can be used to split the common space between them.
   */
  class SplitView : public QWidget {
    public:

      /** Selects the divider. */
      using Divider = Styles::StateSelector<void, struct DividerTag>;

      /** Selects the primary component. */
      using Primary = Styles::StateSelector<void, struct PrimaryTag>;

      /** Selects the secondary component. */
      using Secondary = Styles::StateSelector<void, struct SecondaryTag>;

      /**
       * Constructs a SplitView.
       * @param primary The component displayed at the top when the orientation
       *        is vertical, or the component displayed to the left when the
       *        orientation is horizontal.
       * @param secondary The component displayed at the bottom when the
       *                  orientation is vertical, or the component displayed to
       *                  the left when the orientation is horizontal.
       */
      SplitView(
        QWidget& primary, QWidget& secondary, QWidget* parent = nullptr);

    private:
      QWidget* m_primary;
      QWidget* m_secondary;
      Box* m_divider;
      Qt::Orientation m_orientation;
      boost::signals2::scoped_connection m_style_connection;

      void on_style();
  };
}

#endif
