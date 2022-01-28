#ifndef SPIRE_BOX_HPP
#define SPIRE_BOX_HPP
#include "Spire/Ui/BoxGeometry.hpp"
#include "Spire/Ui/BoxPainter.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

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
      QWidget* m_container;
      QWidget* m_body;
      BoxGeometry m_geometry;
      BoxPainter m_painter;
      boost::signals2::scoped_connection m_style_connection;
      mutable boost::optional<QSize> m_size_hint;

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
