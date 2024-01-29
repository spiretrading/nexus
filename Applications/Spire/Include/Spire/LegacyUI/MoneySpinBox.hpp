#ifndef SPIRE_MONEYSPINBOX_HPP
#define SPIRE_MONEYSPINBOX_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/optional/optional.hpp>
#include <QDoubleSpinBox>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {
namespace LegacyUI {

  /*! \class MoneySpinBox
      \brief Displays a QSpinBox used to represent a Money value.
   */
  class MoneySpinBox : public QDoubleSpinBox {
    public:

      //! Constructs a MoneySpinBox.
      /*!
        \param parent The parent widget.
      */
      MoneySpinBox(QWidget* parent = nullptr);

      //! Constructs a MoneySpinBox.
      /*!
        \param userProfile The user's profile.
        \param node The MoneyNode this MoneySpinBox represents.
        \param parent The parent widget.
      */
      MoneySpinBox(Beam::Ref<UserProfile> userProfile,
        const MoneyNode& node, QWidget* parent = nullptr);

      virtual ~MoneySpinBox();

      //! Returns the value represented.
      Nexus::Money GetValue() const;

      //! Sets the value to display.
      void SetValue(Nexus::Money value);

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
