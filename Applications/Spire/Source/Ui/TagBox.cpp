#include "Spire/Ui/TagBox.hpp"
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListModelTransactionLog.hpp"
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

  auto LIST_VIEW_STYLE(StyleSheet style) {
    style.get(Any()).
      set(ListItemGap(scale_width(4))).
      set(ListOverflowGap(scale_width(3))).
      set(Overflow::WRAP).
      set(Qt::Horizontal);
    style.get(Any() >> is_a<ListItem>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_size(0)).
      set(padding(0));
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

struct TagBox::PartialListModel : public AnyListModel {
  std::shared_ptr<AnyListModel> m_source;
  ListModelTransactionLog<std::any> m_transaction;
  scoped_connection m_source_connection;

  explicit PartialListModel(std::shared_ptr<AnyListModel> source)
    : m_source(std::move(source)),
      m_source_connection(m_source->connect_operation_signal(
        std::bind_front(&PartialListModel::on_operation, this))) {}

  int get_size() const override {
    return m_source->get_size() + 2;
  }

  std::any at(int index) const override {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    if(index < m_source->get_size()) {
      return m_source->get(index);
    }
    static auto value = std::any();
    return value;
  }

  QValidator::State set(int index, const std::any& value) override {
    if(index < 0 || index >= m_source->get_size()) {
      return QValidator::State::Invalid;
    }
    return m_source->set(index, value);
  }

  QValidator::State insert(const std::any& value, int index) override {
    return QValidator::State::Invalid;
  }

  QValidator::State move(int source, int destination) override {
    return QValidator::State::Invalid;
  }

  QValidator::State remove(int index) override {
    return QValidator::State::Invalid;
  }

  connection connect_operation_signal(
      const OperationSignal::slot_type& slot) const override {
    return m_transaction.connect_operation_signal(slot);
  }

  void on_operation(const Operation& operation) {
    m_transaction.push(Operation(operation));
  }
};

TagBox::TagBox(std::shared_ptr<AnyListModel> list,
    std::shared_ptr<TextModel> current, QWidget* parent)
    : QWidget(parent),
      m_model(std::make_shared<PartialListModel>(std::move(list))),
      m_is_read_only(false),
      m_focus_observer(*this),
      m_overflow(TagBoxOverflow::WRAP),
      m_tags_width(0),
      m_list_item_gap(0) {
  m_text_box = new TextBox(std::move(current));
  set_style(*m_text_box, TEXT_BOX_STYLE(get_style(*m_text_box)));
  m_text_box_style_connection = connect_style_signal(*m_text_box,
    std::bind_front(&TagBox::on_text_box_style, this));
  m_highlight_connection = m_text_box->get_highlight()->connect_update_signal(
    [=] (const Highlight&) { reposition_list_view(); });
  m_list_view = new ListView(m_model,
    std::bind_front(&TagBox::make_tag, this));
  m_list_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*m_list_view, [] (auto& style) {
    style = LIST_VIEW_STYLE(style);
  });
  m_list_view_style_connection = connect_style_signal(*m_list_view,
    std::bind_front(&TagBox::on_list_view_style, this));
  m_list_view->get_list()->connect_operation_signal(
    std::bind_front(&TagBox::on_operation, this));
  m_list_view->connect_submit_signal(std::bind_front(&TagBox::on_submit, this));
  m_list_view->setFocusPolicy(Qt::NoFocus);
  m_list_view->installEventFilter(this);
  m_list_view_container = new QWidget();
  auto container_layout = new QHBoxLayout(m_list_view_container);
  container_layout->setContentsMargins({});
  container_layout->setSpacing(0);
  container_layout->addWidget(m_list_view);
  auto input_box = make_input_box(m_list_view_container);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  layout->addWidget(input_box);
  proxy_style(*this, *input_box);
  set_style(*this, INPUT_BOX_STYLE(get_style(*input_box)));
  m_style_connection = connect_style_signal(*this, [=] { on_style(); });
  m_ellipses_item = m_list_view->get_list_item(get_list()->get_size());
  m_ellipses_item->hide();
  setFocusProxy(m_text_box);
  setFocusPolicy(Qt::StrongFocus);
  m_focus_connection = m_focus_observer.connect_state_signal(
    std::bind_front(&TagBox::on_focus, this));
}

const std::shared_ptr<AnyListModel>& TagBox::get_list() const {
  return m_model->m_source;
}

const std::shared_ptr<TextModel>& TagBox::get_current() const {
  return m_text_box->get_current();
}

const std::shared_ptr<HighlightModel>& TagBox::get_highlight() const {
  return m_text_box->get_highlight();
}

void TagBox::set_placeholder(const QString& placeholder) {
  m_text_box->set_placeholder(placeholder);
}

bool TagBox::is_read_only() const {
  return m_is_read_only;
}

