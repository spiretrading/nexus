#include "Spire/Charting/ChartingWindow.hpp"
#include <climits>
#include <QApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QIntValidator>
#include <QKeyEvent>
#include <QListView>
#include <QVBoxLayout>
#include "Spire/Charting/ChartingTechnicalsPanel.hpp"
#include "Spire/Charting/ChartView.hpp"
#include "Spire/SecurityInput/SecurityInputDialog.hpp"
#include "Spire/SecurityInput/SecurityInputModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalTechnicalsModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropdownMenu.hpp"
#include "Spire/Ui/SecurityWidget.hpp"
#include "Spire/Ui/ToggleButton.hpp"
#include "Spire/Charting/TrendLineEditor.hpp"
#include "Spire/Ui/Window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Beam;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto ZOOM_FACTOR = 1.1;
}

ChartingWindow::ChartingWindow(Ref<SecurityInputModel> input_model,
    QWidget* parent)
    : Window(parent),
      m_is_mouse_dragging(false),
      m_security_widget_container(nullptr),
      m_technicals_panel(nullptr),
      m_chart(nullptr),
      m_is_chart_auto_scaled(true),
      m_trend_line_editor_widget(nullptr) {
  setMinimumSize(scale(400, 320));
  resize_body(scale(400, 320));
  set_svg_icon(":/Icons/chart-black.svg",
    ":/Icons/chart-grey.svg");
  setWindowIcon(QIcon(":/Icons/chart-icon-256x256.png"));
  setWindowTitle(tr("Chart"));
  auto body = new QWidget(this);
  body->setStyleSheet("background-color: #FFFFFF;");
  auto layout = new QVBoxLayout(body);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_button_header_widget = new QWidget(body);
  m_button_header_widget->setFixedHeight(scale_height(46));
  m_button_header_widget->setStyleSheet("background-color: #F5F5F5;");
  auto button_header_layout = new QHBoxLayout(m_button_header_widget);
  button_header_layout->setSpacing(0);
  button_header_layout->setContentsMargins(0, scale_height(10), 0,
    scale_height(10));
  button_header_layout->addSpacing(scale_width(8));
  m_period_line_edit = new QLineEdit(m_button_header_widget);
  connect(m_period_line_edit, &QLineEdit::editingFinished, this,
    &ChartingWindow::on_period_line_edit_changed);
  m_period_line_edit->setFixedSize(scale(36, 26));
  m_period_line_edit->setValidator(new QIntValidator(1, INT_MAX,
    m_period_line_edit));
  m_period_line_edit->setText("1");
  m_period_line_edit->setAlignment(Qt::AlignCenter);
  m_period_line_edit->setStyleSheet(QString(R"(
    QLineEdit {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8 %2px solid #C8C8C8;
      color: #000000;
      font-family: Roboto;
      font-size: %3px;
    }

    QLineEdit:focus {
      border: %1px solid #4B23A0 %2px solid #4B23A0;
    })").arg(scale_height(1)).arg(scale_width(1)).arg(scale_height(12)));
  button_header_layout->addWidget(m_period_line_edit);
  button_header_layout->addSpacing(scale_width(4));
  m_period_dropdown = new DropDownMenu(
    {tr("second"), tr("minute"), tr("hour")}, m_button_header_widget);
  m_period_dropdown->setFixedSize(scale(80, 26));
  button_header_layout->addWidget(m_period_dropdown);
  button_header_layout->addSpacing(scale_width(18));
  auto button_image_size = scale(16, 16);
  m_lock_grid_button = new ToggleButton(
    imageFromSvg(":/Icons/lock-grid-purple.svg", button_image_size),
    imageFromSvg(":/Icons/lock-grid-green.svg", button_image_size),
    imageFromSvg(":/Icons/lock-grid-purple.svg", button_image_size),
    imageFromSvg(":/Icons/lock-grid-grey.svg", button_image_size),
    m_button_header_widget);
  m_lock_grid_button->setFixedSize(scale(16, 26));
  m_lock_grid_button->setToolTip(tr("Lock Grid"));
  m_lock_grid_button->setDisabled(true);
  button_header_layout->addWidget(m_lock_grid_button);
  button_header_layout->addSpacing(scale_width(10));
  m_auto_scale_button = new ToggleButton(
    imageFromSvg(":/Icons/auto-scale-purple.svg", button_image_size),
    imageFromSvg(":/Icons/auto-scale-green.svg", button_image_size),
    imageFromSvg(":/Icons/auto-scale-purple.svg", button_image_size),
    imageFromSvg(":/Icons/auto-scale-grey.svg", button_image_size),
    m_button_header_widget);
  m_auto_scale_button->setFixedSize(scale(16, 26));
  m_auto_scale_button->setToolTip(tr("Auto Scale"));
  m_auto_scale_button->set_toggled(true);
  m_auto_scale_button->setDisabled(true);
  m_auto_scale_button->connect_clicked_signal([=] {
    on_auto_scale_button_click();
  });
  button_header_layout->addWidget(m_auto_scale_button);
  button_header_layout->addSpacing(scale_width(10));
  auto seperator = new QWidget(m_button_header_widget);
  seperator->setFixedSize(scale(1, 16));
  seperator->setStyleSheet("background-color: #D0D0D0;");
  button_header_layout->addWidget(seperator);
  button_header_layout->addSpacing(scale_width(10));
  m_draw_line_button = new ToggleButton(
    imageFromSvg(":/Icons/draw-purple.svg", button_image_size),
    imageFromSvg(":/Icons/draw-green.svg", button_image_size),
    imageFromSvg(":/Icons/draw-purple.svg", button_image_size),
    imageFromSvg(":/Icons/draw-grey.svg", button_image_size),
    m_button_header_widget);
  m_draw_line_button->setFixedSize(scale(16, 26));
  m_draw_line_button->setToolTip(tr("Draw Line"));
  m_draw_line_button->setDisabled(true);
  m_draw_line_button->connect_clicked_signal([=] {
    on_draw_line_button_click();
  });
  button_header_layout->addWidget(m_draw_line_button);
  button_header_layout->addStretch(1);
  layout->addWidget(m_button_header_widget);
  m_security_widget = new SecurityWidget(input_model,
    SecurityWidget::Theme::DARK, this);
  m_security_widget->connect_change_security_signal(
      [=] (const auto& security) {
    on_security_change(security);
  });
  layout->addWidget(m_security_widget);
  setTabOrder(m_period_line_edit, m_period_dropdown);
  setTabOrder(m_period_dropdown, m_lock_grid_button);
  setTabOrder(m_lock_grid_button, m_auto_scale_button);
  setTabOrder(m_auto_scale_button, m_draw_line_button);
  setTabOrder(m_draw_line_button, m_period_line_edit);
  m_security_widget->setFocus();
  Window::layout()->addWidget(body);
}

void ChartingWindow::set_models(std::shared_ptr<ChartModel> chart_model,
    std::shared_ptr<TechnicalsModel> technicals_model) {
  m_model = std::move(chart_model);
  m_technicals_model = std::move(technicals_model);
  delete m_technicals_panel;
  delete m_chart;
  delete m_security_widget_container;
  m_security_widget_container = new QWidget(this);
  m_security_widget->installEventFilter(this);
  auto container_layout = new QVBoxLayout(m_security_widget_container);
  container_layout->setContentsMargins({});
  m_technicals_panel = new ChartingTechnicalsPanel(*m_technicals_model);
  container_layout->addWidget(m_technicals_panel);
  m_chart = new ChartView(*m_model, m_security_widget_container);
  m_chart->set_auto_scale(m_is_chart_auto_scaled);
  container_layout->addWidget(m_chart);
  m_lock_grid_button->setEnabled(true);
  m_auto_scale_button->setEnabled(true);
  m_draw_line_button->setEnabled(true);
  m_draw_line_button->set_toggled(false);
  m_trend_line_editor_widget = new TrendLineEditor(m_technicals_panel);
  m_trend_line_editor_widget->hide();
  m_trend_line_editor_widget->connect_color_signal(
    [=] { on_trend_line_color_selected(); });
  m_chart->set_trend_line_color(m_trend_line_editor_widget->get_color());
  m_trend_line_editor_widget->connect_style_signal(
    [=] { on_trend_line_style_selected(); });
  m_chart->set_trend_line_style(m_trend_line_editor_widget->get_style());
  m_security_widget->set_widget(m_security_widget_container);
  m_chart->installEventFilter(this);
}

connection ChartingWindow::connect_security_change_signal(
    const ChangeSecuritySignal::slot_type& slot) const {
  return m_security_widget->connect_change_security_signal(slot);
}

bool ChartingWindow::eventFilter(QObject* object, QEvent* event) {
  if(object == m_chart) {
    if(event->type() == QEvent::MouseMove) {
      auto e = static_cast<QMouseEvent*>(event);
      if(m_is_mouse_dragging && !m_chart->is_draw_mode_enabled()) {
        auto delta = e->pos().x() - m_last_chart_mouse_pos.x();
        auto region = m_chart->get_region();
        region->m_top_left.m_x -= Scalar(delta);
        region->m_bottom_right.m_x -= Scalar(delta);
        // TODO: shift along Y
        m_chart->set_region(*region);
        m_last_chart_mouse_pos = e->pos();
      }
      m_chart->set_crosshair(e->pos(), e->buttons());
    } else if(event->type() == QEvent::MouseButtonPress) {
      auto e = static_cast<QMouseEvent*>(event);
      if(!m_is_mouse_dragging && e->button() == Qt::LeftButton) {
        m_is_mouse_dragging = true;
        m_last_chart_mouse_pos = e->pos();
      }
      m_chart->set_crosshair(e->pos(), e->buttons());
    } else if(event->type() == QEvent::MouseButtonRelease) {
      auto e = static_cast<QMouseEvent*>(event);
      if(e->button() == Qt::LeftButton) {
        m_is_mouse_dragging = false;
      }
      m_chart->set_crosshair(e->pos(), e->buttons());
    } else if(event->type() == QEvent::Wheel) {
      auto e = static_cast<QWheelEvent*>(event);
      auto region = m_chart->get_region();
      auto old_width = region->m_bottom_right.m_x - region->m_top_left.m_x;
      auto new_width = [&] {
        if(e->angleDelta().y() < 0) {
          return ZOOM_FACTOR * old_width;
        } else {
          return old_width / ZOOM_FACTOR;
        }
      }();
      auto width_change = (new_width - old_width) / 2;
      region->m_top_left.m_x -= width_change;
      region->m_bottom_right.m_x += width_change;
      m_chart->set_region(*region);
    } else if(event->type() == QEvent::HoverLeave) {
      m_chart->reset_crosshair();
    } else if(event->type() == QEvent::HoverEnter) {
      auto e = static_cast<QHoverEvent*>(event);
      m_chart->set_crosshair(e->pos(), Qt::NoButton);
    }
  } else if(object == m_security_widget) {
   if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Delete) {
        m_chart->remove_selected_trend_lines();
      } else if(e->key() == Qt::Key_Shift) {
        m_chart->set_multi_select(true);
      }
    } else if(event->type() == QEvent::KeyRelease) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Shift) {
        m_chart->set_multi_select(false);
      }
    }
  }
  return QWidget::eventFilter(object, event);
}

