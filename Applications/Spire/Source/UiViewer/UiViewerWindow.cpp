#include "Spire/UiViewer/UiViewerWindow.hpp"
#include <QGridLayout>
#include <QLabel>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/UiViewer/ColorSelectorButtonTestWidget.hpp"
#include "Spire/UiViewer/FlatButtonTestWidget.hpp"

using namespace Spire;

UiViewerWindow::UiViewerWindow(QWidget* parent)
    : Window(parent) {
  setMinimumSize(scale(500, 300));
  setWindowTitle(tr("UI Viewer"));
  set_svg_icon(":/Icons/spire-icon-black.svg", ":/Icons/spire-icon-grey.svg");
  setWindowIcon(QIcon(":/Icons/spire-icon-256x256.png"));
  auto body = new QWidget(this);
  body->setStyleSheet("background-color: #F5F5F5");
  layout()->addWidget(body);
  m_layout = new QHBoxLayout(body);
  m_widget_list = new QListWidget(this);
  m_widget_list->setSelectionMode(QAbstractItemView::SingleSelection);
  m_widget_list->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  m_widget_list->setStyleSheet(QString(R"(
    QListWidget {
      background-color: white;
      border: 1px solid #A0A0A0;
      outline: none;
      padding: %1px %2px 0px %2px;
    }

    QListWidget:focus {
      border: 1px solid #4B23A0;
    }

    QListWidget::item {
      padding-top: %5px;
      padding-bottom: %5px;
    }

    QListWidget::item:selected {
      border: %3px solid #4B23A0 %4px solid #4B23A0;
      color: #000000;
    })").arg(scale_height(4)).arg(scale_width(4))
        .arg(scale_height(1)).arg(scale_width(1))
        .arg(scale_height(3)));
  connect(m_widget_list, &QListWidget::currentItemChanged, this,
    &UiViewerWindow::on_item_selected);
  m_layout->addWidget(m_widget_list);
  add_test_widget(tr("ColorSelectorButton"),
    new ColorSelectorButtonTestWidget(this));
  add_test_widget(tr("FlatButton"), new FlatButtonTestWidget(this));
  m_widget_list->setCurrentRow(0);
}

void UiViewerWindow::add_test_widget(const QString& name, QWidget* widget) {
  m_widgets.insert(name, widget);
  m_layout->addWidget(widget);
  m_widget_list->addItem(name);
  widget->hide();
}

void UiViewerWindow::on_item_selected(const QListWidgetItem* current,
    const QListWidgetItem* previous) {
  if(previous) {
    m_widgets[previous->text()]->hide();
  }
  m_widgets[current->text()]->show();
}
