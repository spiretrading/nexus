#include "Spire/CanvasView/CondensedCanvasCell.hpp"
#include <Beam/Utilities/VariantLambdaVisitor.hpp>
#include <QApplication>
#include <QEvent>
#include <QFocusEvent>
#include <QLayout>
#include <QLineEdit>
#include <QUndoStack>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/CanvasView/CanvasNodeEditor.hpp"
#include "Spire/CanvasView/CommitCanvasEditor.hpp"
#include "Spire/CanvasView/CondensedCanvasWidget.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

CondensedCanvasCell::CondensedCanvasCell(Ref<UserProfile> userProfile,
    Ref<CondensedCanvasWidget> parent, Ref<const CanvasNode> node)
    : QWidget{parent.get()},
      m_userProfile{userProfile.get()},
      m_parent{parent.get()},
      m_node{node.get()},
      m_editor{nullptr} {
  auto layout = new QVBoxLayout(this);
  m_valueWidget = new QLineEdit(this);
  layout->setMargin(0);
  setLayout(layout);
  m_valueWidget->setText(QString::fromStdString(m_node->GetText()));
  m_valueWidget->installEventFilter(this);
  m_valueWidget->setReadOnly(true);
  layout->addWidget(m_valueWidget);
}

const CanvasNode& CondensedCanvasCell::GetNode() const {
  return *m_node;
}

QSize CondensedCanvasCell::sizeHint() const {
  return m_valueWidget->sizeHint();
}

QSize CondensedCanvasCell::minimumSizeHint() const {
  return m_valueWidget->minimumSizeHint();
}

bool CondensedCanvasCell::eventFilter(QObject* object, QEvent* event) {
  if(event->type() == QEvent::FocusIn && object == m_valueWidget) {
    m_valueWidget->setStyleSheet(
      "QLineEdit { background-color: #000080; color: #ffffff }");
  } else if(event->type() == QEvent::FocusOut && object == m_valueWidget) {
    m_valueWidget->setStyleSheet("");
  } else if(event->type() == QEvent::FocusOut && object == m_editor) {
    focusOutEvent(static_cast<QFocusEvent*>(event));
    return QWidget::eventFilter(object, event);
  } else if(event->type() == QEvent::KeyPress) {
    auto keyEvent = static_cast<QKeyEvent*>(event);
    auto key = keyEvent->key();
    if(object == m_editor) {
      if(key == Qt::Key_Enter || key == Qt::Key_Return) {
        m_valueWidget->setText(QString::fromStdString(m_node->GetText()));
        auto hasFocus = m_editor->hasFocus();
        layout()->removeWidget(m_editor);
        m_editor->removeEventFilter(this);
        auto editor = m_editor;
        m_editor = nullptr;
        layout()->addWidget(m_valueWidget);
        m_valueWidget->setEnabled(true);
        if(hasFocus) {
          m_valueWidget->setFocus();
        }
        auto command = CommitCanvasEditor(*m_node, *editor, *m_parent,
          *m_userProfile);
        editor->deleteLater();
        if(command != nullptr) {
          command->redo();
          delete command;
        }
        return QWidget::eventFilter(object, event);
      } else if(key == Qt::Key_Escape) {
        auto hasFocus = m_editor->hasFocus();
        layout()->removeWidget(m_editor);
        m_editor->removeEventFilter(this);
        m_editor->deleteLater();
        m_editor = nullptr;
        layout()->addWidget(m_valueWidget);
        m_valueWidget->setEnabled(true);
        if(hasFocus) {
          m_valueWidget->setFocus();
        }
        return QWidget::eventFilter(object, event);
      }
    } else if(!keyEvent->text().isEmpty() && key != Qt::Key_Enter &&
        key != Qt::Key_Return && key != Qt::Key_Tab && key != Qt::Key_Backtab ||
        key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Left ||
        key == Qt::Key_Right) {
      if(m_editor != nullptr) {
        QApplication::sendEvent(m_editor, event);
      } else {
        CanvasNodeEditor editor;
        auto editVariant = editor.GetEditor(Ref(*m_node), Ref(*m_parent),
          Ref(*m_userProfile), event);
        apply_variant_lambda_visitor(editVariant,
          [&] (QWidget* widget) {
            m_editor = widget;
            m_editor->setSizePolicy(m_valueWidget->sizePolicy());
            layout()->replaceWidget(m_valueWidget, m_editor);
            m_editor->setFocus();
            m_editor->installEventFilter(this);
          },
          [&] (QUndoCommand* command) {
            if(command == nullptr) {
              return;
            }
            m_valueWidget->setText(QString::fromStdString(m_node->GetText()));
            command->redo();
            delete command;
          });
      }
      return true;
    }
  }
  return QWidget::eventFilter(object, event);
}

void CondensedCanvasCell::focusInEvent(QFocusEvent* event) {
  if(m_editor != nullptr) {
    m_editor->setFocus();
  } else {
    m_valueWidget->setFocus();
  }
}

void CondensedCanvasCell::focusOutEvent(QFocusEvent* event) {
  if(m_editor != nullptr) {
    auto editor = m_editor;
    m_valueWidget->setText(QString::fromStdString(m_node->GetText()));
    layout()->removeWidget(m_editor);
    m_editor->removeEventFilter(this);
    m_editor->deleteLater();
    m_editor = nullptr;
    layout()->addWidget(m_valueWidget);
    m_valueWidget->setEnabled(true);
    auto command = CommitCanvasEditor(*m_node, *editor, *m_parent,
      *m_userProfile);
    if(command != nullptr) {
      auto parent = m_parent;
      command->redo();
      delete command;
      if(event->reason() == Qt::TabFocusReason) {
        parent->NavigateForward();
      } else if(event->reason() == Qt::BacktabFocusReason) {
        parent->NavigateBackward();
      }
    }
  }
  return QWidget::focusOutEvent(event);
}
