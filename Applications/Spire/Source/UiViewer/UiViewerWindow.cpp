#include "Spire/UiViewer/UiViewerWindow.hpp"
#include <QGridLayout>
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
  setMinimumSize(scale(500, 300));
  setWindowTitle(tr("UI Viewer"));
  set_svg_icon(":/Icons/spire-icon-black.svg", ":/Icons/spire-icon-grey.svg");
  setWindowIcon(QIcon(":/Icons/spire-icon-256x256.png"));
  auto body = new QWidget(this);
  body->setStyleSheet("background-color: #F5F5F5");
  layout()->addWidget(body);
  m_layout = new QHBoxLayout(body);
  m_widget_list = new QListWidget(this);
  m_widget_list->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  m_widget_list->setStyleSheet(QString(R"(
    QListWidget {
      background-color: white;
      border: 1px solid #A0A0A0;
      outline: none;
      padding: %1px %2px 0px %2px;
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
  m_layout->addWidget(m_widget_list);
  initialize_color_selector_button();
  m_widgets[m_widget_list->item(0)->text()]->show();
}

void UiViewerWindow::add_widget(const QString& name,
    QWidget* container_widget) {
  m_widgets.insert(name, container_widget);
  m_layout->addWidget(container_widget);
  m_widget_list->addItem(name);
  container_widget->hide();
}

void UiViewerWindow::initialize_color_selector_button() {
  auto container_widget = new QWidget(this);
  auto layout = new QGridLayout(container_widget);
  m_color_selector_button = new ColorSelectorButton(QColor("#4B23A0"),
    this);
  m_color_selector_button->setFixedSize(CONTROL_SIZE());
  layout->addWidget(m_color_selector_button, 0, 0);
  auto color_selector_button_value = new QLabel(
    m_color_selector_button->get_color().name().toUpper(), this);
  layout->addWidget(color_selector_button_value, 0, 1);
  auto set_color_input = new TextInputWidget(this);
  set_color_input->setFixedSize(CONTROL_SIZE());
  layout->addWidget(set_color_input, 1, 0);
  auto set_color_button = create_control_button(tr("Set Color"), this);
  set_color_button->setFixedSize(CONTROL_SIZE());
  layout->addWidget(set_color_button, 1, 1);
  auto create_color_input = new TextInputWidget(this);
  create_color_input->setFixedSize(CONTROL_SIZE());
  layout->addWidget(create_color_input, 2, 0);
  auto create_color_button = create_control_button(tr("Replace Button"), this);
  create_color_button->setFixedSize(CONTROL_SIZE());
  layout->addWidget(create_color_button, 2, 1);
  m_color_selector_button->connect_color_signal([=] (const auto& color) {
    color_selector_button_value->setText(color.name().toUpper());
  });
  connect(set_color_input, &TextInputWidget::editingFinished, [=] {
    on_set_color_button_color(set_color_input->text());
  });
  set_color_button->connect_clicked_signal([=] {
    on_set_color_button_color(set_color_input->text());
  });
  connect(create_color_input, &TextInputWidget::editingFinished, [=] {
    on_create_color_button_color(create_color_input->text(), layout);
  });
  create_color_button->connect_clicked_signal([=] {
    on_create_color_button_color(set_color_input->text(), layout);
  });
  add_widget(tr("ColorSelectorButton"), container_widget);
}

void UiViewerWindow::on_create_color_button_color(const QString& color_hex,
    QGridLayout* layout) {
  auto color = QColor(QString("#%1").arg(color_hex));
  if(color.isValid()) {
    delete_later(m_color_selector_button);
    m_color_selector_button = new ColorSelectorButton(color, this);
    m_color_selector_button->setFixedSize(CONTROL_SIZE());
    layout->addWidget(m_color_selector_button, 0, 0);
  }
}

void UiViewerWindow::on_set_color_button_color(const QString& color_hex) {
  auto color = QColor(QString("#%1").arg(color_hex));
  if(color.isValid()) {
    m_color_selector_button->set_color(color);
  }
}
