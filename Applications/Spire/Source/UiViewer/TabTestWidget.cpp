#include "Spire/UiViewer/TabTestWidget.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include "Spire/Ui/TabWidget.hpp"

using namespace Spire;

namespace {
  void add_tab(const QString& label, TabWidget& tab_widget, QWidget& parent) {
  auto tab_label = new QLabel(label, &parent);
  tab_label->setAlignment(Qt::AlignCenter);
  tab_label->setFocusPolicy(Qt::StrongFocus);
  tab_label->setStyleSheet(R"(
    QLabel {
      background-color: white;
      font-size: 20pt;
    }
    QLabel:focus {
      border: 2px solid #4B23A0;
    })");
  tab_widget.addTab(tab_label, label);
  }
}

TabTestWidget::TabTestWidget(QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  auto tab_widget = new TabWidget(this);
  layout->addWidget(tab_widget);
  add_tab(tr("Tab 1"), *tab_widget, *this);
  add_tab(tr("Tab 2"), *tab_widget, *this);
  add_tab(tr("Tab 3"), *tab_widget, *this);
}
