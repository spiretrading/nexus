#ifndef SPIRE_LAYERED_WIDGET_HPP
#define SPIRE_LAYERED_WIDGET_HPP
#include <vector>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays multiple widgets layered ontop of one another. */
  class LayeredWidget : public QWidget {
    public:

      /**
       * Constructs an empty LayeredWidget.
       * @param parent The parent widget.
       */
      explicit LayeredWidget(QWidget* parent = nullptr);

      /** Adds a layer on top of all other widgets. */
      void add(QWidget* widget);

      QSize sizeHint() const override;

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      std::vector<QWidget*> m_layers;
  };
}

#endif
