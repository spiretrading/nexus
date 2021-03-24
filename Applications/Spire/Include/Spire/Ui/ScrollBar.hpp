#ifndef SPIRE_SCROLL_BAR_HPP
#define SPIRE_SCROLL_BAR_HPP
#include <QScrollBar>
#include "Spire/Styles/StyledWidget.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Sets the width. */
  using Width = BasicProperty<int, struct WidthTag>;

  /** Sets the heigth. */
  using Height = BasicProperty<int, struct HeightTag>;

  /** Sets the minimum width. */
  using MinWidth = BasicProperty<int, struct MinWidthTag>;

  /** Sets the minimum heigth. */
  using MinHeight = BasicProperty<int, struct MinHeightTag>;

  /** Selects the thumb. */
  using ScrollBarThumb = PseudoElement<void, struct ScrollBarThumbTag>;
}

  //! Displays a vertical or horizontal scroll bar.
  class ScrollBar : public Styles::StyledWidget {
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

      //! Connects a slot to the PositionSignal.
      boost::signals2::connection connect_position_signal(
        const PositionSignal::slot_type& slot) const;

    protected:
      void selector_updated() override;

    private:
      mutable PositionSignal m_position_signal;
      QScrollBar* m_scroll_bar;
  };
}

#endif
