#ifndef SPIRE_MONEYSPINBOX_HPP
#define SPIRE_MONEYSPINBOX_HPP
#include <QWidget>
#include <Beam/Pointers/Ref.hpp>
#include <boost/optional/optional.hpp>
#include <boost/signals2/signal.hpp>
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/UI/UI.hpp"

class QDoubleSpinBox;

namespace Spire {

  /*! \class MoneySpinBox
      \brief Displays a QSpinBox used to represent a Money value.
   */
  class MoneySpinBox : public QWidget {
    public:

      //! Signals a change to the value.
      /*!
        \param value The updated value.
      */
      using ValueUpdatedSignal = boost::signals2::signal<
        void (Nexus::Money value)>;

      //! Constructs a MoneySpinBox.
      /*!
        \param parent The parent widget.
        \param flags The widget's flags.
      */
      MoneySpinBox(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      //! Constructs a MoneySpinBox.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags The widget's flags.
      */
      MoneySpinBox(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      //! Initializes this widget.
      /*!
        \param userProfile The user's profile.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile);

      //! Returns the Ticker linked to.
      const boost::optional<Nexus::Ticker>& GetLinkedTicker() const;

      //! Links this spin box to a Ticker.
      /*!
        \param ticker The Ticker to link to.
      */
      void SetLinkedTicker(const boost::optional<Nexus::Ticker>& ticker);

      //! Sets the number of decimal places to display.
      /*!
        \param decimalCount The number of decimals to display.
      */
      void SetDecimals(int decimalCount);

      //! Returns the number of decimals displayed.
      int GetDecimals() const;

      //! Returns the value represented.
      Nexus::Money GetValue() const;

      //! Sets the value to display.
      void SetValue(Nexus::Money value);

      //! Makes this widget read-only.
      /*!
        \param value <code>true</code> to make this widget read-only.
      */
      void SetReadOnly(bool value);

      //! Connects a slot to the ValueUpdatedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectValueUpdatedSignal(
        const ValueUpdatedSignal::slot_type& slot) const;

    protected:
      virtual bool eventFilter(QObject* receiver, QEvent* event);

    private:
      UserProfile* m_userProfile;
      boost::optional<Nexus::Ticker> m_ticker;
      QDoubleSpinBox* m_spinBox;
      bool m_isReadOnly;
      mutable ValueUpdatedSignal m_valueUpdatedSignal;

      void AdjustIncrement(KeyModifiers modifier);
      void OnValueChanged(const QString& value);
  };
}

#endif
