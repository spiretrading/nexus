#ifndef SPIRE_SCROLL_BOX_HPP
#define SPIRE_SCROLL_BOX_HPP
#include <QWidget>
#include "Spire/Styles/ComponentSelector.hpp"
#include "Spire/Styles/StyleSheetMap.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * A widget that serves as a container for another widget, with scroll bars
   * displayed on overflow.
   */
  class ScrollBox : public QWidget {
    public:

      /** Specifies when a scroll bar is displayed. */
      enum class DisplayPolicy {

        /** Scroll bars are never displayed. */
        NEVER,

        /** Scroll bars are always displayed. */
        ALWAYS,

        /**
         * Scroll bars are displayed only when the body overflows the ScrollBox.
         */
        ON_OVERFLOW,

        /**
         * Scroll bars are displayed only when the user scrolls or hovers over
         * the ScrollBox.
         */
        ON_ENGAGE
      };

      /**
       * Constructs a ScrollBox.
       * @param body The widget to display.
       * @param parent The parent widget.
       */
      explicit ScrollBox(QWidget* body, QWidget* parent = nullptr);

      /** Returns the body. */
      const QWidget& get_body() const;

      /** Returns the body. */
      QWidget& get_body();

      /** Returns the horizontal display policy. */
      DisplayPolicy get_horizontal_display_policy() const;

      /** Sets the horizontal display policy. */
      void set_horizontal(DisplayPolicy policy);

      /** Returns the vertical display policy. */
      DisplayPolicy get_vertical_display_policy() const;

      /** Sets the vertical display policy. */
      void set_vertical(DisplayPolicy policy);

      /**
       * Sets both the horizontal and vertical display policies to the same
       * value.
       */
      void set(DisplayPolicy policy);

      /** Sets the horizontal and vertical display policies. */
      void set(DisplayPolicy horizontal_policy, DisplayPolicy vertical_policy);

      /** Returns the vertical ScrollBar. */
      ScrollBar& get_vertical_scroll_bar();

      /** Returns the horizontal ScrollBar. */
      ScrollBar& get_horizontal_scroll_bar();

      /**
       * Scrolls to a widget, ensuring that it's visible. If the widget is
       * already visible fully then no action is taken.
       * @param widget The widget to scroll to.
       */
      void scroll_to(const QWidget& widget);

      QSize sizeHint() const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;

    private:
      QWidget* m_body;
      DisplayPolicy m_horizontal_display_policy;
      DisplayPolicy m_vertical_display_policy;
      QWidget* m_viewport;
      ScrollableLayer* m_scrollable_layer;
      QMargins m_padding;
      QMargins m_borders;
      Styles::StyleSheetMap m_border_styles;
      Styles::StyleSheetMap m_padding_styles;
      boost::signals2::scoped_connection m_style_connection;

      void commit_border_styles();
      void commit_padding_styles();
      void on_style();
      void on_vertical_scroll(int position);
      void on_horizontal_scroll(int position);
      void update_layout();
      void update_ranges();
  };
}

namespace Spire::Styles {
  template<>
  struct ComponentFinder<ScrollBox, Body> :
    BaseComponentFinder<ScrollBox, Body> {};
}

#endif
