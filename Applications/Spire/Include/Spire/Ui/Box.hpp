#ifndef SPIRE_BOX_HPP
#define SPIRE_BOX_HPP
#include <QPainterPath>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Sets the background color of an element. */
  using BackgroundColor = BasicProperty<QColor, struct BackgroundColorTag>;

  /** Sets the size of the top border. */
  using BorderTopSize = BasicProperty<int, struct BorderTopSizeTag>;

  /** Sets the size of the right border. */
  using BorderRightSize = BasicProperty<int, struct BorderRightSizeTag>;

  /** Sets the size of the bottom border. */
  using BorderBottomSize = BasicProperty<int, struct BorderBottomSizeTag>;

  /** Sets the size of the left border. */
  using BorderLeftSize = BasicProperty<int, struct BorderLeftSizeTag>;

  /** Sets the radius of the border's top left corner. */
  using BorderTopLeftRadius =
    BasicProperty<int, struct BorderTopLeftRadiusTag>;

  /** Sets the radius of the border's top right corner. */
  using BorderTopRightRadius =
    BasicProperty<int, struct BorderTopRightRadiusTag>;

  /** Sets the radius of the border's bottom right corner. */
  using BorderBottomRightRadius =
    BasicProperty<int, struct BorderBottomRightRadiusTag>;

  /** Sets the radius of the border's bottom left corner. */
  using BorderBottomLeftRadius =
    BasicProperty<int, struct BorderBottomLeftRadiusTag>;

  /** Composes all border radius properties. */
  using BorderRadius = CompositeProperty<BorderTopLeftRadius,
    BorderTopRightRadius, BorderBottomRightRadius, BorderBottomLeftRadius>;

  /** Composes all border size properties. */
  using BorderSize = CompositeProperty<BorderTopSize, BorderRightSize,
    BorderBottomSize, BorderLeftSize>;

  /** Sets the color of the top border. */
  using BorderTopColor = BasicProperty<QColor, struct BorderTopColorTag>;

  /** Sets the color of the right border. */
  using BorderRightColor = BasicProperty<QColor, struct BorderRightColorTag>;

  /** Sets the color of the bottom border. */
  using BorderBottomColor = BasicProperty<QColor, struct BorderBottomColorTag>;

  /** Sets the color of the left border. */
  using BorderLeftColor = BasicProperty<QColor, struct BorderLeftColorTag>;

  /** Composes all border color properties. */
  using BorderColor = CompositeProperty<BorderTopColor, BorderRightColor,
    BorderBottomColor, BorderLeftColor>;

  /** Composes all border colors and sizes. */
  using Border = CompositeProperty<BorderSize, BorderColor>;

  /** Sets the size of the top padding. */
  using PaddingTop = BasicProperty<int, struct PaddingTopTag>;

  /** Sets the size of the right padding. */
  using PaddingRight = BasicProperty<int, struct PaddingRightTag>;

  /** Sets the size of the bottom padding. */
  using PaddingBottom = BasicProperty<int, struct PaddingBottomTag>;

  /** Sets the size of the left padding. */
  using PaddingLeft = BasicProperty<int, struct PaddingLeftTag>;

  /** Sets the size of the left and right padding. */
  using HorizontalPadding = CompositeProperty<PaddingRight, PaddingLeft>;

  /** Sets the size of the top and bottom padding. */
  using VerticalPadding = CompositeProperty<PaddingTop, PaddingBottom>;

  /** Composes all paddings into a single property. */
  using Padding =
    CompositeProperty<PaddingTop, PaddingRight, PaddingBottom, PaddingLeft>;

  /** Sets the alignment of the body. */
  using BodyAlign = BasicProperty<Qt::AlignmentFlag, struct BodyAlignTag>;

  /** Selects a read-only widget. */
  using ReadOnly = StateSelector<void, struct ReadOnlyTag>;

  /**
   * Applies a single size to all four corners of a border.
   * @param size The size to apply to all borders.
   */
  BorderSize border_size(Expression<int> size);

  /**
   * Applies a single color to all four corners of a border.
   * @param color The color to apply to all borders.
   */
  BorderColor border_color(Expression<QColor> color);

  /**
   * Applies a radius to all four corners of a border.
   * @param radius The radius to apply to all borders.
   */
  BorderRadius border_radius(Expression<int> radius);

  /**
   * Applies a single color and size to all four corners of a border.
   * @param size The size to apply to all borders.
   * @param color The color to apply to all borders.
   */
  Border border(Expression<int> size, Expression<QColor> color);

  /**
   * Applies equal padding to the left and right of a widget.
   * @param size The size of the horizontal padding.
   */
  HorizontalPadding horizontal_padding(int size);

  /**
   * Applies equal padding to the top and bottom of a widget.
   * @param size The size of the vertical padding.
   */
  VerticalPadding vertical_padding(int size);

  /**
   * Applies equal padding to an entire widget.
   * @param size The size of the padding.
   */
  Padding padding(int size);
}

  /** Implements a containter displaying a component within a styled box. */
  class Box : public QWidget {
    public:

      /**
       * Constructs a Box.
       * @param body The component to display within the Box.
       * @param parent The parent widget.
       */
      explicit Box(QWidget* body, QWidget* parent = nullptr);

      /** Returns the body or a <code>nullptr</code>. */
      const QWidget* get_body() const;

      /** Returns the body or a <code>nullptr</code>. */
      QWidget* get_body();

      QSize sizeHint() const override;

    protected:
      bool event(QEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      template<typename T>
      struct SideProperty {
        T m_top;
        T m_right;
        T m_bottom;
        T m_left;
      };
      template <typename T>
      struct CornerProperty {
        T m_top_left;
        T m_top_right;
        T m_bottom_right;
        T m_bottom_left;
      };
      using Padding = SideProperty<int>;
      using BorderWidth = SideProperty<int>;
      using BorderColor = SideProperty<QColor>;
      using BorderRadius = CornerProperty<int>;
      using BorderGeometry = SideProperty<QPainterPath>;
      struct BoxStyle {
        BoxStyle();

        Padding m_padding;
        QColor m_background_color;
        BorderWidth m_border_width;
        BorderColor m_border_color;
        BorderRadius m_border_radius;
        SideProperty<QPainterPath> m_border_geometry;
      };
      QWidget* m_container;
      QWidget* m_body;
      QRect m_body_geometry;
      BoxStyle m_style;
      boost::signals2::scoped_connection m_style_connection;
      mutable boost::optional<QSize> m_size_hint;
      
      void paint_border_side(QPainter& painter,
        int width, const QColor& color, const QPainterPath& geometry) const;
      double radius_reduction_factor(const BorderRadius& radius) const;
      BorderRadius reduce_radius(BorderRadius radius) const;
      void update_border_geometry();
      void on_style();
  };

  /**
   * Returns a Box styled as an input component.
   * @param body The component to display within the Box.
   * @param parent The parent widget.
   */
  Box* make_input_box(QWidget* body, QWidget* parent = nullptr);
}

namespace Spire::Styles {
  template<>
  struct ComponentFinder<Box, Body> : BaseComponentFinder<Box, Body> {};
}

#endif
