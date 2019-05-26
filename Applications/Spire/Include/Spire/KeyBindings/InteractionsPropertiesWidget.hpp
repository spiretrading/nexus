#ifndef SPIRE_INTERACTIONS_PROPERTIES_WIDGET_HPP
#define SPIRE_INTERACTIONS_PROPERTIES_WIDGET_HPP
#include <QWidget>
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  //! Displays and allows modification of the interaction properties.
  class InteractionsPropertiesWidget : public QWidget {
    public:

      //! Constructs an InteractionsPropertiesWidget.
      /*!
        \param parent The parent widget.
      */
      explicit InteractionsPropertiesWidget(QWidget* parent = nullptr);
  };
}

#endif
