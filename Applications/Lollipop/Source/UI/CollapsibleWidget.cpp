#include "Spire/UI/CollapsibleWidget.hpp"
#include <QBoxLayout>
#include <QLineEdit>
#include "Spire/UI/CollapsibleWidgetWindowSettings.hpp"
#include "Spire/UI/ExpandButton.hpp"

using namespace Beam;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  const QColor HEADER_BACKGROUND_COLOR = QColor(233, 231, 227);
}

CollapsibleWidget::CollapsibleWidget(QWidget* parent)
    : QWidget(parent),
      m_widget(nullptr) {
  SetupLayouts(false);
}

CollapsibleWidget::CollapsibleWidget(const QString& label, QWidget* widget,
    bool expanded, QWidget* parent)
    : QWidget(parent),
      m_widget(nullptr) {
  SetupLayouts(expanded);
  Initialize(label, widget, expanded);
}

void CollapsibleWidget::Initialize(const QString& label, QWidget* widget,
    bool expanded) {
  SetLabel(label);
  SetWidget(widget);
  SetExpanded(expanded, true);
}

bool CollapsibleWidget::IsExpanded() const {
  return m_expandButton->IsExpanded();
}

void CollapsibleWidget::SetExpanded(bool expanded, bool adjustSize) {
  if(expanded == m_expandButton->IsExpanded()) {
    return;
  }
  m_expandButton->blockSignals(true);
  m_expandButton->SetExpanded(expanded);
  m_expandButton->blockSignals(false);
  if(expanded) {
    Expand(adjustSize);
  } else {
    Collapse(adjustSize);
  }
}

void CollapsibleWidget::SetLabel(const QString& label) {
  m_header->setText(label);
}

void CollapsibleWidget::SetWidget(QWidget* widget) {
  if(m_widget != nullptr) {
    QLayoutItem* item = m_layout->itemAt(1);
    m_layout->removeItem(m_layout->itemAt(1));
    delete item;
    delete m_widget;
  }
  m_widget = widget;
  m_layout->addWidget(m_widget);
  if(m_expandButton->IsExpanded()) {
    m_widget->show();
  } else {
    m_widget->hide();
  }
}

unique_ptr<WindowSettings> CollapsibleWidget::GetWindowSettings() const {
  return std::make_unique<CollapsibleWidgetWindowSettings>(*this);
}

void CollapsibleWidget::Expand(bool adjustSize) {
  if(m_widget != nullptr) {
    if(adjustSize && parentWidget() != nullptr) {
      QSize adjustedSize(parentWidget()->width(),
        parentWidget()->height() + m_widget->sizeHint().height());
      parentWidget()->resize(adjustedSize);
    }
    m_widget->show();
  }
}

void CollapsibleWidget::Collapse(bool adjustSize) {
  if(m_widget != nullptr) {
    if(adjustSize && parentWidget() != nullptr) {
      QSize adjustedSize(parentWidget()->width(),
        parentWidget()->height() - m_widget->sizeHint().height());
      parentWidget()->resize(adjustedSize);
    }
    m_widget->hide();
  }
}

void CollapsibleWidget::SetupLayouts(bool expanded) {
  QBoxLayout* headerLayout = new QBoxLayout(QBoxLayout::LeftToRight);
  headerLayout->setSpacing(0);
  m_expandButton = new ExpandButton(expanded);
  QPalette expandButtonPalette(m_expandButton->palette());
  expandButtonPalette.setColor(QPalette::Window, HEADER_BACKGROUND_COLOR);
  m_expandButton->setAutoFillBackground(true);
  m_expandButton->setPalette(expandButtonPalette);
  headerLayout->addWidget(m_expandButton);
  m_header = new QLineEdit();
  m_header->setFrame(false);
  m_header->setReadOnly(true);
  QFont font = m_header->font();
  font.setBold(true);
  font.setPointSize(9);
  m_header->setFont(font);
  QString headerStyleSheet = "QLineEdit {background: rgb(%1, %2, %3);}";
  headerStyleSheet = headerStyleSheet.arg(HEADER_BACKGROUND_COLOR.red()).arg(
    HEADER_BACKGROUND_COLOR.green()).arg(HEADER_BACKGROUND_COLOR.blue());
  m_header->setStyleSheet(headerStyleSheet);
  headerLayout->addWidget(m_header);
  m_layout = new QBoxLayout(QBoxLayout::TopToBottom);
  m_layout->setMargin(0);
  m_layout->addLayout(headerLayout);
  m_layout->setSpacing(0);
  setLayout(m_layout);
  m_expandedConnection = m_expandButton->ConnectExpandedSignal(
    std::bind(&CollapsibleWidget::OnExpand, this));
  m_collapsedConnection = m_expandButton->ConnectCollapsedSignal(
    std::bind(&CollapsibleWidget::OnCollapse, this));
}

void CollapsibleWidget::OnCollapse() {
  Collapse(true);
}

void CollapsibleWidget::OnExpand() {
  Expand(true);
}
