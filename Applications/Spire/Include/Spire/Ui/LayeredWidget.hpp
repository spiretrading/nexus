#ifndef SPIRE_LAYERED_WIDGET_HPP
#define SPIRE_LAYERED_WIDGET_HPP
#include <vector>
#include <boost/optional/optional.hpp>
#include <QWidget>

namespace Spire {

  /** Displays multiple widgets layered ontop of one another. */
  class LayeredWidget : public QWidget {
    public:
      using QWidget::QWidget;

      /** Adds a layer on top of all other widgets. */
      void add(QWidget* widget);

      QSize sizeHint() const override;

    protected:
      bool event(QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable boost::optional<QSize> m_size_hint;
      std::vector<QWidget*> m_layers;
  };
}

#endif