void TagBox::set_read_only(bool is_read_only) {
  if(m_is_read_only == is_read_only) {
    return;
  }
  m_is_read_only = is_read_only;
  m_text_box->set_read_only(m_is_read_only);
  update_tags_read_only();
  if(m_is_read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
}

connection TagBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_text_box->connect_submit_signal(slot);
}

bool TagBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_text_box->focusProxy() && event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    switch(key_event.key()) {
      case Qt::Key_Backspace:
        if(m_text_box->get_highlight()->get().m_start == 0 &&
            m_text_box->get_highlight()->get().m_end == 0 &&
            get_list()->get_size() > 0) {
          get_list()->remove(get_list()->get_size() - 1);
        }
        break;
      case Qt::Key_Down:
      case Qt::Key_Up:
      case Qt::Key_PageDown:
      case Qt::Key_PageUp:
        QCoreApplication::sendEvent(this, event);
        return true;
      default:
        break;
    }
  } else if(watched == m_list_view && event->type() == QEvent::Resize) {
    update_tags_width();
    overflow();
    reposition_list_view();
  }
  return QWidget::eventFilter(watched, event);
}

void TagBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    update_tags_read_only();
  }
  QWidget::changeEvent(event);
}

void TagBox::resizeEvent(QResizeEvent* event) {
  overflow();
  QWidget::resizeEvent(event);
}

void TagBox::showEvent(QShowEvent* event) {
  m_text_box->focusProxy()->installEventFilter(this);
  QWidget::showEvent(event);
}

QWidget* TagBox::make_tag(
    const std::shared_ptr<AnyListModel>& model, int index) {
  if(index < model->get_size() - 2) {
    auto label = displayTextAny(model->get(index));
    auto tag = new Tag(label, this);
    tag->set_read_only(m_is_read_only || !isEnabled());
    tag->connect_delete_signal([=] {
      auto tag_index = [&] {
        for(auto i = 0; i < get_list()->get_size(); ++i) {
          if(label == displayTextAny(m_model->get(i))) {
            return i;
          }
        }
        return -1;
      }();
      if(tag_index >= 0) {
        get_list()->remove(tag_index);
      }
    });
    connect(tag, &QWidget::destroyed, [=] {
      if(m_focus_observer.get_state() != FocusObserver::State::NONE) {
        setFocus();
      }
    });
    m_tags.emplace_back(tag);
    return tag;
  } else if(index == model->get_size() - 2) {
    auto ellipses_box = make_label(tr("..."));
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

void TagBox::on_operation(const AnyListModel::Operation& operation) {
  visit(operation,
    [&] (const AnyListModel::AddOperation& operation) {
      m_list_view->setFocusPolicy(Qt::NoFocus);
      if(m_text_box->focusPolicy() != Qt::StrongFocus) {
        m_text_box->setFocusPolicy(Qt::StrongFocus);
      }
      remove_text_box_width_constraint();
      update_tags_width();
      overflow();
    },
    [&] (const AnyListModel::RemoveOperation& operation) {
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
  m_list_view->setFocusPolicy(Qt::NoFocus);
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
      m_list_view_container->layout()->setAlignment(m_list_view, Qt::AlignLeft);
    } else {
      update_style(*m_list_view, [] (auto& style) {
        style.get(Any()).set(Overflow::WRAP);
      });
      m_list_view->layout()->setSizeConstraint(QLayout::SetDefaultConstraint);
      m_list_view->setMinimumSize(0, 0);
      m_list_view->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
      m_list_view->updateGeometry();
      m_list_view_container->layout()->setAlignment(m_list_view,
        Qt::Alignment());
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

void TagBox::on_text_box_style() {
  m_font = {};
  auto& stylist = find_stylist(*m_text_box);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const Font& font) {
        stylist.evaluate(font, [=] (auto font) {
          m_font = font;
        });
      },
      [&] (const FontSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_font.setPixelSize(size);
        });
      });
  }
}

void TagBox::update_tags_read_only() {
  auto is_read_only = m_is_read_only || !isEnabled();
  for(auto tag : m_tags) {
    tag->set_read_only(is_read_only);
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
    auto first_char_length = QFontMetrics(m_font).horizontalAdvance(
      m_text_box->get_current()->get(), 1);
    auto difference = m_tags_width + ellipses_width + m_list_item_gap +
      first_char_length - visible_area_width;
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
    auto text_length = QFontMetrics(m_font).horizontalAdvance(
      m_text_box->get_current()->get(), -1);
    auto cursor_pos = [&] {
      if(text_length > 0) {
        return QFontMetrics(m_font).horizontalAdvance(
          m_text_box->get_current()->get(),
          m_text_box->get_highlight()->get().m_end + 1);
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
  if(m_list_view_container->layout()->isEmpty()) {
    m_list_view_container->setMinimumSize(0, 0);
    m_list_view_container->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    m_list_view_container->layout()->addWidget(m_list_view);
    m_list_view_container->layout()->setAlignment(m_list_view, Qt::AlignLeft);
  }
}

void TagBox::remove_list_view_from_layout() {
  if(!m_list_view_container->layout()->isEmpty()) {
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
