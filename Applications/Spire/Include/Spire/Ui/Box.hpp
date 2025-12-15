#ifndef SPIRE_BOX_HPP
#define SPIRE_BOX_HPP
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/BoxGeometry.hpp"
#include "Spire/Ui/BoxPainter.hpp"

namespace Spire {
namespace Styles {
  using Body = StateSelector<void, struct BodyTag>;
}

  /** Implements a containter displaying a component within a styled box. */
  class Box : public QWidget {
    public:

      /** Specifies how the Box should fit its body. */
      enum class Fit {

        /** The body may overflow or underflow the Box in both dimensions. */
        NONE = 0,

        /** The Box will exactly fit the width of the body (plus styling). */
        WIDTH = 1,

        /** The Box will exactly fit the height of the body (plus styling). */
        HEIGHT = 2,

        /** The Box will fit both the width and height with styling. */
        BOTH = 3
      };

      /**
       * Constructs a Box.
       * @param body The component to display within the Box.
       * @param parent The parent widget.
       */
      explicit Box(QWidget* body = nullptr, QWidget* parent = nullptr);

      /**
       * Constructs a Box.
       * @param body The component to display within the Box.
       * @param fit How the Box fits over the body.
       * @param parent The parent widget.
       */
      explicit Box(QWidget* body, Fit fit, QWidget* parent = nullptr);

      /** Returns the body or a <code>nullptr</code>. */
      const QWidget* get_body() const;

      /** Returns the body or a <code>nullptr</code>. */
      QWidget* get_body();

      QSize minimumSizeHint() const override;
      QSize sizeHint() const override;

    protected:
      bool event(QEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      QWidget* m_container;
      QWidget* m_body;
      Fit m_fit;
      BoxGeometry m_geometry;
      BoxPainter m_painter;
      boost::signals2::scoped_connection m_style_connection;
      mutable boost::optional<QSize> m_minimum_size_hint;
      mutable boost::optional<QSize> m_size_hint;

      void update_fit();
      void on_style();
  };

  /**
   * Returns a Box styled as an input component.
   * @param body The component to display within the Box.
   * @param parent The parent widget.
   */
  Box* make_input_box(QWidget* body, QWidget* parent = nullptr);
}

#endif
