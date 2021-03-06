#ifndef SPIRE_ICON_HPP
#define SPIRE_ICON_HPP
#include "Spire/Styles/StyledWidget.hpp"

namespace Spire {
namespace Styles {

  //! Sets the fill color of an icon.
  using Fill = BasicProperty<QColor, struct FillTag>;
}

  //! Displays an icon.
  class Icon : public Styles::StyledWidget {
    public:

      //! Constructs an Icon.
      /*
        \param icon The icon image.
        \param parnet The parent widget.
      */
      explicit Icon(QImage icon, QWidget* parent = nullptr);

      QSize sizeHint() const override;

    protected:
      void paintEvent(QPaintEvent* event) override;

    private:
      QImage m_icon;
  };
}

#endif
