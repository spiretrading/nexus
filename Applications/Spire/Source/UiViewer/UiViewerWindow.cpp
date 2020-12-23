#include "Spire/UiViewer/UiViewerWindow.hpp"
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSplitter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/Ui/ScrollBarStyle.hpp"
#include "Spire/UiViewer/StandardUiProperty.hpp"
#include "Spire/UiViewer/UiProfile.hpp"
#include "Spire/UiViewer/UiPropertyTableView.hpp"

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

  auto make_flat_button_profile() {
    auto enabled_property = make_standard_bool_property("enabled", true);
    auto width_property = make_standard_int_property("width",
      scale_width(100));
    auto height_property = make_standard_int_property("height",
      scale_height(26));
    auto label_property = make_standard_qstring_property("label",
      QString::fromUtf8("Click me!"));
    auto profile = UiProfile(QString::fromUtf8("FlatButton"),
      {enabled_property, width_property, height_property, label_property},
      [] (const UiProfile& profile) {
        auto& enabled = get<bool>("enabled", profile.get_properties());
        auto& width = get<int>("width", profile.get_properties());
        auto& height = get<int>("height", profile.get_properties());
        auto& label = get<QString>("label", profile.get_properties());
        auto button = make_flat_button(label.get());
        button->setEnabled(enabled.get());
        button->setFixedSize(width.get(), height.get());
        enabled.connect_changed_signal([=] (bool value) {
          button->setEnabled(value);
        });
        width.connect_changed_signal([=] (int value) {
          button->setFixedSize(value, button->height());
        });
        height.connect_changed_signal([=] (int value) {
          button->setFixedSize(button->width(), value);
        });
        label.connect_changed_signal([=] (const QString& value) {
          button->set_label(value);
        });
        return button;
      });
    return profile;
  }
}

UiViewerWindow::UiViewerWindow(QWidget* parent)
    : Window(parent) {
  setWindowTitle(tr("UI Viewer"));
  set_svg_icon(":/Icons/spire.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/spire.png"));
  resize(scale(1000, 800));
  m_body = new QSplitter(Qt::Horizontal, this);
  m_body->setObjectName("ui_viewer_body");
  m_body->setStyleSheet("#ui_viewer_body { background-color: #F5F5F5; }");
  layout()->addWidget(m_body);
  m_widget_list = make_widget_list(this);
  connect(m_widget_list, &QListWidget::currentItemChanged, this,
    &UiViewerWindow::on_item_selected);
  m_body->addWidget(m_widget_list);
  m_body->addWidget(new QWidget(this));
  m_body->addWidget(new QWidget(this));
  m_body->setSizes({scale_width(250), scale_width(500), scale_width(250)});
  add(make_flat_button_profile());
  m_widget_list->setCurrentRow(0);
}

void UiViewerWindow::add(UiProfile profile) {
  auto name = profile.get_name();
  m_widget_list->addItem(name);
  m_profiles.insert(std::pair(std::move(name), std::move(profile)));
}

void UiViewerWindow::on_item_selected(const QListWidgetItem* current,
    const QListWidgetItem* previous) {
  auto& profile = m_profiles.at(current->text());
  auto table = new UiPropertyTableView(profile.get_properties(), this);
  auto previous_properties = m_body->replaceWidget(PROPERTIES_INDEX, table);
  delete previous_properties;
  table->show();
  auto stage = new QScrollArea(this);
  stage->setAlignment(Qt::AlignCenter);
  stage->setWidget(profile.reset());
  auto previous_stage = m_body->replaceWidget(STAGE_INDEX, stage);
  delete previous_stage;
  stage->show();
}
