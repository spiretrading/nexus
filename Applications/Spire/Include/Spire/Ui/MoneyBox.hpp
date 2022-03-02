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
       * Constructs a MoneyBox using a local current value model and default
       * keyboard modifiers.
       * @param parent The parent widget.
       */
      explicit MoneyBox(QWidget* parent = nullptr);

      /**
       * Constructs a MoneyBox with a LocalOptionalValueModel.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      explicit MoneyBox(QHash<Qt::KeyboardModifier, Nexus::Money> modifiers,
        QWidget* parent = nullptr);

      /**
       * Constructs a MoneyBox.
       * @param current The current value model.
       * @param modifiers The keyboard modifier increments.
       * @param parent The parent widget.
       */
      MoneyBox(std::shared_ptr<OptionalMoneyModel> current,
        QHash<Qt::KeyboardModifier, Nexus::Money> modifiers,
        QWidget* parent = nullptr);
  };

  /**
   * Makes a MoneyBox used as a disabled and read-only label.
   * @param current The current value to represent.
   * @param parent The parent widget.
   */
  MoneyBox* make_money_label(
    std::shared_ptr<MoneyModel> current, QWidget* parent = nullptr);
}

#endif
