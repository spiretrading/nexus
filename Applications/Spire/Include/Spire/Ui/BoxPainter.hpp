#ifndef SPIRE_BOX_PAINTER_HPP
#define SPIRE_BOX_PAINTER_HPP
#include <QColor>
#include "Spire/Styles/Selectors.hpp"

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

  /** Encapsulates the painting of Box styles. */
  class BoxPainter {
    public:

      /** Stores a single border's properties. */
      struct Border {

        /** The size of the border in pixels. */
        int m_size;

        /** The border's color. */
        QColor m_color;

        bool operator ==(const Border&) const = default;
      };

      /** Stores all borders and the Box's border radii. */
      struct Borders {

        /** The top border's properties. */
        Border m_top;

        /** The right border's properties. */
        Border m_right;

        /** The bottom border's properties. */
        Border m_bottom;

        /** The left border's properties. */
        Border m_left;

        /** The border radius on the top-right of the Box. */
        int m_top_right_radius;

        /** The border radius on the bottom-right of the Box. */
        int m_bottom_right_radius;

        /** The border radius on the bottom-left of the Box. */
        int m_bottom_left_radius;

        /** The border radius on the top-left of the Box. */
        int m_top_left_radius;
      };

      /**
       * Constructs a BoxPainter with no borders and a transparent background.
       */
      BoxPainter();

      /**
       * Constructs a BoxPainter.
       * @param borders The border properties.
       */
      BoxPainter(QColor background_color, Borders borders);

      /** Returns the background color. */
      QColor get_background_color() const;

      /** Sets the background color. */
      void set_background_color(QColor color);

      /** Returns the border properties. */
      const Borders& get_borders() const;

      /** Sets the border properties. */
      void set_borders(const Borders& borders);

      /** Sets the size of the top border. */
      void set_border_top_size(int size);

      /** Sets the color of the top border. */
      void set_border_top_color(QColor color);

      /** Sets the size of the right border. */
      void set_border_right_size(int size);

      /** Sets the color of the right border. */
      void set_border_right_color(QColor color);

      /** Sets the size of the bottom border. */
      void set_border_bottom_size(int size);

      /** Sets the color of the bottom border. */
      void set_border_bottom_color(QColor color);

      /** Sets the size of the left border. */
      void set_border_left_size(int size);

      /** Sets the color of the left border. */
      void set_border_left_color(QColor color);

      /** Sets the radius of the top-right border. */
      void set_top_right_radius(int radius);

      /** Sets the radius of the bottom-right border. */
      void set_bottom_right_radius(int radius);

      /** Sets the radius of the bottom-left border. */
      void set_bottom_left_radius(int radius);

      /** Sets the radius of the top-left border. */
      void set_top_left_radius(int radius);

      /** Paints a box on a given QPainter. */
      void paint(QPainter& painter) const;

    private:
      enum class Classification {
        NONE,
        REGULAR,
        OTHER
      };
      QColor m_background_color;
      Borders m_borders;
      Classification m_classification;

      Classification evaluate_classification() const;
  };

  /**
   * Applies a style Property to a BoxPainter.
   * @param property The property to apply.
   * @param painter The BoxPainter to apply the <i>property</i> to.
   * @param stylist The Stylist used to evaluate the <i>property</i>.
   */
  void apply(const Styles::Property& property, BoxPainter& painter,
    Styles::Stylist& stylist);
}

#endif
