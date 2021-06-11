#ifndef SPIRE_MONEY_BOX_HPP
#define SPIRE_MONEY_BOX_HPP
#include "Nexus/Definitions/Money.hpp"
#include "Spire/Ui/DecimalBoxAdaptor.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** A ScalarValueModel over Money values. */
  using MoneyModel = ScalarValueModel<Nexus::Money>;

  /** A ScalarValueModel over optional Money values. */
  using OptionalMoneyModel = ScalarValueModel<boost::optional<Nexus::Money>>;

  /** A LocalScalarValueModel over Money values. */
  using LocalMoneyModel = LocalScalarValueModel<Nexus::Money>;

  /** A LocalScalarValueModel over optional Money values. */
  using LocalOptionalMoneyModel =
    LocalScalarValueModel<boost::optional<Nexus::Money>>;

  /** Represents a widget for inputting Money values. */
  class MoneyBox : public DecimalBoxAdaptor<Nexus::Money> {
    public:

      /**
       * Constructs a MoneyBox with a LocalValueModel.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      explicit MoneyBox(QHash<Qt::KeyboardModifier, Nexus::Money> modifiers,
        QWidget* parent = nullptr);

      /**
       * Constructs a MoneyBox.
       * @param model The model used for the current value.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      MoneyBox(std::shared_ptr<OptionalMoneyModel> model,
        QHash<Qt::KeyboardModifier, Nexus::Money> modifiers,
        QWidget* parent = nullptr);
  };
}

#endif
