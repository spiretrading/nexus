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
      set(Fill(QColor::fromRgb(0x33, 0x33, 0x33)));
    style.get(Disabled() >> is_a<Icon>()).
      set(Fill(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    style.get(ReadOnly() >> (is_a<TextBox>() && !(+Any() << is_a<ListView>()))).
      set(PaddingRight(scale_width(8)));
    return style;
  }
}

DropDownBox::DropDownBox(ListView& list_view, QWidget* parent)
    : QWidget(parent),
      m_list_view(&list_view) {
  auto container = new QWidget();
  auto container_layout = new QHBoxLayout(container);
  container_layout->setContentsMargins({});
  container_layout->setSpacing(0);
  m_text_box = new TextBox();
  m_text_box->set_read_only(true);
  m_text_box->setFocusPolicy(Qt::NoFocus);
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
    auto item = m_list_view->get_list_item(i);
    if(item) {
      item->setFocusPolicy(Qt::NoFocus);
    }
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

connection DropDownBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool DropDownBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto key_event = static_cast<QKeyEvent*>(event);
    if(watched == m_drop_down_list->window()) {
      switch(key_event->key()) {
        case Qt::Key_Down:
        case Qt::Key_Up:
        case Qt::Key_Home:
        case Qt::Key_End:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
          QCoreApplication::sendEvent(m_list_view, event);
          break;
        default:
        {
          auto text = key_event->text();
          if(text.size() == 1 &&
              (text[0].isLetterOrNumber() || text[0] == '_')) {
            QCoreApplication::sendEvent(m_list_view, event);
            break;
          }
        }
      }
    }
    if(m_drop_down_list->isVisible() && (key_event->key() == Qt::Key_Tab ||
        key_event->key() == Qt::Key_Backtab)) {
      m_drop_down_list->hide();
      QCoreApplication::sendEvent(m_button, event);
    } else if(m_drop_down_list->isVisible() &&
        key_event->key() == Qt::Key_Escape) {
      on_escape_press();
    }
  } else if(event->type() == QEvent::Close &&
      watched == m_drop_down_list->window()) {
    on_panel_close();
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
      m_drop_down_list->show();
      break;
  }
  QWidget::keyPressEvent(event);
}

void DropDownBox::on_click() {
  if(m_drop_down_list->isVisible()) {
    m_drop_down_list->hide();
  } else {
    m_drop_down_list->show();
  }
}

void DropDownBox::on_list_view_current(const boost::optional<int>& current) {
  auto text_current = [&] {
    if(current) {
      return displayTextAny(
        m_list_view->get_list_model()->get<QString>(*current));
    }
    return QString("");
  }();
  m_text_box->get_model()->set_current(text_current);
}

void DropDownBox::on_list_view_submit(const std::any& submission) {
  update_submission();
  m_drop_down_list->hide();
}

void DropDownBox::on_escape_press() {
  m_list_view->get_current_model()->set_current(m_submission_index);
  m_drop_down_list->hide();
}

void DropDownBox::on_panel_close() {
  m_drop_down_list->hide();
  if(!m_button->hasFocus()) {
    update_submission();
    m_list_view->get_selection_model()->set_current(m_submission_index);
    unmatch(*m_input_box, Focus());
  }
}

void DropDownBox::update_submission() {
  m_submission_index = m_list_view->get_current_model()->get_current();
  if(m_submission_index) {
    m_submit_signal(m_list_view->get_list_model()->at(*m_submission_index));
  } else {
    m_submit_signal(none);
  }
}
