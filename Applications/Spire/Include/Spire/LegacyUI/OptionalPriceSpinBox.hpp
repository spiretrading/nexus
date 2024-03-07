#ifndef SPIRE_OPTIONAL_PRICE_SPIN_BOX_HPP
#define SPIRE_OPTIONAL_PRICE_SPIN_BOX_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/optional/optional.hpp>
#include <QDoubleSpinBox>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/LegacyUI/LegacyUI.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire::LegacyUI {

  /** Displays a QDoubleSpinBox specialized to handle optional prices. */
  class OptionalPriceSpinBox : public QDoubleSpinBox {
    public:

      /**
       * Constructs an OptionalPriceSpinBox.
       * @param userProfile The user's profile.
       * @param node The OptionalPriceNode represented.
       * @param parent The parent widget.
       */
      OptionalPriceSpinBox(Beam::Ref<UserProfile> userProfile,
        const OptionalPriceNode& node, QWidget* parent = nullptr);

      ~OptionalPriceSpinBox() override;

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
      Nexus::Money m_referencePrice;

      void AdjustIncrement(Qt::KeyboardModifiers modifier);
  };
}

#endif
