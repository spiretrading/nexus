#ifndef SPIRE_ICON_HPP
#define SPIRE_ICON_HPP
#include <boost/optional/optional.hpp>
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/BoxPainter.hpp"

namespace Spire {
namespace Styles {

  /** Sets the icon image of an icon. */
  using IconImage = BasicProperty<QImage, struct IconImageTag>;

  /** Sets the fill color of an icon. */
  using Fill = BasicProperty<boost::optional<QColor>, struct FillTag>;
}

  /** Displays an icon. */
  class Icon : public QWidget {
    public:

      /**
       * Constructs an Icon.
       * @param icon The icon image.
       * @param parnet The parent widget.
       */
      explicit Icon(QImage icon, QWidget* parent = nullptr);

      QSize sizeHint() const override;

    protected:
      void paintEvent(QPaintEvent* event) override;

    private:
      QImage m_icon;
      boost::optional<QColor> m_fill;
      BoxPainter m_painter;
      boost::signals2::scoped_connection m_style_connection;

      void on_style();
  };
}

#endif
