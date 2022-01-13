#ifndef SPIRE_BOX_PAINTER_HPP
#define SPIRE_BOX_PAINTER_HPP
#include <QColor>
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

      /** Paints a box on a given QPainter. */
      void paint(QPainter& painter) const;

    private:
      QColor m_background_color;
      Borders m_borders;
  };
}

#endif
