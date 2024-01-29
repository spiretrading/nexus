#ifndef SPIRE_BOX_GEOMETRY_HPP
#define SPIRE_BOX_GEOMETRY_HPP
#include <QRect>
#include "Spire/Styles/Styles.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Stores the areas that make up a basic box component. */
  class BoxGeometry {
    public:

      /** Constructs a BoxGeometry defining a box with zero width and height. */
      BoxGeometry();

      /**
       * Constructs a BoxGeometry with an initial size.
       * @param size The size of the geometry.
       */
      explicit BoxGeometry(QSize size);

      /**
       * Returns the overall geometry which by definition is equal to the border
       * area.
       */
      const QRect& get_geometry() const;

      /** Returns the rectangle defining the edge of the border area. */
      const QRect& get_border_area() const;

      /** Returns the size of the top border. */
      int get_border_top() const;

      /** Sets the size of the top border. */
      void set_border_top(int size);

      /** Returns the size of the right border. */
      int get_border_right() const;

      /** Sets the size of the right border. */
      void set_border_right(int size);

      /** Returns the size of the bottom border. */
      int get_border_bottom() const;

      /** Sets the size of the bottom border. */
      void set_border_bottom(int size);

      /** Returns the size of the left border. */
      int get_border_left() const;

      /** Sets the size of the left border. */
      void set_border_left(int size);

      /** Returns the rectangle defining the edge of the padding area. */
      const QRect& get_padding_area() const;

      /** Returns the top padding. */
      int get_padding_top() const;

      /** Sets the top padding. */
      void set_padding_top(int size);

      /** Returns the right padding. */
      int get_padding_right() const;

      /** Sets the right padding. */
      void set_padding_right(int size);

      /** Returns the bottom padding. */
      int get_padding_bottom() const;

      /** Sets the bottom padding. */
      void set_padding_bottom(int size);

      /** Returns the left padding. */
      int get_padding_left() const;

      /** Sets the left padding. */
      void set_padding_left(int size);

      /** Returns the rectangle defining the edge of the content area. */
      const QRect& get_content_area() const;

      /** Sets the size of the entire box. */
      void set_size(QSize size);

    private:
      QRect m_border_area;
      QRect m_padding_area;
      QRect m_content_area;
  };

  /**
   * Returns the size of the styling (padding/borders) surrounding the content.
   */
  QSize get_styling_size(const BoxGeometry& geometry);

  /**
   * Returns the margins surrounding the content.
   */
  QMargins get_content_margins(const BoxGeometry& geometry);

  /**
   * Applies a style Property to a BoxGeometry.
   * @param property The property to apply.
   * @param geometry The BoxGeometry to apply the <i>property</i> to.
   * @param stylist The Stylist used to evaluate the <i>property</i>.
   */
  void apply(const Styles::Property& property, BoxGeometry& geometry,
    Styles::Stylist& stylist);
}

#endif
