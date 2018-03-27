#include "spire/toolbar_ui_tester/toolbar_ui_tester.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/toolbar/toolbar_window.hpp"

using namespace spire;

toolbar_ui_tester::toolbar_ui_tester(toolbar_window* window,
    recently_closed_model& model,
    QWidget* parent)
    : QWidget(parent),
      m_recently_closed_model(&model) {
  setGeometry(window->pos().x(), window->pos().y() + window->height() + 100, 0,
    0);
  setAttribute(Qt::WA_ShowWithoutActivating);
  setFixedSize(scale(480, 100));
  auto layout = new QVBoxLayout(this);
  auto radio_layout = new QHBoxLayout();
  layout->addLayout(radio_layout);
  m_book_view_radio = new QRadioButton("Book View", this);
  m_book_view_radio->toggle();
  radio_layout->addWidget(m_book_view_radio);
  m_time_sale_radio = new QRadioButton("Time and Sale", this);
  radio_layout->addWidget(m_time_sale_radio);
  m_text_line_edit = new QLineEdit(this);
  m_text_line_edit->setPlaceholderText("Name/Identifier text");
  m_text_line_edit->setFixedHeight(scale_height(20));
  layout->addWidget(m_text_line_edit);
  m_add_button = new QPushButton("Add", this);
  m_add_button->setStyleSheet("background-color: #33FF33;");
  layout->addWidget(m_add_button);
  connect(m_add_button, &QPushButton::clicked, this,
    &toolbar_ui_tester::add_item);
  window->connect_reopen_signal([&] (auto& e) { remove_item(e); });
  window->installEventFilter(this);
}

bool toolbar_ui_tester::eventFilter(QObject* receiver, QEvent* event) {
  if(event->type() == QEvent::Close) {
    close();
  }
  return QWidget::eventFilter(receiver, event);
}

void toolbar_ui_tester::add_item() {
  if(!m_text_line_edit->text().isEmpty()) {
    if(m_book_view_radio->isChecked()) {
      m_recently_closed_model->add(recently_closed_model::type::BOOK_VIEW,
        m_text_line_edit->text().toStdString());
    } else {
      m_recently_closed_model->add(recently_closed_model::type::TIME_AND_SALE,
        m_text_line_edit->text().toStdString());
    }
  }
}

void toolbar_ui_tester::remove_item(const recently_closed_model::entry& e) {
  m_recently_closed_model->remove(e);
}
