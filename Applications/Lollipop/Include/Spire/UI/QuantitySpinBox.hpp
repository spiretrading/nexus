#ifndef SPIRE_QUANTITYSPINBOX_HPP
#define SPIRE_QUANTITYSPINBOX_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/optional/optional.hpp>
#include <QSpinBox>
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {
namespace UI {

  /*! \class QuantitySpinBox
      \brief Displays a QSpinBox used to represent a Quantity.
   */
  class QuantitySpinBox : public QSpinBox {
    public:

      //! Constructs a QuantitySpinBox.
      /*!
        \param userProfile The user's profile.
        \param node The IntegerNode this QuantitySpinBox represents.
        \param parent The parent widget.
      */
      QuantitySpinBox(Beam::Ref<UserProfile> userProfile,
        const IntegerNode& node, QWidget* parent = nullptr);

    protected:
      virtual void keyPressEvent(QKeyEvent* event);
      virtual void keyReleaseEvent(QKeyEvent* event);

    private:
      UserProfile* m_userProfile;
      boost::optional<Nexus::Ticker> m_ticker;

      void AdjustIncrement(KeyModifiers modifier);
  };
}
}

#endif
