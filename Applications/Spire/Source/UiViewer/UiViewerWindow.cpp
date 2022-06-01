#include "Spire/UiViewer/UiViewerWindow.hpp"
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSplitter>
#include <QTextEdit>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/HoverObserver.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/UiViewer/StandardUiProfiles.hpp"
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
    return widget_list;
  }

  auto make_table(const UiProfile& profile, QPushButton* reset_button,
      QPushButton* rebuild_button) {
    auto container = new QWidget();
    auto layout = new QVBoxLayout(container);
    auto table = new UiPropertyTableView(profile.get_properties());
    layout->addWidget(table);
    auto button_layout = new QHBoxLayout();
    button_layout->addWidget(reset_button);
    button_layout->addWidget(rebuild_button);
    layout->addLayout(button_layout);
    return container;
  }

  auto to_string(HoverObserver::State state) {
    switch(state) {
      case HoverObserver::State::MOUSE_IN:
        return QString::fromUtf8("MOUSE_IN");
      case HoverObserver::State::MOUSE_OVER:
        return QString::fromUtf8("MOUSE_OVER");
    }
    return QString::fromUtf8("NONE");
  }

  struct SizeAdjustedContainer : QWidget {
    QWidget* m_body;

    SizeAdjustedContainer(QWidget* body)
        : m_body(body) {
      enclose(*this, *m_body);
    }

    bool event(QEvent* event) override {
      if(event->type() == QEvent::LayoutRequest) {
        adjustSize();
      }
      return QWidget::event(event);
    }
  };
}

UiViewerWindow::UiViewerWindow(QWidget* parent)
    : Window(parent),
      m_line_count(0) {
  setWindowTitle(tr("UI Viewer"));
  set_svg_icon(":/Icons/spire.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/spire.png"));
  resize(scale(775, 580));
  m_body = new QSplitter(Qt::Horizontal, this);
  m_body->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
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
  m_reset_button = new QPushButton(QString::fromUtf8("Reset"));
  connect(m_reset_button, &QPushButton::pressed, [this] { on_reset(); });
  m_rebuild_button = new QPushButton(QString::fromUtf8("Rebuild"));
  connect(m_rebuild_button, &QPushButton::pressed, [this] { on_rebuild(); });
  add(make_box_profile());
  add(make_calendar_date_picker_profile());
  add(make_check_box_profile());
  add(make_closed_filter_panel_profile());
  add(make_combo_box_profile());
  add(make_context_menu_profile());
  add(make_date_box_profile());
  add(make_date_filter_panel_profile());
  add(make_decimal_box_profile());
  add(make_decimal_filter_panel_profile());
  add(make_delete_icon_button_profile());
  add(make_destination_box_profile());
  add(make_destination_list_item_profile());
  add(make_drop_down_box_profile());
  add(make_drop_down_list_profile());
  add(make_duration_box_profile());
  add(make_duration_filter_panel_profile());
  add(make_filter_panel_profile());
  add(make_focus_observer_profile());
  add(make_hover_observer_profile());
  add(make_icon_button_profile());
  add(make_icon_toggle_button_profile());
  add(make_info_tip_profile());
  add(make_input_box_profile());
  add(make_integer_box_profile());
  add(make_integer_filter_panel_profile());
  add(make_key_filter_panel_profile());
  add(make_key_input_box_profile());
  add(make_key_tag_profile());
  add(make_label_button_profile());
  add(make_label_profile());
  add(make_list_item_profile());
  add(make_list_view_profile());
  add(make_market_box_profile());
  add(make_money_box_profile());
  add(make_money_filter_panel_profile());
  add(make_navigation_view_profile());
  add(make_open_filter_panel_profile());
  add(make_order_field_info_tip_profile());
  add(make_order_type_box_profile());
  add(make_order_type_filter_panel_profile());
  add(make_overlay_panel_profile());
  add(make_quantity_box_profile());
  add(make_quantity_filter_panel_profile());
  add(make_radio_button_profile());
  add(make_region_list_item_profile());
  add(make_responsive_label_profile());
  add(make_scroll_bar_profile());
  add(make_scroll_box_profile());
  add(make_scrollable_list_box_profile());
  add(make_search_box_profile());
  add(make_security_box_profile());
  add(make_security_filter_panel_profile());
  add(make_security_list_item_profile());
  add(make_security_view_profile());
  add(make_side_box_profile());
  add(make_side_filter_panel_profile());
  add(make_split_view_profile());
  add(make_tab_view_profile());
  add(make_table_header_profile());
  add(make_table_header_item_profile());
  add(make_table_view_profile());
  add(make_tag_profile());
  add(make_tag_box_profile());
  add(make_text_area_box_profile());
  add(make_text_box_profile());
  add(make_time_box_profile());
  add(make_time_in_force_box_profile());
  add(make_time_in_force_filter_panel_profile());
  add(make_tooltip_profile());
  add(make_transition_view_profile());
  m_widget_list->setCurrentRow(0);
}

void UiViewerWindow::add(UiProfile profile) {
  auto name = profile.get_name();
  m_widget_list->addItem(name);
  m_profiles.insert(std::pair(std::move(name), std::move(profile)));
}

void UiViewerWindow::update_table(const UiProfile& profile) {
  auto table = make_table(profile, m_reset_button, m_rebuild_button);
  auto previous_table = m_body->replaceWidget(PROPERTIES_INDEX, table);
  delete previous_table;
  table->show();
}

void UiViewerWindow::on_event(
    const QString& name, const std::vector<std::any>& arguments) {
  ++m_line_count;
  auto log = QString();
  log += QString::number(m_line_count) + ": " + name;
  if(!arguments.empty()) {
    log += "(";
    auto prepend_comma = false;
    for(auto& argument : arguments) {
      if(prepend_comma) {
        log += ", ";
      } else {
        prepend_comma = true;
      }
      if(auto value = std::any_cast<std::nullptr_t>(&argument)) {
        log += QString::fromUtf8("null");
      } else if(auto value = std::any_cast<HoverObserver::State>(&argument)) {
        log += to_string(*value);
      } else {
        log += displayText(argument);
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
  update_table(profile);
  m_stage = new QSplitter(Qt::Vertical);
  m_center_stage = new QScrollArea();
  m_center_stage->setWidget(new SizeAdjustedContainer(profile.get_widget()));
  m_center_stage->setAlignment(Qt::AlignCenter);
  m_stage->addWidget(m_center_stage);
  m_event_log = new QTextEdit();
  m_event_log->setReadOnly(true);
  m_stage->addWidget(m_event_log);
  m_stage->setSizes({350, 150});
  auto previous_stage = m_body->replaceWidget(STAGE_INDEX, m_stage);
  delete previous_stage;
  profile.connect_event_signal(
    [this] (const auto& name, const auto& arguments) {
      on_event(name, arguments);
    });
  m_line_count = 0;
  m_stage->show();
}

void UiViewerWindow::on_reset() {
  on_item_selected(m_widget_list->currentItem(), m_widget_list->currentItem());
}

void UiViewerWindow::on_rebuild() {
  auto& profile = m_profiles.at(m_widget_list->currentItem()->text());
  profile.remove_widget();
  update_table(profile);
  auto previous_widget = m_center_stage->takeWidget();
  m_center_stage->setWidget(new SizeAdjustedContainer(profile.get_widget()));
  delete previous_widget;
  m_stage->replaceWidget(0, new QWidget(this));
  auto previous_stage = m_stage->replaceWidget(0, m_center_stage);
  delete previous_stage;
}
