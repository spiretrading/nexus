#ifndef SPIRE_SCROLL_BAR_HPP
#define SPIRE_SCROLL_BAR_HPP
#include <QScrollBar>
#include "Spire/Styles/PseudoElement.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Selects the thumb. */
  using ScrollBarThumb = PseudoElement<void, struct ScrollBarThumbTag>;
}

  //! Displays a vertical or horizontal scroll bar.
  class ScrollBar : public QWidget {
    public:

      //! Signals a change in the scrollbar's position.
      using PositionSignal = Signal<void (int position)>;

      //! Stores the range being scrolled over.
      struct Range {

        //! The start of the scrollable range.
        int m_start;

        //! The end of the scrollable range.
        int m_end;
      };

      //! Constructs a ScrollBar.
      /*!
        \param orientation The orientation of the scroll bar.
        \param parent The parent widget.
      */
      explicit ScrollBar(Qt::Orientation orientation,
        QWidget* parent = nullptr);

      //! Returns the orientation of the scrollbar.
      Qt::Orientation get_orientation();

      //! Returns the scrollable range.
      Range get_range() const;

      //! Sets the scrollable range.
      void set_range(const Range& range);

      //! Sets the scrollable range.
      void set_range(int start, int end);

      //! Returns the size of the smallest scrollable unit.
      int get_line_size() const;

      //! Sets the size smallest scrollable unit.
      void set_line_size(int size);

      //! Returns the size of a page.
      int get_page_size() const;

      //! Sets the size of a page.
      void set_page_size(int size);

      //! Returns the current position.
      int get_position() const;

      //! Scrolls to a position.
      void set_position(int position);

      //! Returns the minimum size of the thumb.
      int get_thumb_min_size() const;

      //! Sets the minimum size of the thumb.
      void set_thumb_min_size(int size);

      //! Connects a slot to the PositionSignal.
      boost::signals2::connection connect_position_signal(
        const PositionSignal::slot_type& slot) const;

    private:
      mutable PositionSignal m_position_signal;
      QScrollBar* m_scroll_bar;
      int m_thumb_min_size;
  };
}

#endif
