#include "Spire/UiViewer/TransitionTestWidget.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/FlatButton.hpp"

using namespace Spire;

TransitionTestWidget::TransitionTestWidget(QWidget* parent)
    : QWidget(parent),
      m_transition_widget(nullptr) {
  auto layout = new QVBoxLayout(this);
  auto control_button = make_flat_button(tr("Start"), this);
  control_button->setFixedSize(scale(100, 26));
  layout->addWidget(control_button);
  auto container = new QWidget(this);
  container->setStyleSheet("background-color: #4B23A0;");
  layout->addWidget(container);
  control_button->connect_clicked_signal([=] {
    if(m_transition_widget != nullptr) {
      m_transition_widget->hide();
      delete_later(m_transition_widget);
      control_button->set_label(tr("Start"));
    } else {
      m_transition_widget = new TransitionWidget(container);
      layout->insertWidget(1, m_transition_widget);
      control_button->set_label(tr("Hide"));
    }
  });
}
