#include "Spire/UI/CollapsibleWidgetWindowSettings.hpp"
#include <QBoxLayout>
#include <QLineEdit>
#include "Spire/UI/CollapsibleWidget.hpp"
#include "Spire/UI/ExpandButton.hpp"
#include "Spire/UI/PersistentWindow.hpp"

using namespace Beam;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

CollapsibleWidgetWindowSettings::CollapsibleWidgetWindowSettings() {}

CollapsibleWidgetWindowSettings::CollapsibleWidgetWindowSettings(
    const CollapsibleWidget& widget)
    : m_label(widget.m_header->text().toStdString()),
      m_isExpanded(widget.m_expandButton->IsExpanded()),
      m_geometry(widget.saveGeometry()) {
  m_subWindowSettings = dynamic_cast<const PersistentWindow&>(
    *widget.m_widget).GetWindowSettings();
}

CollapsibleWidgetWindowSettings::~CollapsibleWidgetWindowSettings() {}

string CollapsibleWidgetWindowSettings::GetName() const {
  return m_subWindowSettings->GetName();
}

QWidget* CollapsibleWidgetWindowSettings::Reopen(
    Ref<UserProfile> userProfile) const {
  QWidget* subWidget = m_subWindowSettings->Reopen(Ref(userProfile));
  CollapsibleWidget* widget = new CollapsibleWidget(
    QString::fromStdString(m_label), subWidget, m_isExpanded);
  widget->restoreGeometry(m_geometry);
  return widget;
}

void CollapsibleWidgetWindowSettings::Apply(Ref<UserProfile> userProfile,
    Out<QWidget> widget) const {
  CollapsibleWidget& collapsibleWidget = dynamic_cast<CollapsibleWidget&>(
    *widget);
  collapsibleWidget.m_header->setText(QString::fromStdString(m_label));
  collapsibleWidget.SetExpanded(m_isExpanded, false);
  collapsibleWidget.restoreGeometry(m_geometry);
  if(collapsibleWidget.m_widget != nullptr) {
    m_subWindowSettings->Apply(
      Ref(userProfile), out(*collapsibleWidget.m_widget));
  }
}
