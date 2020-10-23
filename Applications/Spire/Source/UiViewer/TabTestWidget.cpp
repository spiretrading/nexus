#include "Spire/UiViewer/TabTestWidget.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include "Spire/Ui/TabWidget.hpp"

using namespace Spire;

TabTestWidget::TabTestWidget(QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  auto tab_widget = new TabWidget(this);
  layout->addWidget(tab_widget);
  auto tab1_label = new QLabel(tr("Tab 1"), this);
  tab1_label->setAlignment(Qt::AlignCenter);
  tab1_label->setFocusPolicy(Qt::StrongFocus);
  auto style = QString(R"(
    QLabel {
      background-color: white;
      font-size: 20pt;
    }
    QLabel:focus {
      border: 2px solid #4B23A0;
    })");
  tab1_label->setStyleSheet(style);
  tab_widget->addTab(tab1_label, tr("Tab 1"));
  auto tab2_label = new QLabel(tr("Tab 2"), this);
  tab2_label->setAlignment(Qt::AlignCenter);
  tab2_label->setFocusPolicy(Qt::StrongFocus);
  tab2_label->setStyleSheet(style);
  tab_widget->addTab(tab2_label, tr("Tab 2"));
  auto tab3_label = new QLabel(tr("Tab 3"), this);
  tab3_label->setAlignment(Qt::AlignCenter);
  tab3_label->setFocusPolicy(Qt::StrongFocus);
  tab3_label->setStyleSheet(style);
  tab_widget->addTab(tab3_label, tr("Tab 3"));
}
