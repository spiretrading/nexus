#ifndef SPIRE_INT_UI_PROPERTY_HPP
#define SPIRE_INT_UI_PROPERTY_HPP
#include "Spire/UiViewer/TypedUiProperty.hpp"

namespace Spire {
  class IntUiProperty : public TypedUiProperty<int> {
    public:
      IntUiProperty(QString name);

      std::any get_value() const override;

      QWidget* make_setter_widget(QWidget* parent) override;

    private:
      int m_value;
  };
}

#endif
