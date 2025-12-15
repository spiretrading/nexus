#ifndef SPIRE_QUANTITY_SPIN_BOX_HPP
#define SPIRE_QUANTITY_SPIN_BOX_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/optional/optional.hpp>
#include <QSpinBox>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire::LegacyUI {

  /** Displays a QSpinBox used to represent a Quantity. */
  class QuantitySpinBox : public QSpinBox {
    public:

      /**
       * Constructs a QuantitySpinBox.
       * @param userProfile The user's profile.
       * @param node The IntegerNode this QuantitySpinBox represents.
       * @param parent The parent widget.
       */
      QuantitySpinBox(Beam::Ref<UserProfile> userProfile,
        const IntegerNode& node, QWidget* parent = nullptr);

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
