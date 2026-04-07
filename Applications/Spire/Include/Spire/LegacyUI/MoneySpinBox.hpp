#ifndef SPIRE_MONEY_SPIN_BOX_HPP
#define SPIRE_MONEY_SPIN_BOX_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/optional/optional.hpp>
#include <QDoubleSpinBox>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire::LegacyUI {

  /** Displays a QSpinBox used to represent a Money value. */
  class MoneySpinBox : public QDoubleSpinBox {
    public:

      /**
       * Constructs a MoneySpinBox.
       * @param parent The parent widget.
       */
      explicit MoneySpinBox(QWidget* parent = nullptr);

      /**
       * Constructs a MoneySpinBox.
       * @param userProfile The user's profile.
       * @param node The MoneyNode this MoneySpinBox represents.
       * @param parent The parent widget.
       */
      MoneySpinBox(Beam::Ref<UserProfile> userProfile, const MoneyNode& node,
        QWidget* parent = nullptr);

      ~MoneySpinBox() override;

      /** Returns the value represented. */
      Nexus::Money GetValue() const;

      /** Sets the value to display. */
      void SetValue(Nexus::Money value);

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;

    private:
      UserProfile* m_userProfile;
      boost::optional<Nexus::Security> m_security;

      void AdjustIncrement(Qt::KeyboardModifier modifier);
  };
}

#endif
