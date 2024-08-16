#ifndef SPIRE_COMPLIANCEVALUEWIDGET_HPP
#define SPIRE_COMPLIANCEVALUEWIDGET_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Nexus/Compliance/ComplianceParameter.hpp"
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Returns a ComplianceValueWidget to represent a specified value.
  /*!
    \param value The value to represent.
    \param isReadOnly Whether the widget is read-only.
    \param userProfile The user's profile.
    \param parent The parent widget.
    \param flags Qt flags passed to the parent widget.
    \return A ComplianceValueWidget that represents the specified <i>value</i>.
  */
  QWidget* MakeComplianceValueWidget(Nexus::Compliance::ComplianceValue value,
    bool isReadOnly, Beam::Ref<UserProfile> userProfile,
    QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

  //! Returns the ComplianceValue represented by a widget.
  /*!
    \param widget The widget to extract the ComplianceValue from.
  */
  Nexus::Compliance::ComplianceValue GetComplianceValue(const QWidget& widget);

  //! Sets the value of a widget representing a ComplianceValue.
  /*!
    \param widget The widget to set.
    \param value The value to set the widget to.
  */
  void SetComplianceValue(QWidget& widget,
    const Nexus::Compliance::ComplianceValue& value);
}

#endif