void ChartingWindow::keyPressEvent(QKeyEvent* event) {
  QApplication::sendEvent(m_security_widget, event);
}

void ChartingWindow::on_auto_scale_button_click() {
  m_is_chart_auto_scaled = !m_is_chart_auto_scaled;
  if(m_chart != nullptr) {
    m_chart->set_auto_scale(m_is_chart_auto_scaled);
  }
}

void ChartingWindow::on_draw_line_button_click() {
  m_trend_line_editor_widget->setVisible(
    !m_trend_line_editor_widget->isVisible());
  m_chart->set_draw_mode(m_trend_line_editor_widget->isVisible());
}

void ChartingWindow::on_period_line_edit_changed() {
  if(m_period_line_edit->text().toInt() == 1) {
    m_period_dropdown->set_items({tr("second"), tr("minute"), tr("hour")});
  } else {
    m_period_dropdown->set_items({tr("seconds"), tr("minutes"), tr("hours")});
  }
}

void ChartingWindow::on_security_change(const Security& security) {
  setWindowTitle(CustomVariantItemDelegate().displayText(
    QVariant::fromValue(security), QLocale()) + QObject::tr(" - Chart"));
}

void ChartingWindow::on_trend_line_color_selected() {
  m_chart->set_trend_line_color(m_trend_line_editor_widget->get_color());
}

void ChartingWindow::on_trend_line_style_selected() {
  m_chart->set_trend_line_style(m_trend_line_editor_widget->get_style());
}
