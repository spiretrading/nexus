#ifndef SPIRE_SCROLL_BAR_HPP
#define SPIRE_SCROLL_BAR_HPP
#include "Spire/Styles/StyledWidget.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Implements a scrollbar. */
  class ScrollBar : public Styles::StyledWidget {
    public:

      /** Signals a change in the scrollbar's position. */
      using PositionSignal = Signal<void (int position)>;

      /** Stores the range being scrolled over. */
      struct Range {

        /** The start of the scrollable range. */
        int m_start;

        /** The end of the scrollable range. */
        int m_end;
      };

      /**
       * Constructs a ScrollBar.
       * @param orientation The orientation of the scroll bar.
       * @param parent The parent widget.
       */
      explicit ScrollBar(Qt::Orientation orientation,
        QWidget* parent = nullptr);

      /** Returns the orientation of the scrollbar. */
      Qt::Orientation get_orientation();

      /** Returns the scrollable range. */
      Range get_range() const;

      /** Sets the scrollable range. */
      void set_range(const Range& range);

      /** Sets the scrollable range. */
      void set_range(int start, int end);

      /** Returns the size of the smallest scrollable unit. */
      int get_line_size() const;

      /** Sets the size smallest scrollable unit. */
      void set_line_size(int size);

      /** Returns the size of a page. */
      int get_page_size() const;

      /** Sets the size of a page. */
      void set_page_size(int size);

      /** Returns the current position. */
      int get_position() const;

      /** Scrolls to a position. */
      void set_position(int position);

      /** Connects a slot to the PositionSignal. */
      boost::signals2::connection connect_position_signal(
        const PositionSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    protected:
      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      mutable PositionSignal m_position_signal;
      Qt::Orientation m_orientation;
      Range m_range;
      int m_line_size;
      int m_page_size;
      int m_position;
      Box* m_thumb;
      Box* m_track;
      bool m_is_dragging;
      int m_drag_position;
      int m_thumb_position;

      void update_thumb(int position);
  };
}

#endif
