#include "Spire/Ui/TagBox.hpp"
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/Tag.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto INPUT_BOX_STYLE(StyleSheet style) {
    style.get(Any()).
      set(vertical_padding(scale_height(3)));
    return style;
  }

  auto LIST_ITEM_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_size(0)).
      set(padding(0));
    return style;
  }

  auto LIST_VIEW_STYLE(StyleSheet style) {
    style.get(Any()).
      set(ListItemGap(scale_width(4))).
      set(ListOverflowGap(scale_width(3))).
      set(Overflow::WRAP).
      set(Qt::Horizontal);
    return style;
  }

  auto TEXT_BOX_STYLE(StyleSheet style) {
    style.get(Any() || Hover() || Focus() || Disabled()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_size(0)).
      set(horizontal_padding(0)).
      set(vertical_padding(scale_height(2)));
    return style;
  }
}

struct TagBox::PartialListModel : public ListModel {
  std::shared_ptr<ListModel> m_source;

  explicit PartialListModel(std::shared_ptr<ListModel> source)
    : m_source(std::move(source)) {}

  virtual int get_size() const {
    return m_source->get_size() + 2;
  }

  virtual const std::any& at(int index) const {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    if(index < m_source->get_size()) {
      return m_source->at(index);
    }
    static auto value = std::any();
    return value;
  }

  virtual QValidator::State set(int index, const std::any& value) {
    if(index < 0 || index >= get_size()) {
      return QValidator::State::Invalid;
    }
    if(index < m_source->get_size()) {
      return m_source->set(index, value);
    }
    return QValidator::State::Acceptable;
  }

  connection connect_operation_signal(
    const OperationSignal::slot_type& slot) const override {
    return m_source->connect_operation_signal(slot);
  }
};

TagBox::TagBox(std::shared_ptr<ListModel> list_model,
    std::shared_ptr<TextModel> current_model, QWidget* parent)
    : QWidget(parent),
      m_model(std::make_shared<PartialListModel>(std::move(list_model))),
      m_focus_observer(*this),
      m_tags_width(0),
      m_list_item_gap(0) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_text_box = new TextBox(std::move(current_model));
  set_style(*m_text_box, TEXT_BOX_STYLE(get_style(*m_text_box)));
  m_text_box->installEventFilter(this);
  m_list_view = new ListView(m_model,
    std::bind_front(&TagBox::build_tag, this));
  set_style(*m_list_view, LIST_VIEW_STYLE(get_style(*m_list_view)));
  m_list_view_style_connection = connect_style_signal(*m_list_view,
    std::bind_front(&TagBox::on_list_view_style, this));
  for(auto i = 0; i < m_list_view->get_list()->get_size(); ++i) {
    set_style(*m_list_view->get_list_item(i), LIST_ITEM_STYLE());
  }
  m_list_view->get_list()->connect_operation_signal(
    std::bind_front(&TagBox::on_operation, this));
  m_list_view->connect_submit_signal(std::bind_front(&TagBox::on_submit, this));
  m_list_view->setFocusPolicy(Qt::NoFocus);
  m_list_view->installEventFilter(this);
  m_list_view_container = new QWidget();
  m_container_layout = new QHBoxLayout(m_list_view_container);
  m_container_layout->setContentsMargins({});
  m_container_layout->setSpacing(0);
  m_container_layout->addWidget(m_list_view, 0, Qt::AlignLeft);
  auto input_box = make_input_box(m_list_view_container);
  layout->addWidget(input_box);
  proxy_style(*this, *input_box);
  set_style(*this, INPUT_BOX_STYLE(get_style(*input_box)));
  m_style_connection = connect_style_signal(*this, [=] { on_style(); });
  m_ellipses_item = m_list_view->get_list_item(get_list()->get_size());
  m_ellipses_item->hide();
  setFocusProxy(m_text_box);
  setFocusPolicy(Qt::StrongFocus);
  m_line_edit = m_text_box->findChild<QLineEdit*>();
  connect(m_line_edit, &QLineEdit::cursorPositionChanged,
    std::bind_front(&TagBox::reposition_list_view, this));
  m_focus_connection = m_focus_observer.connect_state_signal(
    std::bind_front(&TagBox::on_focus, this));
}

const std::shared_ptr<ListModel>& TagBox::get_list() const {
  return m_model->m_source;
}

const std::shared_ptr<TextModel>& TagBox::get_current() const {
  return m_text_box->get_current();
}

bool TagBox::is_read_only() const {
  return m_text_box->is_read_only();
}

void TagBox::set_read_only(bool is_read_only) {
  m_text_box->set_read_only(is_read_only);
  update_uneditable();
  if(is_read_only) {
    match(*this, ReadOnly());
    match(*m_list_view, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
    unmatch(*m_list_view, ReadOnly());
  }
}

connection TagBox::connect_delete_signal(
    const DeleteSignal::slot_type& slot) const {
  return m_delete_signal.connect(slot);
}

bool TagBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_text_box && event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    switch(key_event.key()) {
      case Qt::Key_Backspace:
        if(m_line_edit->cursorPosition() == 0 && get_list()->get_size() > 0) {
          m_delete_signal(get_list()->get_size() - 1);
        }
        break;
      case Qt::Key_Down:
      case Qt::Key_Up:
      case Qt::Key_PageDown:
      case Qt::Key_PageUp:
        return true;
      default:
        break;
    }
  } else if(watched == m_list_view && event->type() == QEvent::Resize) {
    reposition_list_view();
  }
  return QWidget::eventFilter(watched, event);
}

void TagBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    update_uneditable();
  }
  QWidget::changeEvent(event);
}

void TagBox::resizeEvent(QResizeEvent* event) {
  overflow();
  QWidget::resizeEvent(event);
}

QWidget* TagBox::build_tag(const std::shared_ptr<ListModel>& model, int index) {
  if(index < get_list()->get_size()) {
    auto label = displayTextAny(model->at(index));
    auto tag = new Tag(label, this);
    tag->set_read_only(m_text_box->is_read_only() || !isEnabled());
    tag->connect_delete_signal([=] {
      auto tag_index = [&] {
        for(auto i = 0; i < get_list()->get_size(); ++i) {
          if(label == displayTextAny(model->at(i))) {
            return i;
          }
        }
        return -1;
      }();
      if(tag_index >= 0) {
        m_delete_signal(tag_index);
      }
    });
    connect(tag, &QWidget::destroyed, [=] {
      if(m_focus_observer.get_state() != FocusObserver::State::NONE) {
        setFocus();
      }
    });
    m_tags.emplace_back(tag);
    return tag;
  } else if(index == get_list()->get_size()) {
    auto ellipses_box = make_label(QObject::tr("..."));
    set_style(*ellipses_box, TEXT_BOX_STYLE(get_style(*ellipses_box)));
    return ellipses_box;
  }
  return m_text_box;
}

void TagBox::on_focus(FocusObserver::State state) {
  overflow();
  if(state != FocusObserver::State::NONE) {
    m_text_box->setFocus();
  }
}

void TagBox::on_operation(const ListModel::Operation& operation) {
  visit(operation,
    [&] (const ListModel::AddOperation& operation) {
      auto item = m_list_view->get_list_item(operation.m_index);
      set_style(*item, LIST_ITEM_STYLE());
      item->setFocusPolicy(Qt::NoFocus);
      m_list_view->setFocusPolicy(Qt::NoFocus);
      if(m_text_box->focusPolicy() != Qt::StrongFocus) {
        m_text_box->setFocusPolicy(Qt::StrongFocus);
      }
      remove_text_box_width_constraint();
      update_tags_width();
      overflow();
    },
    [&] (const ListModel::RemoveOperation& operation) {
      m_tags.erase(m_tags.begin() + operation.m_index);
      remove_text_box_width_constraint();
      update_tags_width();
      overflow();
      if(m_focus_observer.get_state() != FocusObserver::State::NONE) {
        setFocus();
      }
    });
}

void TagBox::on_submit(const std::any& submission) {
  if(m_text_box->focusPolicy() != Qt::StrongFocus) {
    m_text_box->setFocusPolicy(Qt::StrongFocus);
  }
  setFocus();
}

void TagBox::on_style() {
  m_margins = {};
  auto& stylist = find_stylist(*this);
  auto has_update = std::make_shared<bool>(false);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (EnumProperty<TagBoxOverflow> overflow) {
        stylist.evaluate(overflow, [=] (auto overflow) {
          m_overflow = overflow;
          *has_update = true;
        });
      },
      [&] (const BorderRightSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_margins.setRight(m_margins.right() + size);
        });
      },
      [&] (const BorderLeftSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_margins.setLeft(m_margins.left() + size);
        });
      },
      [&] (const PaddingRight& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_margins.setRight(m_margins.right() + size);
        });
      },
      [&] (const PaddingLeft& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_margins.setLeft(m_margins.left() + size);
        });
      });
  }
  if(has_update) {
    if(m_overflow == TagBoxOverflow::ELIDE) {
      update_style(*m_list_view, [] (auto& style) {
        style.get(Any()).set(Overflow::NONE);
      });
      m_list_view->layout()->setSizeConstraint(QLayout::SetFixedSize);
      m_list_view->updateGeometry();
      update_tags_width();
    } else {
      update_style(*m_list_view, [] (auto& style) {
        style.get(Any()).set(Overflow::WRAP);
      });
      m_list_view->layout()->setSizeConstraint(QLayout::SetDefaultConstraint);
      m_list_view->setMinimumSize(0, 0);
      m_list_view->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
      m_list_view->updateGeometry();
    }
  }
}

void TagBox::on_list_view_style() {
  m_list_item_gap = 0;
  auto& stylist = find_stylist(*m_list_view);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const ListItemGap& gap) {
        stylist.evaluate(gap, [=] (auto gap) {
          m_list_item_gap = gap;
        });
      });
  }

}

void TagBox::update_uneditable() {
  auto is_uneditable = m_text_box->is_read_only() || !isEnabled();
  for(auto tag : m_tags) {
    tag->set_read_only(is_uneditable);
  }
}

