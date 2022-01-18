#ifndef SPIRE_BOX_PAINTER_HPP
#define SPIRE_BOX_PAINTER_HPP
#include <QColor>
#include "Spire/Styles/Styles.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

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
