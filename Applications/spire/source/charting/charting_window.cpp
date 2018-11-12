#include "spire/charting/charting_window.hpp"
#include <climits>
#include <QHBoxLayout>
#include <QIcon>
#include <QIntValidator>
#include <QListView>
#include <QVBoxLayout>
#include "spire/security_input/security_input_model.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/dropdown_menu.hpp"
#include "spire/ui/toggle_button.hpp"
#include "spire/ui/window.hpp"

using namespace Beam;
using namespace Spire;

ChartingWindow::ChartingWindow(Ref<SecurityInputModel> input_model,
    QWidget* parent)
    : QWidget(parent),
      m_input_model(input_model.Get()) {
  m_body = new QWidget(this);
  m_body->setMinimumSize(scale(400, 320));
  m_body->resize(scale(400, 320));
  m_body->setStyleSheet("background-color: #FFFFFF;");
  auto window_layout = new QHBoxLayout(this);
  window_layout->setContentsMargins({});
  auto window = new Window(m_body, this);
  setWindowTitle(tr("Chart"));
  window->set_svg_icon(":/icons/chart-black.svg",
    ":/icons/chart-grey.svg");
  setWindowIcon(QIcon(":/icons/chart-icon-256x256.png"));
  window_layout->addWidget(window);
  auto layout = new QVBoxLayout(m_body);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_button_header_widget = new QWidget(m_body);
  m_button_header_widget->setFixedHeight(scale_height(46));
  m_button_header_widget->setStyleSheet("background-color: #F5F5F5;");
  auto button_header_layout = new QHBoxLayout(m_button_header_widget);
  button_header_layout->setSpacing(0);
  button_header_layout->setContentsMargins(0, scale_height(10), 0,
    scale_height(10));
  button_header_layout->addSpacing(scale_width(8));
  m_period_line_edit = new QLineEdit(m_button_header_widget);
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
  m_period_dropdown = new DropdownMenu(m_button_header_widget);
  m_period_dropdown->addItem(tr("seconds"));
  m_period_dropdown->addItem(tr("minutes"));
  m_period_dropdown->addItem(tr("hours"));
  m_period_dropdown->setFixedSize(scale(80, 26));
  button_header_layout->addWidget(m_period_dropdown);
  button_header_layout->addSpacing(scale_width(18));
  auto button_size = scale(16, 16);
  auto lock_grid_button = new ToggleButton(
    imageFromSvg(":/icons/lock-grid-purple.svg", button_size),
    imageFromSvg(":/icons/lock-grid-green.svg", button_size),
    imageFromSvg(":/icons/lock-grid-purple.svg", button_size),
    imageFromSvg(":/icons/lock-grid-grey.svg", button_size),
    m_button_header_widget);
  lock_grid_button->setFixedSize(button_size);
  button_header_layout->addWidget(lock_grid_button);
  button_header_layout->addSpacing(scale_width(10));
  auto auto_scale_button = new ToggleButton(
    imageFromSvg(":/icons/auto-scale-purple.svg", button_size),
    imageFromSvg(":/icons/auto-scale-green.svg", button_size),
    imageFromSvg(":/icons/auto-scale-purple.svg", button_size),
    imageFromSvg(":/icons/auto-scale-grey.svg", button_size),
    m_button_header_widget);
  button_header_layout->addWidget(auto_scale_button);
  button_header_layout->addSpacing(scale_width(10));
  auto seperator = new QWidget(m_button_header_widget);
  seperator->setFixedSize(scale(1, 16));
  seperator->setStyleSheet("background-color: #D0D0D0;");
  button_header_layout->addWidget(seperator);
  button_header_layout->addSpacing(scale_width(10));
  auto draw_line_button = new ToggleButton(
    imageFromSvg(":/icons/draw-purple.svg", button_size),
    imageFromSvg(":/icons/draw-green.svg", button_size),
    imageFromSvg(":/icons/draw-purple.svg", button_size),
    imageFromSvg(":/icons/draw-grey.svg", button_size),
    m_button_header_widget);
  button_header_layout->addWidget(draw_line_button);
  button_header_layout->addStretch(1);
  layout->addWidget(m_button_header_widget);
  layout->addStretch(1);
}
