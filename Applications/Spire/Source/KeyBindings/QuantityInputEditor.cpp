#include "Spire/KeyBindings/QuantityInputEditor.hpp"
#include <QIntValidator>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

QuantityInputEditor::QuantityInputEditor(int initial_value, QWidget* parent)
    : TextInputWidget(parent),
      m_initial_value(initial_value) {
  setContextMenuPolicy(Qt::NoContextMenu);
  setValidator(new QIntValidator(0, std::numeric_limits<int>::max(), this));
}

void QuantityInputEditor::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Delete) {
    setText("0");
    Q_EMIT editingFinished();
    return;
  } else if(event->key() == Qt::Key_Enter ||
      event->key() == Qt::Key_Return) {
    if(text().isEmpty()) {
      setText(QString::number(m_initial_value));
    }
    Q_EMIT editingFinished();
    return;
  }
  TextInputWidget::keyPressEvent(event);
}
