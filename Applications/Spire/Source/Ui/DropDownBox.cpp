#include "Spire/Ui/DropDownBox.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QApplication>
#include <QEnterEvent>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(ReadOnly() > is_a<TextBox>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_color(QColor(Qt::transparent))).
      set(horizontal_padding(0));
    style.get(Disabled() > is_a<TextBox>()).
      set(BackgroundColor(QColor(0xF5F5F5))).
      set(border_color(QColor(0xC8C8C8))).
      set(TextColor(QColor(0xC8C8C8)));
    style.get((ReadOnly() && Disabled()) > is_a<TextBox>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_color(QColor(Qt::transparent)));
    style.get(Any() > (is_a<Icon>() && !(+Any() << is_a<ListItem>()))).
      set(Fill(QColor(0x333333))).
      set(BackgroundColor(QColor(Qt::transparent)));
    style.get(Disabled() > (is_a<Icon>() && !(+Any() << is_a<ListItem>()))).
      set(Fill(QColor(0xC8C8C8)));
    style.get(ReadOnly() > (is_a<Icon>() && !(+Any() << is_a<ListItem>()))).
      set(Visibility::NONE);
    style.get(Any() > (is_a<TextBox>() && !(+Any() << is_a<ListItem>()))).
      set(PaddingRight(scale_width(14)));
    style.get(PopUp() > is_a<TextBox>() ||
      (+Any() > is_a<Button>() && (Hover() || FocusIn())) > is_a<TextBox>()).
      set(border_color(QColor(0x4B23A0)));
    style.get(ReadOnly() > (is_a<TextBox>() && !(+Any() << is_a<ListItem>()))).
      set(horizontal_padding(0)).
      set(border_color(QColor(Qt::transparent))).
      set(BackgroundColor(QColor(Qt::transparent)));
    return style;
  }
}

DropDownBox::DropDownBox(std::shared_ptr<AnyListModel> list, QWidget* parent)
  : DropDownBox(std::move(list), ListView::default_view_builder, parent) {}

DropDownBox::DropDownBox(std::shared_ptr<AnyListModel> list,
  ViewBuilder<> view_builder, QWidget* parent)
  : DropDownBox(std::move(list),
      std::make_shared<LocalValueModel<optional<int>>>(),
      std::make_shared<ListSingleSelectionModel>(),
      std::move(view_builder), parent) {}

DropDownBox::DropDownBox(std::shared_ptr<AnyListModel> list,
  std::shared_ptr<CurrentModel> current, ViewBuilder<> view_builder,
  QWidget* parent)
  : DropDownBox(std::move(list), std::move(current),
      std::make_shared<ListSingleSelectionModel>(), std::move(view_builder),
      parent) {}

DropDownBox::DropDownBox(std::shared_ptr<AnyListModel> list,
    std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection, ViewBuilder<> view_builder,
    QWidget* parent)
    : QWidget(parent),
      m_timer(this),
      m_is_read_only(false),
      m_is_modified(false),
      m_is_mouse_press_on_list(false) {
  m_list_view = new ListView(std::move(list), std::move(current),
    std::move(selection), std::move(view_builder));
  m_text_box = new TextBox();
  m_text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_text_box->setFocusPolicy(Qt::NoFocus);
  m_text_box->set_read_only(true);
  m_text_box->setDisabled(true);
  update_style(*m_text_box, [] (auto& style) {
    style.get(ReadOnly()).clear();
    style.get(Disabled()).clear();
    style.get(ReadOnly() && Disabled()).clear();
  });
  auto layers = new LayeredWidget();
  layers->add(m_text_box);
  auto icon_layer = new QWidget();
  icon_layer->setAttribute(Qt::WA_TransparentForMouseEvents);
  icon_layer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto drop_down_icon =
    new Icon(imageFromSvg(":/Icons/dropdown-arrow.svg", scale(6, 4)));
  drop_down_icon->setFixedSize(scale(6, 4));
  auto icon_layer_layout = make_hbox_layout(icon_layer);
  icon_layer_layout->addStretch();
  icon_layer_layout->addWidget(drop_down_icon);
  icon_layer_layout->addSpacing(scale_width(8));
  layers->add(icon_layer);
  m_button = new Button(new QWidget());
  m_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_button_press_observer.emplace(*m_button);
  layers->add(m_button);
  enclose(*this, *layers);
  m_drop_down_list = new DropDownList(*m_list_view, *this);
  m_drop_down_list->installEventFilter(this);
  auto window = m_drop_down_list->window();
  window->setWindowFlags(Qt::Popup | (window->windowFlags() & ~Qt::Tool));
  window->installEventFilter(this);
  set_style(*this, DEFAULT_STYLE());
  setFocusProxy(m_button);
  on_current(get_current()->get());
  m_button_press_observer->connect_press_end_signal(
    std::bind_front(&DropDownBox::on_button_press_end, this));
  m_current_connection = m_list_view->get_current()->connect_update_signal(
    [=] (const auto& current) { on_current(current); });
  m_submit_connection = m_list_view->connect_submit_signal(
    [=] (const auto& submission) { on_submit(submission); });
  m_button->installEventFilter(this);
  m_timer.setSingleShot(true);
}

