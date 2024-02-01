#include "Spire/InputWidgets/SecurityInputWidget.hpp"
#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QVBoxLayout>
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Spire/InputWidgets/SecurityInputDialog.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SecurityInputWidget::SecurityInputWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_isReadOnly(false) {
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);
  m_lineEdit = new QLineEdit(this);
  m_lineEdit->installEventFilter(this);
  layout->addWidget(m_lineEdit);
}

SecurityInputWidget::SecurityInputWidget(Ref<UserProfile> userProfile,
    QWidget* parent, Qt::WindowFlags flags)
    : SecurityInputWidget{parent, flags} {
  Initialize(Ref(userProfile));
}

void SecurityInputWidget::Initialize(Ref<UserProfile> userProfile) {
  m_userProfile = userProfile.Get();
}

const Security& SecurityInputWidget::GetSecurity() const {
  return m_security;
}

void SecurityInputWidget::SetSecurity(Security security) {
  m_security = std::move(security);
  m_lineEdit->setText(QString::fromStdString(ToWildCardString(security,
    m_userProfile->GetMarketDatabase(), m_userProfile->GetCountryDatabase())));
  m_securityUpdatedSignal(m_security);
}

void SecurityInputWidget::SetReadOnly(bool value) {
  m_isReadOnly = value;
}

connection SecurityInputWidget::ConnectSecurityUpdatedSignal(
    const SecurityUpdatedSignal::slot_type& slot) const {
  return m_securityUpdatedSignal.connect(slot);
}

bool SecurityInputWidget::eventFilter(QObject* receiver, QEvent* event) {
  if(receiver == m_lineEdit) {
    if(event->type() == QEvent::MouseButtonDblClick) {
      mouseDoubleClickEvent(static_cast<QMouseEvent*>(event));
      return true;
    } else if(event->type() == QEvent::KeyPress) {
      keyPressEvent(static_cast<QKeyEvent*>(event));
      return true;
    }
  }
  return QWidget::eventFilter(receiver, event);
}

void SecurityInputWidget::keyPressEvent(QKeyEvent* event) {
  auto text = event->text().trimmed();
  if(event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab ||
      event->key() == Qt::Key_Escape || text.isEmpty()) {
    QWidget::keyPressEvent(event);
    return;
  }
  if(m_isReadOnly) {
    return;
  }
  ShowWildCardSecurityInputDialog(Ref(*m_userProfile), text.toStdString(), this,
    [=] (auto security) {
      if(!security || security == Security()) {
        return;
      }
      SetSecurity(std::move(*security));
    });
}

void SecurityInputWidget::mouseDoubleClickEvent(QMouseEvent* event) {
  if(m_isReadOnly) {
    return;
  }
  auto dialog = SecurityInputDialog(Ref(*m_userProfile), m_security, this);
  dialog.GetSymbolInput().selectAll();
  if(dialog.exec() == QDialog::Rejected) {
    return;
  }
  auto newValue = dialog.GetSecurity(true);
  if(newValue == Security()) {
    return;
  }
  SetSecurity(newValue);
}
