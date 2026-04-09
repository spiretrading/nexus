#ifndef SPIRE_OPTIONALPRICESPINBOX_HPP
#define SPIRE_OPTIONALPRICESPINBOX_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/optional/optional.hpp>
#include <QDoubleSpinBox>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {
namespace UI {

  /*! \class OptionalPriceSpinBox
      \brief Displays a QDoubleSpinBox specialized to handle optional prices.
   */
  class OptionalPriceSpinBox : public QDoubleSpinBox {
    public:

      //! Constructs an OptionalPriceSpinBox.
      /*!
        \param userProfile The user's profile.
        \param node The OptionalPriceNode represented.
        \param parent The parent widget.
      */
      OptionalPriceSpinBox(Beam::Ref<UserProfile> userProfile,
        const OptionalPriceNode& node, QWidget* parent = nullptr);

      virtual ~OptionalPriceSpinBox();

      //! Returns the value represented.
      Nexus::Money GetValue() const;

      //! Sets the value to display.
      void SetValue(Nexus::Money value);

    protected:
      virtual void keyPressEvent(QKeyEvent* event);
      virtual void keyReleaseEvent(QKeyEvent* event);

    private:
      UserProfile* m_userProfile;
      boost::optional<Nexus::Ticker> m_ticker;
      Nexus::Money m_referencePrice;

      void AdjustIncrement(KeyModifiers modifier);
  };
}
}

#endif