void TagBox::update_tags_width() {
  if(m_overflow == TagBoxOverflow::ELIDE) {
    m_tags_width = 0;
    for(auto i = 0; i < get_list()->get_size(); ++i) {
      m_tags_width += m_list_view->get_list_item(i)->sizeHint().width()
        + m_list_item_gap;
    }
  }
}

void TagBox::overflow() {
  if(m_overflow == TagBoxOverflow::ELIDE &&
      m_focus_observer.get_state() == FocusObserver::State::NONE) {
    add_list_view_to_layout();
    auto visible_area_width = width() - m_margins.left() - m_margins.right();
    auto ellipses_width = m_ellipses_item->sizeHint().width();
    auto text_box_with = m_text_box->sizeHint().width();
    auto first_char_length = m_line_edit->fontMetrics().horizontalAdvance(
      m_text_box->get_current()->get(), 1);
    auto difference = m_tags_width + first_char_length + ellipses_width +
      ellipses_width + m_list_item_gap - visible_area_width;
    if(difference <= 0) {
      show_all_tags();
      m_text_box->setFixedWidth(visible_area_width - m_tags_width);
      return;
    }
    auto length = 0;
    bool is_tag_hidden = false;
    auto i = get_list()->get_size() - 1;
    while(i >= 0 && length <= difference) {
      auto item = m_list_view->get_list_item(i);
      length += item->sizeHint().width() + m_list_item_gap;
      item->hide();
      is_tag_hidden = true;
      --i;
    }
    auto visible_tags_width = 0;
    while(i >= 0) {
      auto item = m_list_view->get_list_item(i);
      item->show();
      visible_tags_width += item->sizeHint().width() + m_list_item_gap;
      --i;
    }
    if(is_tag_hidden) {
      m_ellipses_item->show();
      visible_tags_width += ellipses_width + m_list_item_gap;
    } else {
      m_ellipses_item->hide();
    }
    m_text_box->setFixedWidth(visible_area_width - visible_tags_width);
  } else {
    show_all_tags();
    remove_text_box_width_constraint();
  }
}

void TagBox::reposition_list_view() {
  if(m_overflow == TagBoxOverflow::ELIDE &&
      m_focus_observer.get_state() == FocusObserver::State::FOCUS_IN) {
    auto text_length = m_line_edit->fontMetrics().horizontalAdvance(
      m_text_box->get_current()->get(), -1);
    auto cursor_pos = [=] {
      if(text_length > 0) {
        return m_line_edit->fontMetrics().horizontalAdvance(
          m_text_box->get_current()->get(), m_line_edit->cursorPosition() + 1);
      }
      return 1;
    }();
    auto text_box_pos = mapFromGlobal(m_text_box->mapToGlobal(QPoint()));
    if(text_box_pos.x() + cursor_pos > width() - m_margins.right()) {
      remove_list_view_from_layout();
      auto offset = width() - m_margins.right() - text_box_pos.x() - cursor_pos;
      m_list_view->move(m_list_view->x() + offset, m_list_view->y());
    }
    if(text_box_pos.x() + cursor_pos < m_margins.left()) {
      auto offset = m_margins.left() - text_box_pos.x() - cursor_pos;
      m_list_view->move(m_list_view->x() + offset, m_list_view->y());
    }
    if(text_box_pos.x() + text_length < width() - m_margins.right()) {
      auto list_view_width = m_list_view->sizeHint().width();
      auto body_width = width() - m_margins.left() - m_margins.right();
      if(list_view_width <= body_width) {
        add_list_view_to_layout();
      } else {
        if(text_length == 0) {
          text_length = 1;
        }
        auto offset = width() - m_margins.right() - text_box_pos.x() -
          text_length;
        m_list_view->move(m_list_view->x() + offset, m_list_view->y());
      }
    }
  }
}

void TagBox::show_all_tags() {
  if(m_ellipses_item->isVisible()) {
    m_ellipses_item->hide();
    for(auto i = get_list()->get_size() - 1; i >= 0; --i) {
      if(m_ellipses_item->isVisible()) {
        break;
      }
      m_list_view->get_list_item(i)->show();
    }
  }
}

void TagBox::add_list_view_to_layout() {
  if(m_container_layout->isEmpty()) {
    m_list_view_container->setMinimumSize(0, 0);
    m_list_view_container->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    m_container_layout->addWidget(m_list_view, 0, Qt::AlignLeft);
  }
}

void TagBox::remove_list_view_from_layout() {
  if(!m_container_layout->isEmpty()) {
    m_container_layout->removeWidget(m_list_view);
    m_list_view_container->layout()->removeWidget(m_list_view);
    m_list_view_container->setFixedSize(m_list_view_container->size());
    m_list_view->setParent(m_list_view_container);
    m_list_view->show();
  }
}

void TagBox::remove_text_box_width_constraint() {
  if(m_text_box->maximumWidth() == m_text_box->minimumWidth()) {
    m_text_box->setMinimumWidth(0);
    m_text_box->setMaximumWidth(QWIDGETSIZE_MAX);
  }
}