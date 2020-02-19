#ifndef SPIRE_COLON_WIDGET_HPP
#define SPIRE_COLON_WIDGET_HPP
#include <QWidget>

namespace Spire {

  class ColonWidget : public QWidget {
    public:

      explicit ColonWidget(QWidget* parent = nullptr);

      void set_default_style();

      void set_active_style();

    protected:
      void paintEvent(QPaintEvent* event) override;

    private:
      QColor m_border_color;
  };
}

#endif
