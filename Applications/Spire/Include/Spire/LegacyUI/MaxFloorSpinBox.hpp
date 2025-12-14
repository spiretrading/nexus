#ifndef SPIRE_MAX_FLOOR_SPIN_BOX_HPP
#define SPIRE_MAX_FLOOR_SPIN_BOX_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/optional/optional.hpp>
#include <QSpinBox>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/LegacyUI/LegacyUI.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire::LegacyUI {

  /** Displays a QSpinBox specialized to handle max floor quantities. */
  class MaxFloorSpinBox : public QSpinBox {
    public:

      /**
       * Constructs a MaxFloorSpinBox.
       * @param userProfile The user's profile.
       * @param node The MaxFloorNode this MaxFloorSpinBox represents.
       * @param parent The parent widget.
       */
      MaxFloorSpinBox(Beam::Ref<UserProfile> userProfile,
        const MaxFloorNode& node, QWidget* parent = nullptr);

      ~MaxFloorSpinBox() override;

      void stepBy(int steps) override;

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
