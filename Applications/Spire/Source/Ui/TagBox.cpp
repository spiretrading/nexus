#include "Spire/Ui/TagBox.hpp"
#include <QKeyEvent>
#include <QStringBuilder>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListModelTransactionLog.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/Tag.hpp"
#include "Spire/Ui/Tooltip.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto INPUT_BOX_STYLE(StyleSheet style) {
    style.get(Any()).set(padding(0));
    style.get(Any() >> is_a<ScrollableListBox>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_size(0));
    return style;
  }

  auto LIST_VIEW_STYLE(StyleSheet style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(ListItemGap(scale_width(4))).
      set(ListOverflowGap(scale_width(3))).
      set(Overflow::WRAP).
      set(Qt::Horizontal).
      set(vertical_padding(scale_height(3)));
    style.get(Any() > is_a<ListItem>()).
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
      m_focus_observer(*this),
      m_overflow(TagBoxOverflow::WRAP),
      m_is_read_only(false),
      m_tags_width(0),
      m_list_item_gap(0),
      m_input_box_horizontal_padding(0),
      m_list_view_horizontal_padding(0),
      m_scroll_bar_end_range(0),
      m_focus_connection(m_focus_observer.connect_state_signal(
        std::bind_front(&TagBox::on_focus, this))) {
  m_text_box = new TextBox(std::move(current));
  update_style(*m_text_box, [] (auto& style) {
    style = TEXT_BOX_STYLE(style);
  });
  m_text_box->get_current()->connect_update_signal(
    std::bind_front(&TagBox::on_text_box_current, this));
  m_text_box_style_connection = connect_style_signal(*m_text_box,
    std::bind_front(&TagBox::on_text_box_style, this));
  m_list_view = new ListView(m_model,
    std::bind_front(&TagBox::make_tag, this));
  m_list_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_list_view->set_item_size_policy(QSizePolicy::Preferred,
    QSizePolicy::Preferred);
  update_style(*m_list_view, [] (auto& style) {
    style = LIST_VIEW_STYLE(style);
  });
  m_list_view_style_connection = connect_style_signal(*m_list_view,
    std::bind_front(&TagBox::on_list_view_style, this));
  m_list_view->get_list()->connect_operation_signal(
    std::bind_front(&TagBox::on_operation, this));
  m_list_view->connect_submit_signal(
    std::bind_front(&TagBox::on_list_view_submit, this));
  m_list_view->setFocusPolicy(Qt::NoFocus);
  m_list_view->installEventFilter(this);
  auto scrollable_list_box = new ScrollableListBox(*m_list_view);
  scrollable_list_box->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  scrollable_list_box->setFocusPolicy(Qt::NoFocus);
  m_scroll_box = &scrollable_list_box->get_scroll_box();
  m_scroll_box->installEventFilter(this);
  m_vertical_scroll_bar = &m_scroll_box->get_vertical_scroll_bar();
  auto input_box = make_input_box(scrollable_list_box);
  enclose(*this, *input_box);
  proxy_style(*this, *input_box);
  set_style(*this, INPUT_BOX_STYLE(get_style(*input_box)));
  m_style_connection = connect_style_signal(*this, [=] { on_style(); });
  m_ellipses_item = m_list_view->get_list_item(get_list()->get_size());
  m_ellipses_item->hide();
  m_tooltip = new Tooltip("", this);
  setFocusProxy(m_text_box);
  setFocusPolicy(Qt::StrongFocus);
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
  m_placeholder = placeholder;
  update_placeholder();
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
  if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(watched == m_text_box->focusProxy() &&
        key_event.key() == Qt::Key_Backspace &&
        m_text_box->get_highlight()->get().m_start == 0 &&
        m_text_box->get_highlight()->get().m_end == 0 &&
        get_list()->get_size() > 0) {
      get_list()->remove(get_list()->get_size() - 1);
      return true;
    } else if(watched == m_list_view && (key_event.key() == Qt::Key_Down ||
        key_event.key() == Qt::Key_Up || key_event.key() == Qt::Key_PageDown ||
        key_event.key() == Qt::Key_PageUp)) {
      event->ignore();
      return true;
    }
  } else if(event->type() == QEvent::LayoutRequest) {
    if(watched == m_list_view) {
      if(m_overflow == TagBoxOverflow::ELIDE &&
          m_text_box->maximumWidth() == m_text_box->minimumWidth()) {
        m_list_view->get_list_item(m_model->get_size() - 1)->
          setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
      } else {
        m_list_view->get_list_item(m_model->get_size() - 1)->
          setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
      }
      update_tags_width();
    } else if(watched == m_scroll_box && !m_is_read_only &&
        m_focus_observer.get_state() != FocusObserver::State::NONE &&
        m_vertical_scroll_bar->get_range().m_end != m_scroll_bar_end_range) {
      scroll_to_end(*m_vertical_scroll_bar);
      m_scroll_bar_end_range = m_vertical_scroll_bar->get_range().m_end;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TagBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    update_tags_read_only();
    update_tooltip();
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
    update_style(*ellipses_box, [] (auto& style) {
      style = TEXT_BOX_STYLE(style);
    });
    return ellipses_box;
  }
  return m_text_box;
}

void TagBox::on_focus(FocusObserver::State state) {
  if(m_is_read_only) {
    return;
  }
  update();
  if(state != FocusObserver::State::NONE) {
    scroll_to_end(*m_vertical_scroll_bar);
    setFocus();
  }
}

void TagBox::on_operation(const AnyListModel::Operation& operation) {
  visit(operation,
    [&] (const AnyListModel::AddOperation& operation) {
      m_list_view->setFocusPolicy(Qt::NoFocus);
      if(m_text_box->focusPolicy() != Qt::StrongFocus) {
        m_text_box->setFocusPolicy(Qt::StrongFocus);
      }
      update_placeholder();
      update_tags_width();
      update_tip();
      update();
    },
    [&] (const AnyListModel::RemoveOperation& operation) {
      m_tags.erase(m_tags.begin() + operation.m_index);
      update_placeholder();
      update_tags_width();
      update_tip();
      update();
      if(m_focus_observer.get_state() != FocusObserver::State::NONE) {
        setFocus();
      }
    });
}

void TagBox::on_text_box_current(const QString& current) {
  m_list_view->adjustSize();
  scroll_to_end(*m_vertical_scroll_bar);
}

void TagBox::on_list_view_submit(const std::any& submission) {
  if(m_is_read_only) {
    return;
  }
  m_list_view->setFocusPolicy(Qt::NoFocus);
  if(m_text_box->focusPolicy() != Qt::StrongFocus) {
    m_text_box->setFocusPolicy(Qt::StrongFocus);
  }
  scroll_to_end(*m_vertical_scroll_bar);
  setFocus();
}

void TagBox::on_style() {
  m_input_box_horizontal_padding = 0;
  auto& stylist = find_stylist(*this);
  auto has_update = std::make_shared<bool>(false);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (EnumProperty<TagBoxOverflow> overflow) {
        stylist.evaluate(overflow, [=] (auto overflow) {
          if(m_overflow != overflow) {
            m_overflow = overflow;
            *has_update = true;
          }
        });
      },
      [&] (const BorderRightSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_input_box_horizontal_padding += size;
        });
      },
      [&] (const BorderLeftSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_input_box_horizontal_padding += size;
        });
      },
      [&] (const PaddingRight& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_input_box_horizontal_padding += size;
        });
      },
      [&] (const PaddingLeft& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_input_box_horizontal_padding += size;
        });
      });
  }
  update_tags_width();
  if(*has_update) {
    update_tip();
    update();
  }
}

