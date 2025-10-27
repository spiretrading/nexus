#include "Spire/Ui/TagBox.hpp"
#include <QKeyEvent>
#include <QStringBuilder>
#include <QTimer>
#include "Spire/Spire/AnyListValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/GlobalPositionObserver.hpp"
#include "Spire/Ui/InfoTip.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/Tag.hpp"
#include "Spire/Ui/TextAreaBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto INPUT_BOX_STYLE(StyleSheet style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(0));
    style.get(Any() > is_a<Box>() > is_a<ScrollableListBox>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_size(0));
    return style;
  }

  auto LIST_VIEW_STYLE(StyleSheet style) {
    style.get(Any()).
      set(ListItemGap(scale_width(4))).
      set(ListOverflowGap(scale_width(3))).
      set(Qt::Horizontal).
      set(PaddingRight(0)).
      set(vertical_padding(scale_height(3)));
    style.get(Any() > is_a<ListItem>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_size(0)).
      set(padding(0));
    return style;
  }

  auto TEXT_AREA_BOX_STYLE(StyleSheet style) {
    auto font = QFont("Roboto");
    font.setPixelSize(scale_width(11));
    style.get(ReadOnly()).
      set(Font(font)).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
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

  int horizontal_length(const QMargins& margins) {
    return margins.left() + margins.right();
  }

  int vertical_length(const QMargins& margins) {
    return margins.top() + margins.bottom();
  }

  int get_maximum_width(QWidget& widget) {
    if(widget.maximumWidth() != QWIDGETSIZE_MAX) {
      return widget.maximumWidth();
    }
    auto child = &widget;
    auto parent = child->parentWidget();
    while(parent && parent->layout() && parent->layout()->count() == 1 &&
        parent->layout()->indexOf(child) == 0 &&
        parent->layout()->contentsMargins().isNull()) {
      if(parent->maximumWidth() != QWIDGETSIZE_MAX) {
        return parent->maximumWidth();
      }
      child = parent;
      parent = parent->parentWidget();
    }
    return QWIDGETSIZE_MAX;
  }

  int get_maximum_height(QWidget& widget) {
    if(widget.maximumHeight() != QWIDGETSIZE_MAX) {
      return widget.maximumHeight();
    }
    auto child = &widget;
    auto parent = child->parentWidget();
    while(parent && parent->layout() && parent->layout()->count() == 1 &&
        parent->layout()->indexOf(child) == 0 &&
        parent->layout()->contentsMargins().isNull()) {
      if(parent->maximumHeight() != QWIDGETSIZE_MAX) {
        return parent->maximumHeight();
      }
      child = parent;
      parent = parent->parentWidget();
    }
    return QWIDGETSIZE_MAX;
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
    return m_source->get_size() + 1;
  }

  std::any at(int index) const override {
    if(index < 0 || index >= get_size()) {
      throw std::out_of_range("The index is out of range.");
    }
    if(index < m_source->get_size()) {
      return m_source->get(index);
    }
    return {};
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

  void transact(const std::function<void ()>& transaction) override {
    m_transaction.transact(transaction);
  }

  void on_operation(const Operation& operation) {
    m_transaction.push(
      reinterpret_cast<const ListModel<std::any>::Operation&>(operation));
  }
};

TagBox::TagBox(std::shared_ptr<AnyListModel> list,
    std::shared_ptr<TextModel> current, QWidget* parent)
    : QWidget(parent),
      m_model(std::make_shared<PartialListModel>(std::move(list))),
      m_text_focus_proxy(nullptr),
      m_focus_observer(*this),
      m_list_view_overflow(Overflow::NONE),
      m_is_read_only(false),
      m_list_item_gap(0),
      m_min_scroll_height(0),
      m_horizontal_scroll_bar_end_range(0),
      m_vertical_scroll_bar_end_range(0) {
  m_text_box = new TextBox(std::move(current));
  update_style(*m_text_box, [] (auto& style) {
    style = TEXT_BOX_STYLE(style);
  });
  m_text_box->get_current()->connect_update_signal(
    std::bind_front(&TagBox::on_text_box_current, this));
  m_list_view = new ListView(m_model, std::bind_front(&TagBox::make_tag, this));
  m_list_view->get_current()->set(m_model->get_size() - 1);
  update_style(*m_list_view, [] (auto& style) {
    style = LIST_VIEW_STYLE(style);
  });
  m_list_view_style_connection = connect_style_signal(
    *m_list_view, std::bind_front(&TagBox::on_list_view_style, this));
  m_list_view->get_list()->connect_operation_signal(
    std::bind_front(&TagBox::on_operation, this));
  m_list_view->connect_submit_signal(
    std::bind_front(&TagBox::on_list_view_submit, this));
  m_list_view->get_current()->connect_update_signal(
    std::bind_front(&TagBox::on_list_view_current, this));
  m_list_view->setFocusPolicy(Qt::NoFocus);
  m_list_view->installEventFilter(this);
  m_scrollable_list_box = new ScrollableListBox(*m_list_view);
  m_scrollable_list_box->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_scrollable_list_box->setFocusPolicy(Qt::NoFocus);
  m_scrollable_list_box->get_scroll_box().set_horizontal(
    ScrollBox::DisplayPolicy::NEVER);
  m_scrollable_list_box->get_scroll_box().installEventFilter(this);
  m_horizontal_scroll_bar =
    &m_scrollable_list_box->get_scroll_box().get_horizontal_scroll_bar();
  m_vertical_scroll_bar =
    &m_scrollable_list_box->get_scroll_box().get_vertical_scroll_bar();
  m_scrollable_list_box->installEventFilter(this);
  auto input_box = make_input_box(m_scrollable_list_box);
  enclose(*this, *input_box);
  link(*this, *m_list_view);
  proxy_style(*this, *input_box);
  set_style(*this, INPUT_BOX_STYLE(get_style(*input_box)));
  m_style_connection = connect_style_signal(*this,
    std::bind_front(&TagBox::on_style, this));
  m_text_area_box = new TextAreaBox("");
  m_text_area_box->set_read_only(true);
  m_text_area_box_style_connection = connect_style_signal(*m_text_area_box,
    std::bind_front(&TagBox::on_text_area_style, this));
  update_style(*m_text_area_box, [] (auto& style) {
    style = TEXT_AREA_BOX_STYLE(style);
  });
  m_info_tip = new InfoTip(m_text_area_box, this);
  update_tip();
  on_style();
  on_list_view_style();
  m_min_scroll_height = vertical_length(m_list_view_padding) +
    vertical_length(m_input_box_border) + vertical_length(m_input_box_padding) +
      m_text_box->sizeHint().height();
  setFocusProxy(m_text_box);
  setFocusPolicy(Qt::StrongFocus);
  m_focus_observer.connect_state_signal(
    std::bind_front(&TagBox::on_focus, this));
  m_text_box_position_observer =
    std::make_unique<GlobalPositionObserver>(*m_text_box);
  m_text_box_position_observer->connect_position_signal([=] (auto pos) {
    if(m_list_view_overflow == Styles::Overflow::NONE) {
      update_tooltip();
    }
  });
  update_overflow();
}

const std::shared_ptr<AnyListModel>& TagBox::get_tags() const {
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
    install_text_proxy_event_filter();
    unmatch(*this, ReadOnly());
  }
}

