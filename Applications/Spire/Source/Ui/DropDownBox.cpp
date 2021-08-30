#include "Spire/Ui/DropDownBox.hpp"
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(ReadOnly() >> is_a<Icon>()).
      set(Visibility::INVISIBLE);
    style.get(ReadOnly() >> is_a<Button>()).
      set(Visibility::INVISIBLE);
    style.get(Disabled() >> is_a<Icon>()).
      set(Fill(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }

  auto ICON_STYLE(StyleSheet style) {
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(Fill(QColor::fromRgb(0x33, 0x33, 0x33)));
    return style;
  }

  auto TEXT_BOX_STYLE(StyleSheet style) {
    style.get(Any()).
      set(PaddingRight(scale_width(22)));
    style.get(ReadOnly()).
      set(PaddingRight(scale_width(0)));
    return style;
  }
}

class DropDownBox::DropDownListWrapper : public QWidget {
  public:
    DropDownListWrapper(ListView& list_view, QWidget* parent)
        : QWidget(parent),
          m_list_view(&list_view) {
      m_drop_down_list = new DropDownList(*m_list_view, parent);
      m_panel = m_drop_down_list->window();
      m_drop_down_list->setFocusProxy(m_list_view);
      for(auto i = 0; i < m_list_view->get_list_model()->get_size(); ++i) {
        m_list_view->get_list_item(i)->setFocusPolicy(Qt::NoFocus);
      }
      m_list_view->installEventFilter(this);
      m_panel->installEventFilter(this);
    }

    QWidget& get_panel() const {
      return *m_panel;
    }

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
      if(watched == m_list_view) {
        if(event->type() == QEvent::KeyPress) {
          auto& key_event = *static_cast<QKeyEvent*>(event);
          switch(key_event.key()) {
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
            case Qt::Key_Escape:
              hide();
              parentWidget()->setFocus();
              QCoreApplication::sendEvent(parentWidget(), event);
              break;
          }
        }
      } else if(watched == m_panel) {
        if(event->type() == QEvent::Close) {
          hide();
        } else if(event->type() == QEvent::KeyPress) {
          QCoreApplication::sendEvent(m_list_view, event);
        } else if(event->type() == QEvent::MouseButtonPress) {
          auto& mouse_event = *static_cast<QMouseEvent*>(event);
          if(parentWidget()->rect().contains(
              parentWidget()->mapFromGlobal(mouse_event.globalPos()))) {
            m_panel->setAttribute(Qt::WA_NoMouseReplay);
          }
        }
      }
      return QWidget::eventFilter(watched, event);
    }

    void showEvent(QShowEvent* event) override {
      m_drop_down_list->show();
      QWidget::showEvent(event);
    }

    void hideEvent(QHideEvent* event) override {
      m_drop_down_list->hide();
      QWidget::hideEvent(event);
    }

  private:
    ListView* m_list_view;
    DropDownList* m_drop_down_list;
    QWidget* m_panel;
};

DropDownBox::DropDownBox(ListView& list_view, QWidget* parent)
    : QWidget(parent),
      m_list_view(&list_view),
      m_submission(m_list_view->get_current_model()->get_current()) {
  auto layers = new LayeredWidget();
  m_text_box = new TextBox();
  m_text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_text_box->setFocusPolicy(Qt::NoFocus);
  set_style(*m_text_box, TEXT_BOX_STYLE(get_style(*m_text_box)));
  layers->add(m_text_box);
  auto icon_layer = new QWidget();
  icon_layer->setAttribute(Qt::WA_TransparentForMouseEvents);
  icon_layer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto icon_layer_layout = new QHBoxLayout(icon_layer);
  icon_layer_layout->setContentsMargins({});
  icon_layer_layout->setSpacing(0);
  icon_layer_layout->addStretch();
  auto drop_down_icon = new Icon(
    imageFromSvg(":/Icons/dropdown-arrow.svg", scale(6, 4)));
  drop_down_icon->setFixedSize(scale(6, 4));
  set_style(*drop_down_icon, ICON_STYLE(get_style(*drop_down_icon)));
  icon_layer_layout->addWidget(drop_down_icon);
  icon_layer_layout->addSpacing(scale_width(8));
  layers->add(icon_layer);
  m_button = new Button(new QWidget());
  m_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layers->add(m_button);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(layers);
  m_drop_down_list = new DropDownListWrapper(*m_list_view, this);
  m_drop_down_list->hide();
  set_style(*this, DEFAULT_STYLE());
  setFocusProxy(m_button);
  m_button->connect_clicked_signal([=] { on_click(); });
  m_current_connection =
    m_list_view->get_current_model()->connect_current_signal(
      [=] (auto& current) { on_current(current); });
  m_submit_connection = m_list_view->connect_submit_signal(
    [=] (auto& submission) { on_submit(submission); });
  m_button->installEventFilter(this);
  m_drop_down_list->get_panel().installEventFilter(this);
}

bool DropDownBox::is_read_only() const {
  return m_text_box->is_read_only();
}

void DropDownBox::set_read_only(bool is_read_only) {
  m_text_box->set_read_only(is_read_only);
  if(is_read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
}

connection DropDownBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool DropDownBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_button) {
    if(event->type() == QEvent::FocusIn) {
      match(*m_text_box, Focus());
    } else if(event->type() == QEvent::FocusOut) {
      if(m_drop_down_list->isVisible()) {
        match(*m_text_box, Focus());
      } else {
        unmatch(*m_text_box, Focus());
        update_submission();
      }
    } else if(event->type() == QEvent::Enter) {
      match(*m_text_box, Hover());
    } else if(event->type() == QEvent::Leave) {
      unmatch(*m_text_box, Hover());
    }
  } else if(watched == &m_drop_down_list->get_panel()) {
    if(event->type() == QEvent::Close) {
      unmatch(*m_text_box, Focus());
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DropDownBox::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Escape:
      update_current();
      break;
    default:
      QCoreApplication::sendEvent(m_list_view, event);
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

void DropDownBox::on_current(const boost::optional<int>& current) {
  auto text_current = [=] {
    if(current) {
      return displayTextAny(m_list_view->get_list_model()->at(*current));
    }
    return QString("");
  }();
  m_text_box->get_model()->set_current(std::move(text_current));
}

void DropDownBox::on_submit(const std::any& submission) {
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