void TagBox::on_list_view_style() {
  m_list_item_gap = 0;
  m_list_view_horizontal_padding = 0;
  auto& stylist = find_stylist(*m_list_view);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const ListItemGap& gap) {
        stylist.evaluate(gap, [=] (auto gap) {
          m_list_item_gap = gap;
        });
      },
      [&] (const PaddingRight& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_list_view_horizontal_padding += size;
        });
      },
      [&] (const PaddingLeft& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_list_view_horizontal_padding += size;
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

void TagBox::update() {
  if(m_overflow == TagBoxOverflow::ELIDE &&
      m_focus_observer.get_state() == FocusObserver::State::NONE) {
    update_style(*m_list_view, [] (auto& style) {
      style.get(Any()).set(Overflow::NONE);
    });
  } else {
    update_style(*m_list_view, [] (auto& style) {
      style.get(Any()).set(Overflow::WRAP);
    });
  }
  overflow();
}

void TagBox::update_placeholder() {
  if(m_tags.empty()) {
    m_text_box->set_placeholder(m_placeholder);
  } else {
    m_text_box->set_placeholder("");
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

void TagBox::update_tip() {
  if(m_overflow == TagBoxOverflow::ELIDE) {
    m_tip.clear();
    for(auto i = 0; i < get_list()->get_size(); ++i) {
      m_tip = m_tip % displayTextAny(get_list()->get(i)) % ", ";
    }
    m_tip.remove(m_tip.length() - 2, 2);
  }
}

void TagBox::update_tooltip() {
  if(isEnabled() && m_ellipses_item->isVisible()) {
    m_tooltip->set_label(m_tip);
  } else {
    m_tooltip->set_label("");
  }
}

void TagBox::overflow() {
  if(m_overflow == TagBoxOverflow::ELIDE &&
      m_focus_observer.get_state() == FocusObserver::State::NONE) {
    auto text_box_height = m_text_box->sizeHint().height();
    auto visible_area_width =
      width() - m_input_box_horizontal_padding - m_list_view_horizontal_padding;
    auto ellipses_width = m_ellipses_item->sizeHint().width();
    auto first_char_length = QFontMetrics(m_font).horizontalAdvance(
      m_text_box->get_current()->get(), 3);
    auto difference = m_tags_width + ellipses_width + m_list_item_gap +
      first_char_length - visible_area_width;
    if(difference <= 0) {
      show_all_tags();
      m_text_box->setFixedSize(visible_area_width - m_tags_width,
        text_box_height);
    } else {
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
      auto text_box_width = visible_area_width - visible_tags_width;
      if(text_box_width > 0) {
        m_text_box->setFixedSize(text_box_width, text_box_height);
      }
    }
  } else {
    show_all_tags();
    remove_text_box_size_constraint();
  }
  update_tooltip();
}

void TagBox::show_all_tags() {
  if(m_ellipses_item->isVisible()) {
    for(auto i = 0; i < get_list()->get_size(); ++i) {
      m_list_view->get_list_item(i)->show();
    }
    m_ellipses_item->hide();
  }
}

void TagBox::remove_text_box_size_constraint() {
  if(m_text_box->maximumWidth() == m_text_box->minimumWidth()) {
    m_text_box->setMinimumWidth(0);
    m_text_box->setMaximumWidth(QWIDGETSIZE_MAX);
  }
  if(m_text_box->maximumHeight() == m_text_box->minimumHeight()) {
    m_text_box->setMinimumHeight(0);
    m_text_box->setMaximumHeight(QWIDGETSIZE_MAX);
  }
}