connection TagBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_text_box->connect_submit_signal(slot);
}

QSize TagBox::minimumSizeHint() const {
  return {QWidget::minimumSizeHint().width(), m_min_scroll_height};
}

QSize TagBox::sizeHint() const {
  if(m_size_hint) {
    return *m_size_hint;
  }
  auto list_view_size_hint = m_list_view->sizeHint();
  auto scroll_bar_size = [&] {
    if(m_vertical_scroll_bar->isVisible()) {
      return m_vertical_scroll_bar->sizeHint().width();
    }
    return 0;
  }();
  m_size_hint.emplace(
    list_view_size_hint.width() + horizontal_length(m_input_box_border) +
      horizontal_length(m_input_box_padding) + scroll_bar_size,
    list_view_size_hint.height() + vertical_length(m_input_box_border) +
      vertical_length(m_input_box_padding));
  return *m_size_hint;
}

bool TagBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(watched == m_text_focus_proxy && !is_read_only() &&
        key_event.key() == Qt::Key_Backspace && get_tags()->get_size() > 0 &&
          (m_text_box->get_highlight()->get().m_start == 0 &&
            m_text_box->get_highlight()->get().m_end == 0 ||
              m_text_box->get_current()->get().isEmpty())) {
      get_tags()->remove(get_tags()->get_size() - 1);
      return true;
    } else if(watched == m_list_view) {
      event->ignore();
      return true;
    }
  } else if(event->type() == QEvent::LayoutRequest && !m_is_read_only &&
      watched == &m_scrollable_list_box->get_scroll_box() &&
      find_focus_state(*this) != FocusObserver::State::NONE) {
    update_scroll_bar_end_range(*m_horizontal_scroll_bar,
      m_horizontal_scroll_bar_end_range);
    update_scroll_bar_end_range(*m_vertical_scroll_bar,
      m_vertical_scroll_bar_end_range);
  } else if(watched == m_list_view) {
    if(event->type() == QEvent::Resize) {
      update_tooltip();
    } else if(event->type() == QEvent::LayoutRequest) {
      update_vertical_scroll_bar_visible();
    }
  } else if(watched == m_scrollable_list_box && event->type() == QEvent::Show) {
    return true;
  }
  return QWidget::eventFilter(watched, event);
}

