#include "Spire/Ui/DropDownBox.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QApplication>
#include <QEnterEvent>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/Utility.hpp"
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

  bool has_focus(const QWidget& root) {
    if(auto focus_widget = QApplication::focusWidget()) {
      return is_ancestor(&root, focus_widget);
    }
    return false;
  }
}

DropDownBox::DropDownBox(std::shared_ptr<AnyListModel> list, QWidget* parent)
  : DropDownBox(std::move(list), ListView::default_item_builder, parent) {}

DropDownBox::DropDownBox(std::shared_ptr<AnyListModel> list,
  ListViewItemBuilder<> item_builder, QWidget* parent)
  : DropDownBox(std::move(list),
      std::make_shared<LocalValueModel<optional<int>>>(),
      std::make_shared<ListSingleSelectionModel>(),
      std::move(item_builder), parent) {}

DropDownBox::DropDownBox(std::shared_ptr<AnyListModel> list,
  ListViewItemBuilder<> item_builder, ToText to_text, QWidget* parent)
  : DropDownBox(std::move(list),
      std::make_shared<LocalValueModel<optional<int>>>(),
      std::make_shared<ListSingleSelectionModel>(),
      std::move(item_builder), std::move(to_text), parent) {}

DropDownBox::DropDownBox(std::shared_ptr<AnyListModel> list,
  std::shared_ptr<CurrentModel> current, ListViewItemBuilder<> item_builder,
  QWidget* parent)
  : DropDownBox(std::move(list), std::move(current),
      std::make_shared<ListSingleSelectionModel>(), std::move(item_builder),
      parent) {}

DropDownBox::DropDownBox(std::shared_ptr<AnyListModel> list,
  std::shared_ptr<CurrentModel> current, ListViewItemBuilder<> item_builder,
  ToText to_text, QWidget* parent)
  : DropDownBox(std::move(list), std::move(current),
      std::make_shared<ListSingleSelectionModel>(), std::move(item_builder),
      std::move(to_text), parent) {}

DropDownBox::DropDownBox(std::shared_ptr<AnyListModel> list,
  std::shared_ptr<CurrentModel> current,
  std::shared_ptr<SelectionModel> selection,
  ListViewItemBuilder<> item_builder, QWidget* parent)
  : DropDownBox(std::move(list), std::move(current), std::move(selection),
      std::move(item_builder),
      [] (const std::any& value) { return to_text(value); }, parent) {}

DropDownBox::DropDownBox(std::shared_ptr<AnyListModel> list,
    std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection,
    ListViewItemBuilder<> item_builder, ToText to_text, QWidget* parent)
    : QWidget(parent),
      m_list(std::move(list)),
      m_current(std::move(current)),
      m_selection(std::move(selection)),
      m_item_builder(std::move(item_builder)),
      m_to_text(std::move(to_text)),
      m_timer(this),
      m_is_read_only(false),
      m_is_modified(false),
      m_is_mouse_press_on_list(false),
      m_drop_down_list(nullptr) {
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
  link(*this, *m_text_box);
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
  link(*this, *drop_down_icon);
  m_button = new Button(new QWidget());
  m_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_button_press_observer.emplace(*m_button);
  layers->add(m_button);
  link(*this, *m_button);
  enclose(*this, *layers);
  set_style(*this, DEFAULT_STYLE());
  setFocusProxy(m_button);
  on_current(get_current()->get());
  m_current_connection = get_current()->connect_update_signal(
    std::bind_front(&DropDownBox::on_current, this));
  m_button_press_observer->connect_press_end_signal(
    std::bind_front(&DropDownBox::on_button_press_end, this));
  m_button->installEventFilter(this);
  m_timer.setSingleShot(true);
}

const std::shared_ptr<AnyListModel>& DropDownBox::get_list() const {
  return m_list;
}

const std::shared_ptr<DropDownBox::CurrentModel>&
    DropDownBox::get_current() const {
  return m_current;
}

