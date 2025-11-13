#include "Spire/InputWidgets/ValueListInputWidget.hpp"
#include <QKeyEvent>
#include <QLineEdit>
#include <QLocale>
#include <QVBoxLayout>
#include "Spire/InputWidgets/ValueListInputDialog.hpp"
#include "Spire/UI/CustomQtVariants.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  struct QVariantConverter {
    template<typename T>
    QVariant operator ()(const T& value) const {
      return QVariant::fromValue(value);
    }
  };
}

ValueListInputWidget::ValueListInputWidget(QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget{parent, flags},
      m_userProfile{nullptr},
      m_type{&typeid(Security)},
      m_isReadOnly{false} {
  auto layout = new QVBoxLayout{this};
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);
  m_lineEdit = new QLineEdit{this};
  m_lineEdit->installEventFilter(this);
  m_lineEdit->setReadOnly(true);
  layout->addWidget(m_lineEdit);
}

ValueListInputWidget::ValueListInputWidget(Ref<UserProfile> userProfile,
    const type_info& type, QWidget* parent, Qt::WindowFlags flags)
    : ValueListInputWidget{parent, flags} {
  Initialize(Ref(userProfile), type);
}

ValueListInputWidget::~ValueListInputWidget() {}

void ValueListInputWidget::Initialize(Ref<UserProfile> userProfile,
    const type_info& type) {
  m_userProfile = userProfile.get();
  m_type = &type;
  m_itemDelegate = std::make_unique<CustomVariantItemDelegate>(
    Ref(*m_userProfile));
}

const vector<ValueListInputDialog::Value>&
    ValueListInputWidget::GetValues() const {
  return m_values;
}

void ValueListInputWidget::SetValues(
    vector<ValueListInputDialog::Value> values) {
  m_values = std::move(values);
  m_lineEdit->clear();
  QString text;
  for(auto i = size_t{0}; i != m_values.size(); ++i) {
    text += m_itemDelegate->displayText(
      apply_visitor(QVariantConverter(), m_values[i]), QLocale{});
    if(i != m_values.size() - 1) {
      text += ", ";
    }
  }
  m_lineEdit->setText(text);
  m_valuesUpdatedSignal(m_values);
}

void ValueListInputWidget::SetReadOnly(bool value) {
  m_isReadOnly = value;
}

connection ValueListInputWidget::ConnectValuesUpdatedSignal(
    const ValuesUpdatedSignal::slot_type& slot) const {
  return m_valuesUpdatedSignal.connect(slot);
}

bool ValueListInputWidget::eventFilter(QObject* receiver, QEvent* event) {
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

void ValueListInputWidget::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab ||
      event->text().isEmpty()) {
    QWidget::keyPressEvent(event);
    return;
  }
  if(m_isReadOnly) {
    return;
  }
}

void ValueListInputWidget::mouseDoubleClickEvent(QMouseEvent* event) {
  if(m_isReadOnly) {
    return;
  }
  ValueListInputDialog dialog{Ref(*m_userProfile), *m_type, m_values, this};
  if(dialog.exec() == QDialog::Rejected) {
    return;
  }
  auto newValues = dialog.GetValues();
  SetValues(std::move(newValues));
}