bool TagBox::event(QEvent* event) {
  if(event->type() == QEvent::LayoutRequest) {
    m_size_hint = none;
    update_overflow();
  }
  return QWidget::event(event);
}

void TagBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    update_tags_read_only();
    update_tooltip();
    update_vertical_scroll_bar_visible();
  }
  QWidget::changeEvent(event);
}

bool TagBox::focusNextPrevChild(bool next) {
  if(next) {
    m_text_box->setFocus(Qt::TabFocusReason);
  }
  return QWidget::focusNextPrevChild(next);
}

void TagBox::resizeEvent(QResizeEvent* event) {
  update_overflow();
  update_tooltip();
  update_vertical_scroll_bar_visible();
  QWidget::resizeEvent(event);
}

void TagBox::showEvent(QShowEvent* event) {
  install_text_proxy_event_filter();
  QWidget::showEvent(event);
}

QWidget* TagBox::make_tag(
    const std::shared_ptr<AnyListModel>& model, int index) {
  if(index == model->get_size() - 1) {
    auto box = new QWidget();
    enclose(*box, *m_text_box);
    connect(box, &QObject::destroyed, this, [=] {
      m_text_box->setParent(this);
    });
    return box;
  }
  auto label = make_read_only_to_text_model(
    std::make_shared<AnyListValueModel>(model, index));
  auto tag = new Tag(label, this);
  tag->set_read_only(m_is_read_only || !isEnabled());
  tag->connect_delete_signal([=] {
    QTimer::singleShot(0, this, [=] {
      auto tag_index = [&] {
        for(auto i = 0; i < get_tags()->get_size(); ++i) {
          if(label->get() == to_text(m_model->get(i))) {
            return i;
          }
        }
        return -1;
      }();
      setFocus();
      if(tag_index >= 0) {
        get_tags()->remove(tag_index);
      }
    });
  });
  return tag;
}

void TagBox::install_text_proxy_event_filter() {
  if(!m_text_focus_proxy) {
    m_text_focus_proxy = m_text_box->focusProxy();
    if(m_text_focus_proxy) {
      m_text_focus_proxy->installEventFilter(this);
    }
  }
}

void TagBox::scroll_to_text_box() {
  scroll_to_end(*m_horizontal_scroll_bar);
  scroll_to_end(*m_vertical_scroll_bar);
}

void TagBox::update_placeholder() {
  if(m_model->m_source->get_size() == 0) {
    m_text_box->set_placeholder(m_placeholder);
  } else {
    m_text_box->set_placeholder("");
  }
}

void TagBox::update_scroll_bar_end_range(ScrollBar& scroll_bar,
    int& end_range) {
  if(scroll_bar.get_range().m_end != end_range) {
    scroll_to_end(scroll_bar);
    end_range = scroll_bar.get_range().m_end;
  }
}

void TagBox::update_tag_size_policy() {
  if(m_list_view_overflow == Overflow::WRAP) {
    m_list_view->set_item_size_policy(QSizePolicy::Preferred,
      QSizePolicy::Minimum);
  } else {
    m_list_view->set_item_size_policy(QSizePolicy::Fixed, QSizePolicy::Minimum);
  }
}

