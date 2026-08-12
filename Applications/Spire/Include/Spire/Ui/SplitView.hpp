#ifndef SPIRE_SPLIT_VIEW_HPP
#define SPIRE_SPLIT_VIEW_HPP
#include <boost/signals2/connection.hpp>
#include "Spire/Styles/StateSelector.hpp"

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

      QSize minimumSizeHint() const override;

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      struct DividerBox;
      struct Panel;
      Panel* m_primary;
      Panel* m_secondary;
      Qt::Orientation m_orientation;
      DividerBox* m_divider;
      double m_primary_ratio;
      boost::signals2::scoped_connection m_style_connection;

      int get_content_size() const;
      int get_primary_size() const;
      int get_primary_min_expanded_size() const;
      int get_secondary_min_expanded_size() const;
      int get_secondary_reserved_size() const;
      int get_primary_max_size() const;
      int clamp_primary_size(int size) const;
      void resize_primary(int size);
      void set_primary_size(int size);
      void update_split();
      void on_drag(int offset);
      void on_reset();
      void on_style();
  };
}

#endif
