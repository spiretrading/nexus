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

class DropDownBox::ButtonContainer : public QWidget {
  public:
    ButtonContainer(QWidget* parent)
        : QWidget(parent) {
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      layout->setSpacing(0);
      m_label = make_label("");
      m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      m_label->setAttribute(Qt::WA_TransparentForMouseEvents);
      layout->addWidget(m_label);
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
      layout->addLayout(icon_layout);
    }

    const std::shared_ptr<TextModel>& get_text_model() const {
      return m_label->get_model();
    }

  private:
    TextBox* m_label;
};

class DropDownBox::DropDownListWrapper : public QWidget {
  public:
    DropDownListWrapper(ListView& list_view, QWidget* parent)
        : QWidget(parent),
          m_list_view(&list_view) {
      m_drop_down_list = new DropDownList(*m_list_view, parent);
      auto list_model = m_list_view->get_list_model();
      for(auto i = 0; i < list_model->get_size(); ++i) {
        m_list_view->get_list_item(i)->setFocusPolicy(Qt::NoFocus);
      }
      m_list_view->installEventFilter(this);
      get_panel()->installEventFilter(this);
    }

    QWidget* get_panel() const {
      return m_drop_down_list->window();
    }

  protected:
    bool eventFilter(QObject* watched, QEvent* event) {
      if(watched == m_list_view) {
        if(event->type() == QEvent::KeyPress) {
          auto key_event = static_cast<QKeyEvent*>(event);
          switch(key_event->key()) {
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
            case Qt::Key_Escape:
              hide();
              QCoreApplication::sendEvent(parentWidget(), event);
              break;
          }
        }
      } else if(watched == get_panel()) {
        if(event->type() == QEvent::Close) {
          hide();
        } else if(event->type() == QEvent::Show) {
          m_list_view->setFocus();
          m_list_view->get_current_model()->set_current(
            m_list_view->get_current_model()->get_current());
        }
      }
      return QWidget::eventFilter(watched, event);
    }

    bool event(QEvent* event) override {
      if(event->type() == QEvent::Show) {
        m_drop_down_list->show();
      } else if(event->type() == QEvent::Hide) {
        m_drop_down_list->hide();
      }
      return QWidget::event(event);
    }

  private:
    ListView* m_list_view;
    DropDownList* m_drop_down_list;
};

DropDownBox::DropDownBox(ListView& list_view, QWidget* parent)
    : QWidget(parent),
      m_list_view(&list_view),
      m_submission(m_list_view->get_current_model()->get_current()),
      m_is_read_only(false) {
  m_button_container = new ButtonContainer(this);
  m_input_box = make_input_box(m_button_container);
  m_button = new Button(m_input_box);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_button);
  m_drop_down_list = new DropDownListWrapper(*m_list_view, this);
  m_drop_down_list->hide();
  proxy_style(*this, *m_input_box);
  set_style(*this, DEFAULT_STYLE());
  setFocusProxy(m_button);
  m_button->connect_clicked_signal([=] { on_click(); });
  m_list_view_current_connection =
    m_list_view->get_current_model()->connect_current_signal(
      [=] (auto& current) { on_list_view_current(current); });
  m_list_view_submit_connection = m_list_view->connect_submit_signal(
    [=] (auto& submission) { on_list_view_submit(submission); });
  m_button->installEventFilter(this);
  m_drop_down_list->get_panel()->installEventFilter(this);
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
  if(watched == m_button) {
    if(event->type() == QEvent::FocusIn) {
      match(*m_input_box, Focus());
    } else if(event->type() == QEvent::FocusOut) {
      if(m_drop_down_list->isVisible()) {
        match(*m_input_box, Focus());
      } else {
        unmatch(*m_input_box, Focus());
      }
    }
  } else if(watched == m_drop_down_list->get_panel()) {
    if(event->type() == QEvent::Close) {
      if(!m_button->hasFocus()) {
        update_submission();
        unmatch(*m_input_box, Focus());
      } else {
        update_current();
      }
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
    case Qt::Key_Escape:
      update_current();
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
  m_button_container->get_text_model()->set_current(std::move(text_current));
}

void DropDownBox::on_list_view_submit(const std::any& submission) {
  m_drop_down_list->hide();
  update_submission();
}

void DropDownBox::update_current() {
  if(m_submission != m_list_view->get_current_model()->get_current()) {
    m_list_view->get_current_model()->set_current(m_submission);
  }
}

void DropDownBox::update_submission() {
  m_submission = m_list_view->get_current_model()->get_current();
  if(m_submission) {
    m_submit_signal(m_list_view->get_list_model()->at(*m_submission));
  }
  m_list_view->get_selection_model()->set_current(m_submission);
}