const std::shared_ptr<AnyListModel>& DropDownBox::get_list() const {
  return m_list_view->get_list();
}

const std::shared_ptr<DropDownBox::CurrentModel>&
    DropDownBox::get_current() const {
  return m_list_view->get_current();
}

const std::shared_ptr<DropDownBox::SelectionModel>&
    DropDownBox::get_selection() const {
  return m_list_view->get_selection();
}

bool DropDownBox::is_read_only() const {
  return m_is_read_only;
}

void DropDownBox::set_read_only(bool is_read_only) {
  if(m_is_read_only == is_read_only) {
    return;
  }
  m_is_read_only = is_read_only;
  if(m_is_read_only) {
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
    if(event->type() == QEvent::KeyPress) {
      auto& key_event = *static_cast<const QKeyEvent*>(event);
      if(key_event.key() == Qt::Key_Enter ||
          key_event.key() == Qt::Key_Return) {
        if(is_read_only()) {
          event->ignore();
        } else {
          m_is_modified = true;
          submit();
        }
        return true;
      }
    } else if(event->type() == QEvent::KeyRelease) {
      auto& key_event = *static_cast<const QKeyEvent*>(event);
      if(key_event.key() == Qt::Key_Enter ||
          key_event.key() == Qt::Key_Return) {
        return true;
      }
    } else if(event->type() == QEvent::FocusOut) {
      if(!is_read_only() && !m_drop_down_list->isVisible()) {
        submit();
        if(m_submission) {
          m_list_view->get_selection()->push(*m_submission);
        } else {
          clear(*m_list_view->get_selection());
        }
      }
    }
  } else if(watched == m_drop_down_list) {
    if(event->type() == QEvent::KeyPress) {
      if(!is_read_only()) {
        auto key = static_cast<QKeyEvent*>(event)->key();
        if(key == Qt::Key_Escape) {
          revert_current();
        }
      }
    } else if(event->type() == QEvent::MouseMove) {
      if(m_is_mouse_press_on_list) {
        auto& mouse_event = *static_cast<QMouseEvent*>(event);
        if(m_drop_down_list->rect().contains(
            m_drop_down_list->mapFromGlobal(mouse_event.globalPos()))) {
          if(m_list_view->rect().contains(
              m_list_view->mapFromGlobal(mouse_event.globalPos()))) {
            if(auto index = get_index_under_mouse(mouse_event.globalPos());
                index >= 0) {
              auto item = m_list_view->get_list_item(index);
              if(m_hovered_item != item) {
                leave_hovered_item();
                m_hovered_item = item;
                enter_hovered_item(mouse_event);
              }
            }
          } else {
            leave_hovered_item();
          }
        } else {
          leave_hovered_item();
        }
      }
    } else if(event->type() == QEvent::MouseButtonPress) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(mouse_event.button() == Qt::LeftButton) {
        m_is_mouse_press_on_list = true;
      }
    } else if(event->type() == QEvent::MouseButtonRelease) {
      if(m_drop_down_list->isVisible()) {
        auto& mouse_event = *static_cast<QMouseEvent*>(event);
        if(mouse_event.button() == Qt::LeftButton &&
            m_drop_down_list->rect().contains(
              m_drop_down_list->mapFromGlobal(mouse_event.globalPos()))) {
          auto delta = mouse_event.globalPos() - m_mouse_press_position;
          if(delta.manhattanLength() > 9 || m_timer.isActive()) {
            if(auto index = get_index_under_mouse(mouse_event.globalPos());
                index >= 0) {
              m_list_view->get_current()->set(index);
              on_submit(index);
              return true;
            }
          }
        }
      }
    }
  } else if(watched == m_drop_down_list->window()) {
    if(event->type() == QEvent::Close) {
      auto& close_event = static_cast<QCloseEvent&>(*event);
      close_event.ignore();
      m_drop_down_list->hide();
    } else if(event->type() == QEvent::Show) {
      match(*this, PopUp());
    } else if(event->type() == QEvent::Hide) {
      leave_hovered_item();
      unmatch(*this, PopUp());
    } else if(event->type() == QEvent::MouseButtonPress) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(rect().contains(mapFromGlobal(mouse_event.globalPos()))) {
        m_drop_down_list->window()->setAttribute(Qt::WA_NoMouseReplay);
      }
    } else if(event->type() == QEvent::MouseButtonRelease) {
      if(!m_is_mouse_press_on_list && m_drop_down_list->isVisible()) {
        auto& mouse_event = *static_cast<QMouseEvent*>(event);
        if(mouse_event.button() == Qt::LeftButton) {
          if(!m_timer.isActive()) {
            m_drop_down_list->hide();
          }
        }
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DropDownBox::keyPressEvent(QKeyEvent* event) {
  if(is_read_only()) {
    QWidget::keyPressEvent(event);
    return;
  }
  switch(event->key()) {
    case Qt::Key_Escape:
      revert_current();
      break;
    default:
      QCoreApplication::sendEvent(m_list_view, event);
  }
  QWidget::keyPressEvent(event);
}

void DropDownBox::mousePressEvent(QMouseEvent* event) {
  if(is_read_only()) {
    return;
  }
  if(event->button() == Qt::LeftButton) {
    if(m_drop_down_list->isVisible()) {
      m_drop_down_list->hide();
    } else {
      m_is_mouse_press_on_list = false;
      m_mouse_press_position = event->globalPos();
      m_drop_down_list->show();
      m_drop_down_list->setFocus();
      m_timer.start(QApplication::doubleClickInterval());
    }
  }
}

int DropDownBox::get_index_under_mouse(const QPoint& global_point) const {
  for(auto i = 0; i < m_list_view->get_list()->get_size(); ++i) {
    auto& item = *m_list_view->get_list_item(i);
    if(item.rect().contains(item.mapFromGlobal(global_point))) {
      return i;
    }
  }
  return -1;
}

void DropDownBox::enter_hovered_item(const QMouseEvent& event) {
  if(!m_hovered_item) {
    return;
  }
  auto local_position = m_hovered_item->mapFromGlobal(event.globalPos());
  auto enter_event =
    QEnterEvent(local_position, event.windowPos(), event.screenPos());
  QCoreApplication::sendEvent(m_hovered_item, &enter_event);
}

void DropDownBox::leave_hovered_item() {
  if(!m_hovered_item) {
    return;
  }
  auto leave_event = QEvent(QEvent::Type::Leave);
  QCoreApplication::sendEvent(m_hovered_item, &leave_event);
  m_hovered_item = nullptr;
}

void DropDownBox::revert_current() {
  if(m_submission != m_list_view->get_current()->get()) {
    m_list_view->get_current()->set(m_submission);
  }
}

void DropDownBox::submit() {
  if(!m_is_modified) {
    return;
  }
  m_submission = m_list_view->get_current()->get();
  if(m_submission) {
    m_is_modified = false;
    m_submit_signal(m_list_view->get_list()->get(*m_submission));
  }
}

void DropDownBox::on_button_press_end(PressObserver::Reason reason) {
  if(reason == PressObserver::Reason::KEYBOARD) {
    if(m_drop_down_list->isVisible()) {
      m_drop_down_list->hide();
    } else if(!is_read_only()) {
      m_drop_down_list->show();
      m_drop_down_list->setFocus();
    }
  }
}

void DropDownBox::on_current(const optional<int>& current) {
  auto text = [&] {
    if(current) {
      return to_text(m_list_view->get_list()->get(*current));
    }
    return QString();
  }();
  m_is_modified = true;
  m_text_box->get_current()->set(text);
}

void DropDownBox::on_submit(const std::any& submission) {
  if(is_read_only()) {
    return;
  }
  m_drop_down_list->hide();
  submit();
}
