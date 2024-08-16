#ifndef SPIRE_VALUELISTINPUTWIDGET_HPP
#define SPIRE_VALUELISTINPUTWIDGET_HPP
#include <typeinfo>
#include <vector>
#include <QWidget>
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/signal.hpp>
#include "Spire/InputWidgets/ValueListInputDialog.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/UI.hpp"

class QLineEdit;

namespace Spire {

  /*! \class ValueListInputWidget
      \brief Displays a widget to input a list of values.
   */
  class ValueListInputWidget : public QWidget {
    public:

      //! Signals a change to the value.
      /*!
        \param values The updated values.
      */
      using ValuesUpdatedSignal = boost::signals2::signal<
        void (const std::vector<ValueListInputDialog::Value>& values)>;

      //! Constructs a ValueListInputWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      ValueListInputWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = 0);

      //! Constructs a ValueListInputWidget.
      /*!
        \param userProfile The user's profile.
        \param type The type of value to display.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      ValueListInputWidget(Beam::Ref<UserProfile> userProfile,
        const std::type_info& type, QWidget* parent = nullptr,
        Qt::WindowFlags flags = 0);

      virtual ~ValueListInputWidget();

      //! Initializes this widget.
      /*!
        \param userProfile The user's profile.
        \param type The type of value to display.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile,
        const std::type_info& type);

      //! Returns the list of values.
      const std::vector<ValueListInputDialog::Value>& GetValues() const;

      //! Sets the list of values.
      /*!
        \param values The list of values.
      */
      void SetValues(std::vector<ValueListInputDialog::Value> values);

      //! Makes this widget read-only.
      /*!
        \param value <code>true</code> to make this widget read-only.
      */
      void SetReadOnly(bool value);

      //! Connects a slot to the ValuesUpdatedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectValuesUpdatedSignal(
        const ValuesUpdatedSignal::slot_type& slot) const;

    protected:
      virtual bool eventFilter(QObject* receiver, QEvent* event);
      virtual void keyPressEvent(QKeyEvent* event);
      virtual void mouseDoubleClickEvent(QMouseEvent* event);

    private:
      UserProfile* m_userProfile;
      const std::type_info* m_type;
      std::vector<ValueListInputDialog::Value> m_values;
      QLineEdit* m_lineEdit;
      std::unique_ptr<UI::CustomVariantItemDelegate> m_itemDelegate;
      bool m_isReadOnly;
      mutable ValuesUpdatedSignal m_valuesUpdatedSignal;
  };
}

#endif
