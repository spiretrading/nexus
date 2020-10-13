#include "Spire/UiViewer/FlatButtonTestWidget.hpp"
#include <QFont>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Spire;

namespace {
  auto BUTTON_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }

  auto make_labeled_button(const QString& label, QWidget* parent) {
    auto button = new FlatButton(label, parent);
    button->setFixedSize(BUTTON_SIZE());
    auto button_style = button->get_style();
    button_style.m_background_color = QColor("#EBEBEB");
    auto button_hover_style = button->get_hover_style();
    button_hover_style.m_background_color = QColor("#4B23A0");
    button_hover_style.m_text_color = Qt::white;
    auto button_focus_style = button->get_focus_style();
    button_focus_style.m_background_color = QColor("#EBEBEB");
    button_focus_style.m_border_color = QColor("#4B23A0");
    auto disabled_style = button->get_disabled_style();
    disabled_style.m_text_color = QColor("#A0A0A0");
    disabled_style.m_border_color = QColor("#C8C8C8");
    disabled_style.m_background_color = QColor("#C8C8C8");
    auto button_font = QFont();
    button_font.setFamily("Roboto");
    button_font.setPixelSize(scale_height(12));
    button->setFont(button_font);
    button->set_style(button_style);
    button->set_disabled_style(disabled_style);
    button->set_hover_style(button_hover_style);
    button->set_focus_style(button_focus_style);
    return button;
  }
}

FlatButtonTestWidget::FlatButtonTestWidget(QWidget* parent)
    : QWidget(parent) {
  auto container_widget = new QWidget(this);
  m_layout = new QGridLayout(container_widget);
  m_status_label = new QLabel(this);
  m_status_label->setFocusPolicy(Qt::NoFocus);
  m_layout->addWidget(m_status_label, 0, 1);
  set_button(make_labeled_button(tr("FlatButton"), this));
  m_label_input = new TextInputWidget(this);
  m_label_input->setFixedSize(BUTTON_SIZE());
  m_layout->addWidget(m_label_input, 1, 0);
  connect(m_label_input, &TextInputWidget::editingFinished, [=] {
    on_create_button();
  });
  auto create_button = make_labeled_button("Create Button", this);
  m_layout->addWidget(create_button, 1, 1);
  create_button->connect_clicked_signal([=] { on_create_button(); });
  m_disable_check_box = make_check_box(tr("Disable"), this);
  m_disable_check_box->setChecked(false);
  m_layout->addWidget(m_disable_check_box, 2, 1);
  connect(m_disable_check_box, &QCheckBox::stateChanged, [=] (auto state) {
    m_button->setDisabled(m_disable_check_box->isChecked());
  });
  m_pressed_timer = new QTimer(this);
  m_pressed_timer->setInterval(1000);
  connect(m_pressed_timer, &QTimer::timeout, [=] {
    m_status_label->setText("");
  });
}

void FlatButtonTestWidget::set_button(FlatButton* button) {
  m_button = button;
  m_button->connect_clicked_signal([=] {
    m_status_label->setText(tr("Button pressed."));
    m_pressed_timer->start();
  });
  m_layout->addWidget(m_button, 0, 0);
}

void FlatButtonTestWidget::on_create_button() {
  m_disable_check_box->setChecked(false);
  delete_later(m_button);
  set_button(make_labeled_button(m_label_input->text(), this));
}
