#ifndef SPIRE_SCROLL_BAR_HPP
#define SPIRE_SCROLL_BAR_HPP
#include <QTimer>
#include <QWidget>
#include "Spire/Styles/StateSelector.hpp"

namespace Spire {
  class Box;

namespace Styles {

  /** Selects the track in a ScrollBar. */
  using ScrollTrack = StateSelector<void, struct ScrollTrackSelectorTag>;

  /** Selects the thumb in a ScrollBar. */
  using ScrollThumb = StateSelector<void, struct ScrollThumbSelectorTag>;
}

  /** Implements a scroll bar. */
  class ScrollBar : public QWidget {
    public:

      /** Signals a change in the scrollbar's position. */
      using PositionSignal = Signal<void (int position)>;

      /** Stores the range being scrolled over. */
      struct Range {

        /** The start of the scrollable range. */
        int m_start;

        /** The end of the scrollable range. */
        int m_end;

        bool operator ==(const Range&) const = default;
      };

      /**
       * Constructs a ScrollBar.
       * @param orientation The orientation of the scroll bar.
       * @param parent The parent widget.
       */
      explicit ScrollBar(
        Qt::Orientation orientation, QWidget* parent = nullptr);

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
      void resizeEvent(QResizeEvent* event) override;

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
      int m_track_scroll_direction;
      QTimer m_track_scroll_timer;

      void update_thumb();
      void scroll_page();
  };

  /**
   * Scrolls up by a number of lines.
   * @param scroll_bar The ScrollBar to scroll by lines.
   * @param lines The number of lines to scroll by.
   */
  void scroll_line_up(ScrollBar& scroll_bar, int lines = 1);

  /**
   * Scrolls down by a number of lines.
   * @param scroll_bar The ScrollBar to scroll by lines.
   */
  void scroll_line_down(ScrollBar& scroll_bar, int lines = 1);

  /**
   * Scrolls up by a number of pages.
   * @param scroll_bar The ScrollBar to scroll by pages.
   */
  void scroll_page_up(ScrollBar& scroll_bar, int pages = 1);

  /**
   * Scrolls down by a number of pages.
   * @param scroll_bar The ScrollBar to scroll by pages.
   */
  void scroll_page_down(ScrollBar& scroll_bar, int pages = 1);

  /**
   * Scrolls to the start of a ScrollBar's range.
   * @param scroll_bar The ScrollBar to scroll to the start.
   */
  void scroll_to_start(ScrollBar& scroll_bar);

  /**
   * Scrolls to the end of a ScrollBar's range.
   * @param scroll_bar The ScrollBar to scroll to the end.
   */
  void scroll_to_end(ScrollBar& scroll_bar);
}

#endif
