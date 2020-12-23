#include "Spire/UiViewer/UiViewerWindow.hpp"
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSplitter>
#include <QTextEdit>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ScrollBarStyle.hpp"
#include "Spire/UiViewer/StandardUiProfiles.hpp"
#include "Spire/UiViewer/UiProfile.hpp"
#include "Spire/UiViewer/UiPropertyTableView.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Spire;

namespace {
  const auto LIST_INDEX = 0;
  const auto STAGE_INDEX = 1;
  const auto PROPERTIES_INDEX = 2;

  auto make_widget_list(QWidget* parent) {
    auto widget_list = new QListWidget(parent);
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
    widget_list->verticalScrollBar()->setStyle(new ScrollBarStyle(widget_list));
    return widget_list;
  }
}

UiViewerWindow::UiViewerWindow(QWidget* parent)
    : Window(parent),
      m_line_count(0) {
  setWindowTitle(tr("UI Viewer"));
  set_svg_icon(":/Icons/spire.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/spire.png"));
  resize(scale(775, 500));
  m_body = new QSplitter(Qt::Horizontal, this);
  m_body->setObjectName("ui_viewer_body");
  m_body->setStyleSheet("#ui_viewer_body { background-color: #F5F5F5; }");
  m_body->setContentsMargins(scale_width(6), scale_height(6), scale_width(6),
    scale_height(6));
  layout()->addWidget(m_body);
  m_widget_list = make_widget_list(this);
  connect(m_widget_list, &QListWidget::currentItemChanged, this,
    &UiViewerWindow::on_item_selected);
  m_body->addWidget(m_widget_list);
  m_body->addWidget(new QWidget(this));
  m_body->addWidget(new QWidget(this));
  m_body->setSizes({scale_width(150), scale_width(375), scale_width(250)});
  add(make_flat_button_profile());
  m_widget_list->setCurrentRow(0);
}

void UiViewerWindow::add(UiProfile profile) {
  auto name = profile.get_name();
  m_widget_list->addItem(name);
  m_profiles.insert(std::pair(std::move(name), std::move(profile)));
}

void UiViewerWindow::on_event(const QString& name,
    const std::vector<std::any>& arguments) {
  ++m_line_count;
  auto log = QString();
  log += QString::number(m_line_count) + ": " + name;
  if(!arguments.empty()) {
    log += "(";
    auto prepend_comma = false;
    for(auto& argument : arguments) {
      if(prepend_comma) {
        log += ", ";
      }
    }
    log += ")";
  }
  m_event_log->append(log);
}

void UiViewerWindow::on_item_selected(const QListWidgetItem* current,
    const QListWidgetItem* previous) {
  if(previous) {
    auto& profile = m_profiles.at(previous->text());
    profile.reset();
  }
  auto& profile = m_profiles.at(current->text());
  auto table = new UiPropertyTableView(profile.get_properties(), this);
  auto previous_properties = m_body->replaceWidget(PROPERTIES_INDEX, table);
  delete previous_properties;
  table->show();
  auto stage = new QSplitter(Qt::Vertical, this);
  auto center_stage = new QScrollArea(this);
  center_stage->setAlignment(Qt::AlignCenter);
  center_stage->setWidget(profile.get_widget());
  stage->addWidget(center_stage);
  m_event_log = new QTextEdit(this);
  m_event_log->setReadOnly(true);
  stage->addWidget(m_event_log);
  stage->setSizes({350, 150});
  auto previous_stage = m_body->replaceWidget(STAGE_INDEX, stage);
  delete previous_stage;
  profile.connect_event_signal(
    [this] (const QString& name, const std::vector<std::any>& arguments) {
      on_event(name, arguments);
    });
  m_line_count = 0;
  stage->show();
}
