#ifndef SPIRE_QUANTITY_BOX_HPP
#define SPIRE_QUANTITY_BOX_HPP
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Ui/DecimalBoxAdaptor.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** A ScalarValueModel over Quantity values. */
  using QuantityModel = ScalarValueModel<Nexus::Quantity>;

  /** A ScalarValueModel over optional Quantity values. */
  using OptionalQuantityModel =
    ScalarValueModel<boost::optional<Nexus::Quantity>>;

  /** A LocalScalarValueModel over Quantity values. */
  using LocalQuantityModel = LocalScalarValueModel<Nexus::Quantity>;

  /** A LocalScalarValueModel over optional Quantity values. */
  using LocalOptionalQuantityModel =
    LocalScalarValueModel<boost::optional<Nexus::Quantity>>;

  /** Represents a widget for inputting Quantity values. */
  class QuantityBox : public DecimalBoxAdaptor<Nexus::Quantity> {
    public:

      /**
       * Constructs a QuantityBox with a LocalOptionalValueModel.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      explicit QuantityBox(QHash<Qt::KeyboardModifier, Nexus::Quantity>
        modifiers, QWidget* parent = nullptr);

      /**
       * Constructs a QuantityBox.
       * @param model The model used for the current value.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      QuantityBox(std::shared_ptr<OptionalQuantityModel> model,
        QHash<Qt::KeyboardModifier, Nexus::Quantity> modifiers,
        QWidget* parent = nullptr);
  };
}

#endif
