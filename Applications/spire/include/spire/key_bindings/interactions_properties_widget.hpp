#ifndef SPIRE_INTERACTIONS_PROPERTIES_WIDGET_HPP
#define SPIRE_INTERACTIONS_PROPERTIES_WIDGET_HPP
#include <QWidget>
#include "spire/key_bindings/key_bindings.hpp"

namespace spire {

  //! Displays and allows modification of the interaction properties.
  class InteractionsPropertiesWidget : public QWidget {
    public:

      //! Constructs an InteractionsPropertiesWidget.
      /*!
        \param parent The parent widget.
      */
      InteractionsPropertiesWidget(QWidget* parent = nullptr);
  };
}

#endif
