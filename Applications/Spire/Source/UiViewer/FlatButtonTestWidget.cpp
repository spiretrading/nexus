#include "Spire/UiViewer/FlatButtonTestWidget.hpp"
#include <QFont>
#include <QGridLayout>
#include <QLabel>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/FlatButton.hpp"

using namespace Spire;

namespace {
  auto BUTTON_SIZE() {
    static auto size = scale(100, 26);
    return size;
  }

  auto make_unlabeled_button(QWidget* parent) {
    auto button = new FlatButton(parent);
    button->setFixedSize(BUTTON_SIZE());
    auto style = button->get_style();
    style.m_background_color = QColor("#4B23A0");
    style.m_border_color = QColor("#C8C8C8");
    button->set_style(style);
    style.m_border_color = QColor("#4B23A0");
    button->set_hover_style(style);
    button->set_focus_style(style);
    auto disabled_style = button->get_disabled_style();
    disabled_style.m_border_color = QColor("#C8C8C8");
    disabled_style.m_background_color = QColor("#C8C8C8");
    button->set_disabled_style(disabled_style);
    return button;
  }

  auto make_labeled_button(const QString& text, QWidget* parent) {
    auto button = new FlatButton(text, parent);
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
  auto layout = new QGridLayout(container_widget);
  auto label = new QLabel(this);
  label->setFocusPolicy(Qt::NoFocus);
  layout->addWidget(label, 1, 0);
  auto unlabeled_button = make_unlabeled_button(this);
  unlabeled_button->connect_clicked_signal([=] {
    label->setText(tr("Unlabeled pressed."));
  });
  layout->addWidget(unlabeled_button, 0, 0);
  auto labeled_button = make_labeled_button(tr("Labeled Button"), this);
  labeled_button->connect_clicked_signal([=] {
    label->setText(tr("Labeled pressed."));
  });
  layout->addWidget(labeled_button, 0, 1);
  auto check_box = make_check_box(tr("Disable"), this);
  check_box->setChecked(false);
  layout->addWidget(check_box, 1, 1);
  connect(check_box, &QCheckBox::stateChanged, [=] (auto state) {
    if(state == Qt::Checked) {
      unlabeled_button->setDisabled(true);
      labeled_button->setDisabled(true);
    } else {
      unlabeled_button->setEnabled(true);
      labeled_button->setEnabled(true);
    }
  });
}
