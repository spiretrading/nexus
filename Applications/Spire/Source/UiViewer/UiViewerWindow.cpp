#include "Spire/UiViewer/UiViewerWindow.hpp"
#include <QGridLayout>
#include <QLabel>
#include <QSplitter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/ScrollBarStyle.hpp"
#include "Spire/UiViewer/IntUiProperty.hpp"
#include "Spire/UiViewer/UiPropertyTableView.hpp"

using namespace Spire;

namespace {
  QListWidget* make_widget_list(UiViewerWindow* window) {
    auto widget_list = new QListWidget(window);
    widget_list->setSelectionMode(QAbstractItemView::SingleSelection);
    widget_list->setStyleSheet(QString(R"(
      QListWidget {
        background-color: white;
        border: 1px solid #A0A0A0;
        outline: none;
        padding: %1px 0px %1px %2px;
      }
      QListWidget:focus {
        border: 1px solid #4B23A0;
      }
      QListWidget::item {
        margin-right: %2px;
        padding-top: %5px;
        padding-bottom: %5px;
      }
      QListWidget::item:selected {
        border: %3px solid #4B23A0 %4px solid #4B23A0;
        color: #000000;
        padding-left: -%4px;
      })").arg(scale_height(4)).arg(scale_width(4))
        .arg(scale_height(1)).arg(scale_width(1))
        .arg(scale_height(3)));
    widget_list->verticalScrollBar()->setStyle(new ScrollBarStyle(window));
    return widget_list;
  }
}

UiViewerWindow::UiViewerWindow(QWidget* parent)
    : Window(parent) {
  setWindowTitle(tr("UI Viewer"));
  set_svg_icon(":/Icons/spire.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/spire.png"));
  resize(scale(1400, 1080));
  auto body = new QSplitter(Qt::Horizontal, this);
  body->setObjectName("ui_viewer_body");
  body->setStyleSheet("#ui_viewer_body { background-color: #F5F5F5; }");
  layout()->addWidget(body);
  m_widget_list = make_widget_list(this);
  connect(m_widget_list, &QListWidget::currentItemChanged, this,
    &UiViewerWindow::on_item_selected);
  body->addWidget(m_widget_list);
  auto stage = new QWidget(this);
  body->addWidget(stage);
  auto properties = std::vector<std::unique_ptr<UiProperty>>();
  properties.push_back(std::make_unique<IntUiProperty>(tr("P1")));
  properties.push_back(std::make_unique<IntUiProperty>(tr("P2")));
  properties.push_back(std::make_unique<IntUiProperty>(tr("P3")));
  auto table = new UiPropertyTableView(std::move(properties), this);
  table->setDisabled(true);
  body->addWidget(table);
  body->setSizes({scale_width(250), scale_width(900), scale_width(250)});
}

void UiViewerWindow::on_item_selected(const QListWidgetItem* current,
  const QListWidgetItem* previous) {}
