#ifndef SPIRE_MAXFLOORSPINBOX_HPP
#define SPIRE_MAXFLOORSPINBOX_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/optional/optional.hpp>
#include <QSpinBox>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {
namespace UI {

  /*! \class MaxFloorSpinBox
      \brief Displays a QSpinBox specialized to handle max floor quantities.
   */
  class MaxFloorSpinBox : public QSpinBox {
    public:

      //! Constructs a MaxFloorSpinBox.
      /*!
        \param userProfile The user's profile.
        \param node The MaxFloorNode this MaxFloorSpinBox represents.
        \param parent The parent widget.
      */
      MaxFloorSpinBox(Beam::Ref<UserProfile> userProfile,
        const MaxFloorNode& node, QWidget* parent = nullptr);

      virtual ~MaxFloorSpinBox();

      virtual void stepBy(int steps);

    protected:
      virtual void keyPressEvent(QKeyEvent* event);
      virtual void keyReleaseEvent(QKeyEvent* event);

    private:
      UserProfile* m_userProfile;
      boost::optional<Nexus::Security> m_security;

      void AdjustIncrement(KeyModifiers modifier);
  };
}
}

#endif
