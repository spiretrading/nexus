#ifndef SPIRE_BOX_HPP
#define SPIRE_BOX_HPP
#include "Spire/Styles/StyledWidget.hpp"

namespace Spire {
  class Box : public Styles::StyledWidget {
    public:
      explicit Box(QWidget& body, QWidget* parent = nullptr);

      explicit Box(QWidget* parent = nullptr);

    protected:
      void enterEvent(QEvent* event) override;
      void leaveEvent(QEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      QWidget* m_body;

      Box(QWidget* body, QWidget* parent);
  };
}

#endif
