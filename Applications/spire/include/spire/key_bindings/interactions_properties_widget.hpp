#ifndef SPIRE_INTERACTIONS_PROPERTIES_WIDGET_HPP
#define SPIRE_INTERACTIONS_PROPERTIES_WIDGET_HPP
#include <QWidget>
#include "spire/key_bindings/key_bindings.hpp"

namespace spire {

  //! Displays and allows modification of the interaction properties.
  class interactions_properties_widget : public QWidget {
    public:

      //! Constructs an interactions_properties_widget.
      /*!
        \param parent The parent widget.
      */
      interactions_properties_widget(QWidget* parent = nullptr);
  };
}

#endif
