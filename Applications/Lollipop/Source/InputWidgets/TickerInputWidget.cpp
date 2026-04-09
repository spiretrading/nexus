#include "Spire/InputWidgets/TickerInputWidget.hpp"
#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QVBoxLayout>
#include "Spire/InputWidgets/TickerInputDialog.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;

TickerInputWidget::TickerInputWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_isReadOnly(false) {
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);
  m_lineEdit = new QLineEdit(this);
  m_lineEdit->installEventFilter(this);
  layout->addWidget(m_lineEdit);
}

TickerInputWidget::TickerInputWidget(Ref<UserProfile> userProfile,
    QWidget* parent, Qt::WindowFlags flags)
    : TickerInputWidget{parent, flags} {
  Initialize(Ref(userProfile));
}

void TickerInputWidget::Initialize(Ref<UserProfile> userProfile) {
  m_userProfile = userProfile.get();
}

const Ticker& TickerInputWidget::GetTicker() const {
  return m_ticker;
}

void TickerInputWidget::SetTicker(Ticker ticker) {
  m_ticker = std::move(ticker);
  m_lineEdit->setText(displayText(m_ticker));
  m_tickerUpdatedSignal(m_ticker);
}

void TickerInputWidget::SetReadOnly(bool value) {
  m_isReadOnly = value;
}

connection TickerInputWidget::ConnectTickerUpdatedSignal(
    const TickerUpdatedSignal::slot_type& slot) const {
  return m_tickerUpdatedSignal.connect(slot);
}

bool TickerInputWidget::eventFilter(QObject* receiver, QEvent* event) {
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

void TickerInputWidget::keyPressEvent(QKeyEvent* event) {
  auto text = event->text().trimmed();
  if(event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab ||
      event->key() == Qt::Key_Escape || text.isEmpty()) {
    QWidget::keyPressEvent(event);
    return;
  }
  if(m_isReadOnly) {
    return;
  }
  ShowTickerInputDialog(Ref(*m_userProfile), text.toStdString(), this,
    [=] (auto ticker) {
      if(!ticker || ticker == Ticker()) {
        return;
      }
      SetTicker(std::move(*ticker));
    });
}

void TickerInputWidget::mouseDoubleClickEvent(QMouseEvent* event) {
  if(m_isReadOnly) {
    return;
  }
  auto dialog = TickerInputDialog(Ref(*m_userProfile), m_ticker, this);
  dialog.GetSymbolInput().selectAll();
  if(dialog.exec() == QDialog::Rejected) {
    return;
  }
  auto newValue = dialog.GetTicker();
  if(newValue == Ticker()) {
    return;
  }
  SetTicker(newValue);
}