void TagBox::update_tags_read_only() {
  auto is_read_only = m_is_read_only || !isEnabled();
  for(auto i = 0; i < m_model->m_source->get_size(); ++i) {
    auto& item = *m_list_view->get_list_item(i);
    if(item.is_mounted()) {
      static_cast<Tag*>(&item.get_body())->set_read_only(is_read_only);
    }
  }
}

void TagBox::update_tip() {
  static auto split_string = QString(" ") + QChar(0x2022) + " ";
  auto tip = QString();
  for(auto i = 0; i < get_tags()->get_size(); ++i) {
    tip = tip % to_text(get_tags()->get(i)) % split_string;
  }
  tip.remove(tip.length() - split_string.length(), split_string.length());
  m_text_area_box->get_current()->set(tip);
}

void TagBox::update_tooltip() {
  auto is_tag_overflow = [&] {
    if(m_list_view_overflow == Styles::Overflow::NONE) {
      auto scroll_bar_width = [&] {
        if(m_vertical_scroll_bar->isVisible()) {
          return m_vertical_scroll_bar->sizeHint().width();
        }
        return 0;
      }();
      if(m_model->m_source->get_size() > 0) {
        auto first_tag_left =
          m_list_view->get_list_item(0)->mapToGlobal(QPoint(0, 0)).x();
        auto last_tag = m_list_view->get_list_item(m_model->get_size() - 2);
        auto last_tag_right =
          last_tag->mapToGlobal(QPoint(last_tag->width() - 1, 0)).x();
        auto x = m_scrollable_list_box->mapToGlobal(QPoint(0, 0)).x();
        auto right = x + m_scrollable_list_box->width() - 1;
        if(first_tag_left < x || last_tag_right > right) {
          return true;
        }
      }
    } else {
      auto list_view_content_width =
        m_list_view->width() - horizontal_length(m_list_view_padding);
      for(auto i = 0; i < m_model->m_source->get_size(); ++i) {
        auto& tag = *m_list_view->get_list_item(i);
        if(tag.sizeHint().width() > list_view_content_width) {
          return true;
        }
      }
    }
    return false;
  }();
  if(isEnabled() && is_tag_overflow &&
      !m_text_area_box->get_current()->get().isEmpty()) {
    m_info_tip->setAttribute(Qt::WA_DontShowOnScreen, false);
    invalidate_descendant_layouts(*m_info_tip);
    m_info_tip->adjustSize();
  } else {
    m_info_tip->setAttribute(Qt::WA_DontShowOnScreen, true);
  }
}

void TagBox::update_overflow() {
  auto old_overflow = m_list_view_overflow;
  m_list_view_overflow = [&] {
    if(width() < get_maximum_width(*this)) {
      if(height() <= m_min_scroll_height) {
        return Overflow::NONE;
      }
      return Overflow::WRAP;
    } else if(height() <= m_min_scroll_height &&
        height() >= get_maximum_height(*this)) {
      return Overflow::NONE;
    }
    return Overflow::WRAP;
  }();
  if(old_overflow != m_list_view_overflow) {
    if(m_list_view_overflow == Overflow::NONE) {
      update_style(*this, [] (auto& style) {
        style.get(Any() > is_a<ListView>()).
          set(Overflow::NONE).
          set(PaddingRight(0));
        style.get(Any()).set(PaddingRight(scale_width(8)));
      });
      m_list_view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    } else {
      update_style(*this, [] (auto& style) {
        style.get(Any() > is_a<ListView>()).
          set(Overflow::WRAP).
          set(PaddingRight(scale_width(8)));
        style.get(Any()).set(PaddingRight(0));
      });
      m_list_view->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    }
    update_tag_size_policy();
  }
}

void TagBox::update_vertical_scroll_bar_visible() {
  if(m_list_view_overflow == Overflow::WRAP &&
      m_list_view->sizeHint().height() + vertical_length(m_input_box_padding) +
      vertical_length(m_input_box_border) > get_maximum_height(*this)) {
    m_scrollable_list_box->get_scroll_box().set_vertical(
      ScrollBox::DisplayPolicy::ON_OVERFLOW);
  } else {
    m_scrollable_list_box->get_scroll_box().set_vertical(
      ScrollBox::DisplayPolicy::NEVER);
  }
}