const std::shared_ptr<DropDownBox::SelectionModel>&
    DropDownBox::get_selection() const {
  return m_selection;
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
      if(!is_read_only() && is_drop_down_list_visible() && !has_focus(*this)) {
        submit();
        if(m_submission) {
          m_selection->push(*m_submission);
        } else {
          clear(*m_selection);
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
          if(m_drop_down_list->get_list_view().rect().contains(
              m_drop_down_list->get_list_view().mapFromGlobal(
                mouse_event.globalPos()))) {
            if(auto index = get_index_under_mouse(mouse_event.globalPos());
                index >= 0) {
              auto item =
                m_drop_down_list->get_list_view().get_list_item(index);
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
      if(is_drop_down_list_visible()) {
        auto& mouse_event = *static_cast<QMouseEvent*>(event);
        if(mouse_event.button() == Qt::LeftButton &&
            m_drop_down_list->rect().contains(
              m_drop_down_list->mapFromGlobal(mouse_event.globalPos()))) {
          auto delta = mouse_event.globalPos() - m_mouse_press_position;
          if(delta.manhattanLength() > 9 || m_timer.isActive()) {
            if(auto index = get_index_under_mouse(mouse_event.globalPos());
                index >= 0) {
              m_current->set(index);
              on_submit(index);
              return true;
            }
          }
        }
      }
    }
  } else if(m_drop_down_list && watched == m_drop_down_list->window()) {
    if(event->type() == QEvent::Close) {
      auto& close_event = static_cast<QCloseEvent&>(*event);
      close_event.ignore();
      hide_drop_down_list();
    } else if(event->type() == QEvent::Hide) {
      leave_hovered_item();
    } else if(event->type() == QEvent::MouseButtonPress) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(rect().contains(mapFromGlobal(mouse_event.globalPos()))) {
        m_drop_down_list->window()->setAttribute(Qt::WA_NoMouseReplay);
      }
    } else if(event->type() == QEvent::MouseButtonRelease) {
      if(!m_is_mouse_press_on_list) {
        auto& mouse_event = *static_cast<QMouseEvent*>(event);
        if(mouse_event.button() == Qt::LeftButton) {
          if(!m_timer.isActive()) {
            hide_drop_down_list();
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
  if(event->key() == Qt::Key_Escape) {
    revert_current();
  } else if(!is_read_only() &&
      (event->key() != Qt::Key_Space || is_drop_down_list_visible())) {
    make_drop_down_list();
    QCoreApplication::sendEvent(&m_drop_down_list->get_list_view(), event);
  }
  QWidget::keyPressEvent(event);
}

void DropDownBox::mousePressEvent(QMouseEvent* event) {
  if(is_read_only()) {
    return;
  }
  if(event->button() == Qt::LeftButton) {
    if(is_drop_down_list_visible()) {
      hide_drop_down_list();
    } else {
      m_is_mouse_press_on_list = false;
      m_mouse_press_position = event->globalPos();
      show_drop_down_list();
      m_drop_down_list->setFocus();
      m_timer.start(QApplication::doubleClickInterval());
    }
  }
}

int DropDownBox::get_index_under_mouse(const QPoint& global_point) const {
  if(!is_drop_down_list_visible()) {
    return -1;
  }
  for(auto i = 0; i < m_list->get_size(); ++i) {
    auto& item = *m_drop_down_list->get_list_view().get_list_item(i);
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
  if(m_submission && m_submission != m_current->get()) {
    m_current->set(m_submission);
  }
}

bool DropDownBox::is_drop_down_list_visible() const {
  return m_drop_down_list && m_drop_down_list->window()->isVisible();
}

void DropDownBox::make_drop_down_list() {
  if(m_drop_down_list) {
    return;
  }
  if(m_selection->get_size() > 0) {
    m_selection->transact([&] {
      auto has_current = [&] {
        if(!m_current->get()) {
          return false;
        }
        auto i = std::find(
          m_selection->begin(), m_selection->end(), *m_current->get());
        return i != m_selection->end();
      }();
      clear(*m_selection);
      if(has_current) {
        m_selection->push(*m_current->get());
      }
    });
  }
  auto list_view =
    new ListView(m_list, m_current, m_selection, m_item_builder, m_to_text);
  m_drop_down_list = new DropDownList(*list_view, *this);
  m_drop_down_list->installEventFilter(this);
  auto window = m_drop_down_list->window();
  window->setWindowFlags(Qt::Popup | (window->windowFlags() & ~Qt::Tool));
  window->installEventFilter(this);
  m_submit_connection = list_view->connect_submit_signal(
    std::bind_front(&DropDownBox::on_submit, this));
}

void DropDownBox::show_drop_down_list() {
  make_drop_down_list();
  m_drop_down_list->window()->show();
  match(*this, PopUp());
}

void DropDownBox::hide_drop_down_list() {
  m_drop_down_list->hide();
  delete_later(m_drop_down_list);
  unmatch(*this, PopUp());
}

void DropDownBox::submit() {
  if(!m_is_modified) {
    return;
  }
  m_submission = m_current->get();
  if(m_submission) {
    m_is_modified = false;
    m_submit_signal(m_list->get(*m_submission));
  }
}

void DropDownBox::on_button_press_end(PressObserver::Reason reason) {
  if(reason == PressObserver::Reason::KEYBOARD) {
    if(is_drop_down_list_visible()) {
      hide_drop_down_list();
    } else if(!is_read_only()) {
      show_drop_down_list();
      m_drop_down_list->setFocus();
    }
  }
}

void DropDownBox::on_current(const optional<int>& current) {
  auto text = [&] {
    if(current) {
      return m_to_text(m_list->get(*current));
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
  hide_drop_down_list();
  submit();
}
