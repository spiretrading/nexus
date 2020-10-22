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
  auto widget1 = new QLabel(tr("Tab 1"), this);
  widget1->setAlignment(Qt::AlignCenter);
  widget1->setFocusPolicy(Qt::StrongFocus);
  auto style = QString(R"(
    QLabel {
      background-color: white;
      font-size: 20pt;
    }
    QLabel:focus {
      border: 2px solid #4B23A0;
    })");
  widget1->setStyleSheet(style);
  tab_widget->addTab(widget1, tr("Tab 1"));
  auto widget2 = new QLabel(tr("Tab 2"), this);
  widget2->setAlignment(Qt::AlignCenter);
  widget2->setFocusPolicy(Qt::StrongFocus);
  widget2->setStyleSheet(style);
  tab_widget->addTab(widget2, tr("Tab 2"));
  auto widget3 = new QLabel(tr("Tab 3"), this);
  widget3->setAlignment(Qt::AlignCenter);
  widget3->setFocusPolicy(Qt::StrongFocus);
  widget3->setStyleSheet(style);
  tab_widget->addTab(widget3, tr("Tab 3"));
}
