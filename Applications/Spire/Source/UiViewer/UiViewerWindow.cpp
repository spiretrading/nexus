#include "Spire/UiViewer/UiViewerWindow.hpp"
#include <QLabel>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/Ui/TextInputWidget.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Spire;

namespace {
  auto CONTROL_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }

  auto create_control_button(const QString& label, QWidget* parent) {
    auto button = new FlatButton(label, parent);
    button->setFixedSize(CONTROL_SIZE());
    auto style = button->get_style();
    style.m_background_color = QColor("#F8F8F8");
    style.m_border_color = QColor("#C8C8C8");
    button->set_style(style);
    style.m_border_color = QColor("#4B23A0");
    button->set_hover_style(style);
    button->set_focus_style(style);
    return button;
  }
}

UiViewerWindow::UiViewerWindow(QWidget* parent)
    : Window(parent) {
  setMinimumSize(scale(600, 400));
  setWindowTitle(tr("UI Viewer"));
  set_svg_icon(":/Icons/spire-icon-black.svg", ":/Icons/spire-icon-grey.svg");
  setWindowIcon(QIcon(":/Icons/spire-icon-256x256.png"));
  auto body = new QWidget(this);
  body->setStyleSheet("background-color: #F5F5F5");
  layout()->addWidget(body);
  m_layout = new QGridLayout(body);
  auto current_row = 0;
  add_color_selector_button(current_row++);
}

void UiViewerWindow::add_color_selector_button(int row) {
  auto color_selector_button_label = new QLabel(tr("ColorSelectorButton"));
  m_layout->addWidget(color_selector_button_label, row, 0, Qt::AlignCenter);
  m_color_selector_button = new ColorSelectorButton(QColor("#4B23A0"),
    this);
  m_color_selector_button->setFixedSize(CONTROL_SIZE());
  m_layout->addWidget(m_color_selector_button, row, 1, Qt::AlignCenter);
  auto controls_widget = new QWidget(this);
  controls_widget->setFixedWidth(scale_width(224));
  m_layout->addWidget(controls_widget, row, 2, Qt::AlignCenter);
  auto controls_layout = new QGridLayout(controls_widget);
  auto set_color_input = new TextInputWidget(controls_widget);
  set_color_input->setFixedSize(CONTROL_SIZE());
  controls_layout->addWidget(set_color_input, 0, 0);
  auto set_color_button = create_control_button(tr("Set Color"),
    controls_widget);
  set_color_button->setFixedSize(CONTROL_SIZE());
  controls_layout->addWidget(set_color_button, 0, 1);
  auto create_color_input = new TextInputWidget(controls_widget);
  create_color_input->setFixedSize(CONTROL_SIZE());
  controls_layout->addWidget(create_color_input, 1, 0);
  auto create_color_button = create_control_button(tr("Create Button"),
    controls_widget);
  controls_layout->addWidget(create_color_button, 1, 1);
  auto color_selector_button_value = new QLabel(
    m_color_selector_button->get_color().name().toUpper(), this);
  m_color_selector_button->connect_color_signal([=] (const auto& color) {
    color_selector_button_value->setText(color.name().toUpper());
  });
  set_color_button->connect_clicked_signal([=] {
    auto color = QColor(QString("#%1").arg(set_color_input->text()));
    if(color.isValid()) {
      m_color_selector_button->set_color(color);
    }
  });
  create_color_button->connect_clicked_signal([=, row = row] {
    auto color = QColor(QString("#%1").arg(create_color_input->text()));
    if(color.isValid()) {
      m_layout->removeWidget(m_color_selector_button);
      delete_later(m_color_selector_button);
      m_color_selector_button = new ColorSelectorButton(QColor(
        QString("#%1").arg(create_color_input->text())), this);
      m_color_selector_button->setFixedSize(CONTROL_SIZE());
      m_layout->addWidget(m_color_selector_button, row, 1, Qt::AlignCenter);
      update();
    }
  });
  m_layout->addWidget(color_selector_button_value, row, 3, Qt::AlignCenter);
}
