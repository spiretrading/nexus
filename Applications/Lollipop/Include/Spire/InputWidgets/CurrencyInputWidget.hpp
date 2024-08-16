#ifndef SPIRE_CURRENCYINPUTWIDGET_HPP
#define SPIRE_CURRENCYINPUTWIDGET_HPP
#include <QWidget>
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/signal.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Spire/Spire/Spire.hpp"

class QComboBox;

namespace Spire {

  /*! \class CurrencyInputWidget
      \brief Displays a combo box for selecting a currency.
   */
  class CurrencyInputWidget : public QWidget {
    public:

      //! Signals a change to the currency.
      /*!
        \param currency The updated currency.
      */
      using CurrencyUpdatedSignal = boost::signals2::signal<
        void (Nexus::CurrencyId currency)>;

      //! Constructs a CurrencyInputWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      CurrencyInputWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      //! Constructs a CurrencyInputWidget.
      /*!
        \param userProfile The user's profile.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      CurrencyInputWidget(Beam::Ref<UserProfile> userProfile,
        QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      virtual ~CurrencyInputWidget();

      //! Initializes this widget.
      /*!
        \param userProfile The user's profile.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile);

      //! Returns the currency displayed.
      Nexus::CurrencyId GetCurrency() const;

      //! Sets the currency to display.
      /*!
        \param currency The currency to display.
      */
      void SetCurrency(Nexus::CurrencyId currency);

      //! Makes this widget read-only.
      /*!
        \param value <code>true</code> to make this widget read-only.
      */
      void SetReadOnly(bool value);

      //! Connects a slot to the CurrencyUpdatedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectCurrencyUpdatedSignal(
        const CurrencyUpdatedSignal::slot_type& slot) const;

    private:
      UserProfile* m_userProfile;
      QComboBox* m_currencyComboBox;
      bool m_isReadOnly;
      mutable CurrencyUpdatedSignal m_currencyUpdatedSignal;

      void OnCurrencyActivated(int index);
  };
}

#endif
