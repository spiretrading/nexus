#include "Spire/Ui/DropDownBox.hpp"
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any() >> is_a<Icon>()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(Fill(QColor::fromRgb(0x33, 0x33, 0x33)));
    style.get(Disabled() >> is_a<Icon>()).
      set(Fill(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    style.get(ReadOnly() >> is_a<Icon>()).
      set(Visibility::NONE);
    style.get(Any() >> (is_a<TextBox>() && !(+Any() << is_a<ListView>()))).
      set(PaddingRight(scale_width(8)));
    style.get(Disabled() >> (is_a<TextBox>() && !(+Any() << is_a<ListView>()))).
      set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }
}

DropDownBox::DropDownBox(ListView& list_view, QWidget* parent)
    : QWidget(parent),
      m_list_view(&list_view),
      m_is_read_only(false) {
  auto container = new QWidget();
  auto container_layout = new QHBoxLayout(container);
  container_layout->setContentsMargins({});
  container_layout->setSpacing(0);
  m_text_box = make_label("");
  m_text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_text_box->setAttribute(Qt::WA_TransparentForMouseEvents);
  container_layout->addWidget(m_text_box);
  auto icon_layout = new QVBoxLayout();
  icon_layout->setContentsMargins({});
  icon_layout->setSpacing(0);
  icon_layout->addStretch();
  auto drop_down_icon = new Icon(
    imageFromSvg(":/Icons/dropdown-arrow.svg", scale(6, 4)));
  drop_down_icon->setFixedSize(scale(6, 4));
  drop_down_icon->setFocusPolicy(Qt::NoFocus);
  icon_layout->addWidget(drop_down_icon);
  icon_layout->addStretch();
  container_layout->addLayout(icon_layout);
  m_input_box = make_input_box(container);
  m_button = new Button(m_input_box);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_button);
  proxy_style(*this, *m_input_box);
  set_style(*this, DEFAULT_STYLE());
  setFocusProxy(m_button);
  m_drop_down_list = new DropDownList(*m_list_view, this);
  auto list_model = m_list_view->get_list_model();
  for(auto i = 0; i < list_model->get_size(); ++i) {
    m_list_view->get_list_item(i)->setFocusPolicy(Qt::NoFocus);
  }
  m_button->connect_clicked_signal([=] { on_click(); });
  m_list_view_current_connection =
    m_list_view->get_current_model()->connect_current_signal(
      [=] (auto& current) { on_list_view_current(current); });
  m_list_view_submit_connection = m_list_view->connect_submit_signal(
    [=] (auto& submission) { on_list_view_submit(submission); });
  m_button->installEventFilter(this);
  m_drop_down_list->window()->installEventFilter(this);
  m_list_view->installEventFilter(this);
}

bool DropDownBox::is_read_only() const {
  return m_is_read_only;
}

void DropDownBox::set_read_only(bool is_read_only) {
  m_is_read_only = is_read_only;
  if(m_is_read_only) {
    match(*this, ReadOnly());
    match(*m_input_box, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
    unmatch(*m_input_box, ReadOnly());
  }
}

connection DropDownBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool DropDownBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    if(m_drop_down_list->isVisible()) {
      auto key_event = static_cast<QKeyEvent*>(event);
      switch(key_event->key()) {
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
          m_drop_down_list->hide();
          QCoreApplication::sendEvent(m_button, event);
          break;
        case Qt::Key_Escape:
          m_list_view->get_current_model()->set_current(m_submission_index);
          m_drop_down_list->hide();
          break;
      }
    }
  } else if(event->type() == QEvent::Close &&
      watched == m_drop_down_list->window()) {
    m_drop_down_list->hide();
    if(!m_button->hasFocus()) {
      update_submission();
      m_list_view->get_selection_model()->set_current(m_submission_index);
      unmatch(*m_input_box, Focus());
    }
  } else if(event->type() == QEvent::Show &&
      watched == m_drop_down_list->window()) {
    m_list_view->setFocus();
    m_list_view->get_current_model()->set_current(
      m_list_view->get_current_model()->get_current());
  } else if(event->type() == QEvent::FocusIn) {
    match(*m_input_box, Focus());
  } else if(event->type() == QEvent::FocusOut) {
    if(m_drop_down_list->isVisible()) {
      match(*m_input_box, Focus());
    } else {
      unmatch(*m_input_box, Focus());
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DropDownBox::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Down:
    case Qt::Key_Up:
      if(!m_is_read_only) {
        m_drop_down_list->show();
      }
      break;
  }
  QWidget::keyPressEvent(event);
}

void DropDownBox::on_click() {
  if(m_is_read_only) {
    return;
  }
  if(m_drop_down_list->isVisible()) {
    m_drop_down_list->hide();
  } else {
    m_drop_down_list->show();
  }
}

void DropDownBox::on_list_view_current(const boost::optional<int>& current) {
  auto text_current = [=] {
    if(current) {
      return displayTextAny(m_list_view->get_list_model()->at(*current));
    }
    return QString("");
  }();
  m_text_box->get_model()->set_current(std::move(text_current));
}

void DropDownBox::on_list_view_submit(const std::any& submission) {
  update_submission();
  m_drop_down_list->hide();
}

void DropDownBox::update_submission() {
  m_submission_index = m_list_view->get_current_model()->get_current();
  if(m_submission_index) {
    m_submit_signal(m_list_view->get_list_model()->at(*m_submission_index));
  } else {
    m_submit_signal(none);
  }
}