void TagBox::on_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::NONE) {
    scroll_to_start(*m_horizontal_scroll_bar);
  } else {
    m_text_box->setFocusPolicy(focusPolicy());
    setFocus();
    if(!is_read_only()) {
      QTimer::singleShot(0, this, [=] {
        m_list_view->get_current()->set(m_model->get_size() - 1);
      });
    }
  }
}

void TagBox::on_operation(const AnyListModel::Operation& operation) {
  auto update_all = [=] {
    m_list_view->setFocusPolicy(Qt::NoFocus);
    m_text_box->setFocusPolicy(focusPolicy());
    update_placeholder();
    update_tip();
    update_tooltip();
  };
  visit(operation,
    [&] (const AnyListModel::AddOperation&) {
      update_all();
    },
    [&] (const AnyListModel::RemoveOperation&) {
      update_all();
    },
    [&] (const AnyListModel::MoveOperation&) {
      update_tip();
      update_tooltip();
    },
    [&] (const AnyListModel::UpdateOperation&) {
      update_all();
    });
  QTimer::singleShot(0, this, [=] {
    if(m_list_view->get_current()->get() != m_model->get_size() - 1) {
      m_list_view->get_current()->set(m_model->get_size() - 1);
    }
    if(m_is_read_only) {
      scroll_to_start(*m_horizontal_scroll_bar);
    }
  });
}

void TagBox::on_text_box_current(const QString& current) {
  m_list_view->adjustSize();
  scroll_to_text_box();
}

void TagBox::on_list_view_current(const optional<int>& current) {
  if(current != m_model->get_size() - 1) {
    m_list_view->get_current()->set(m_model->get_size() - 1);
  }
}

void TagBox::on_list_view_submit(const std::any& submission) {
  m_list_view->setFocusPolicy(Qt::NoFocus);
  setFocus();
  if(!m_is_read_only) {
    scroll_to_text_box();
  } else {
    scroll_to_start(*m_horizontal_scroll_bar);
  }
}

void TagBox::on_style() {
  m_input_box_border = {};
  m_input_box_padding = {};
  auto& stylist = find_stylist(*this);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const BorderRightSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_input_box_border.setRight(size);
        });
      },
      [&] (const BorderLeftSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_input_box_border.setLeft(size);
        });
      },
      [&] (const BorderTopSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_input_box_border.setTop(size);
        });
      },
      [&] (const BorderBottomSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_input_box_border.setBottom(size);
        });
      },
      [&] (const PaddingRight& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_input_box_padding.setRight(size);
        });
      },
      [&] (const PaddingLeft& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_input_box_padding.setLeft(size);
        });
      },
      [&] (const PaddingTop& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_input_box_padding.setTop(size);
        });
      },
      [&] (const PaddingBottom& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_input_box_padding.setBottom(size);
        });
      });
  }
}

void TagBox::on_list_view_style() {
  m_list_view_padding = {};
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
          m_list_view_padding.setRight(size);
        });
      },
      [&] (const PaddingLeft& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_list_view_padding.setLeft(size);
        });
      },
      [&] (const PaddingBottom& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_list_view_padding.setBottom(size);
        });
      },
      [&] (const PaddingTop& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_list_view_padding.setTop(size);
        });
      });
  }
}

void TagBox::on_text_area_style() {
  auto text_font = std::make_shared<QFont>();
  auto horizontal_padding = std::make_shared<int>();
  auto& stylist = find_stylist(*m_text_area_box);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const Font& font) {
        stylist.evaluate(font, [=] (auto font) {
          *text_font = font;
        });
      },
      [&] (const FontSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          text_font->setPixelSize(size);
        });
      },
      [&] (const BorderBottomSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          *horizontal_padding += size;
        });
      },
      [&] (const BorderTopSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          *horizontal_padding += size;
        });
      },
      [&] (const PaddingBottom& size) {
        stylist.evaluate(size, [=] (auto size) {
          *horizontal_padding += size;
        });
      },
      [&] (const PaddingTop& size) {
        stylist.evaluate(size, [=] (auto size) {
          *horizontal_padding += size;
        });
      });
  }
  m_text_area_box->setMaximumWidth(
    50 * QFontMetrics(*text_font).averageCharWidth() + *horizontal_padding);
}
