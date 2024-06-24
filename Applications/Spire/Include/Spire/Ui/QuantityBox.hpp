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
       * Constructs a QuantityBox with a LocalOptionalValueModel and default
       * modifiers.
       * @param parent The parent widget.
       */
      explicit QuantityBox(QWidget* parent = nullptr);

      /**
       * Constructs a QuantityBox with a LocalOptionalValueModel.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      explicit QuantityBox(QHash<Qt::KeyboardModifier, Nexus::Quantity>
        modifiers, QWidget* parent = nullptr);

      /**
       * Constructs a QuantityBox with default modifiers.
       * @param current The current value model.
       * @param parent The parent widget.
       */
      explicit QuantityBox(std::shared_ptr<OptionalQuantityModel> current,
        QWidget* parent = nullptr);

      /**
       * Constructs a QuantityBox.
       * @param current The current value model.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      QuantityBox(std::shared_ptr<OptionalQuantityModel> current,
        QHash<Qt::KeyboardModifier, Nexus::Quantity> modifiers,
        QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
  };
}

#endif
