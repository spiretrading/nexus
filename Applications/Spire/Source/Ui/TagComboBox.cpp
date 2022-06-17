#include "Spire/Ui/TagComboBox.hpp"
#include <QKeyEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  QWidget* find_pop_up_window(QWidget& widget) {
    for(auto child : widget.children()) {
      if(!child->isWidgetType()) {
        continue;
      }
      if(auto& widget = *static_cast<QWidget*>(child); widget.isWindow()) {
        return &widget;
      }
    }
    return nullptr;
  }

  void copy_list_model(const std::shared_ptr<AnyListModel>& from,
      const std::shared_ptr<AnyListModel>& to) {
    to->transact([&] {
      for(auto i = 0; i != from->get_size(); ++i) {
        if(i < to->get_size()) {
          if(!is_equal(from->get(i), to->get(i))) {
            to->insert(from->get(i), i);
          }
        } else {
          to->push(from->get(i));
        }
      }
      while(to->get_size() > from->get_size()) {
        to->remove(to->get_size() - 1);
      }
    });
  }
}

struct TagComboBoxQueryModel : ComboBox::QueryModel {
  std::shared_ptr<ComboBox::QueryModel> m_source;
  std::shared_ptr<AnyListModel> m_exclusions;
  std::unordered_set<QString> m_exclusion_set;
  scoped_connection m_connection;

  TagComboBoxQueryModel(std::shared_ptr<QueryModel> source,
      std::shared_ptr<AnyListModel> exclusions)
      : m_source(std::move(source)),
        m_exclusions(std::move(exclusions)),
        m_connection(m_exclusions->connect_operation_signal(
          std::bind_front(&TagComboBoxQueryModel::on_operation, this))) {
    for(auto i = 0; i < m_exclusions->get_size(); ++i) {
      m_exclusion_set.insert(displayText(m_exclusions->get(i)));
    }
  }

  std::any parse(const QString& query) override {
    auto value = m_source->parse(query);
    if(!value.has_value()) {
      return value;
    }
    if(m_exclusion_set.contains(displayText(value))) {
      static auto empty_value = std::any();
      return empty_value;
    }
    return value;
  }

  QtPromise<std::vector<std::any>> submit(const QString& query) override {
    return m_source->submit(query).then([=] (auto&& source_result) {
      auto& matches = [&] () -> std::vector<std::any>& {
        try {
          return source_result.Get();
        } catch(const std::exception&) {
          static auto empty_matches = std::vector<std::any>();
          return empty_matches;
        }
      }();
      std::erase_if(matches, [=] (auto& value) {
        return m_exclusion_set.contains(displayText(value));
      });
      return matches;
    });
  }

  void on_operation(const AnyListModel::Operation& operation) {
    visit(operation,
      [&] (const AnyListModel::AddOperation& operation) {
        m_exclusion_set.insert(displayText(operation.m_value));
      },
      [&] (const AnyListModel::RemoveOperation& operation) {
        m_exclusion_set.erase(displayText(operation.m_value));
      });
  }
};

TagComboBox::TagComboBox(std::shared_ptr<ComboBox::QueryModel> query_model,
  QWidget* parent)
  : TagComboBox(std::move(query_model), &ListView::default_view_builder,
      parent) {}

TagComboBox::TagComboBox(std::shared_ptr<ComboBox::QueryModel> query_model,
  ViewBuilder view_builder, QWidget* parent)
  : TagComboBox(std::move(query_model),
      std::make_shared<ArrayListModel<std::any>>(),
      std::move(view_builder), parent) {}

TagComboBox::TagComboBox(std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<AnyListModel> current, ViewBuilder view_builder,
    QWidget* parent)
    : QWidget(parent),
      m_submission(std::make_shared<ArrayListModel<std::any>>()),
      m_focus_observer(*this),
      m_input_box(nullptr),
      m_parent_window(nullptr),
      m_alignment(Alignment::NONE),
      m_last_alignment(m_alignment),
      m_is_modified(false),
      m_is_internal_move(false),
      m_is_in_layout(false),
      m_min_height(0),
      m_above_space(0),
      m_below_space(0),
      m_max_height(QWIDGETSIZE_MAX),
      m_focus_connection(m_focus_observer.connect_state_signal(
        std::bind_front(&TagComboBox::on_focus, this))) {
  m_tag_box = new TagBox(std::move(current), std::make_shared<LocalTextModel>());
  m_tag_box->installEventFilter(this);
  m_list_connection = m_tag_box->get_tags()->connect_operation_signal(
    std::bind_front(&TagComboBox::on_operation, this));
  m_tag_box_style_connection = connect_style_signal(*m_tag_box,
    std::bind_front(&TagComboBox::on_tag_box_style, this));
  m_combo_box = new ComboBox(
    std::make_shared<TagComboBoxQueryModel>(std::move(query_model),
    m_tag_box->get_tags()), std::make_shared<LocalValueModel<std::any>>(),
    new AnyInputBox(*m_tag_box), std::move(view_builder));
  m_combo_box->connect_submit_signal(
    std::bind_front(&TagComboBox::on_combo_box_submit, this));
  enclose(*this, *m_combo_box);
  setFocusProxy(m_combo_box);
  m_drop_down_window = find_pop_up_window(*m_combo_box);
  on_tag_box_style();
}

const std::shared_ptr<ComboBox::QueryModel>&
    TagComboBox::get_query_model() const {
  return m_combo_box->get_query_model();
}

const std::shared_ptr<AnyListModel>& TagComboBox::get_current() const {
  return m_tag_box->get_tags();
}

void TagComboBox::set_placeholder(const QString& placeholder) {
  m_combo_box->set_placeholder(placeholder);
}

bool TagComboBox::is_read_only() const {
  return m_combo_box->is_read_only();
}

void TagComboBox::set_read_only(bool is_read_only) {
  m_combo_box->set_read_only(is_read_only);
}

connection TagComboBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool TagComboBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_input_box && event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(key_event.key() == Qt::Key_Enter || key_event.key() == Qt::Key_Return) {
      if(key_event.text() != "\r") {
        return QWidget::eventFilter(watched, event);
      }
      if(!m_tag_box->get_current()->get().isEmpty()) {
        if(auto value = m_combo_box->get_query_model()->parse(
            m_tag_box->get_current()->get()); value.has_value()) {
          if(!is_equal(m_combo_box->get_current()->get(), value)) {
            m_combo_box->get_current()->set(value);
          }
          get_current()->push(value);
        }
        m_tag_box->get_current()->set("");
      }
      if(get_current()->get_size() > 0) {
        submit();
      }
      return true;
    } else if(key_event.key() == Qt::Key_Space) {
      if(m_combo_box->get_query_model()->parse(
          m_tag_box->get_current()->get()).has_value()) {
        auto new_event = QKeyEvent(key_event.type(), Qt::Key_Enter,
          key_event.modifiers(), key_event.text());
        QCoreApplication::sendEvent(m_input_box, &new_event);
        return true;
      }
    } else if(key_event.key() == Qt::Key_Escape) {
      m_tag_box->get_current()->set("");
      copy_list_model(m_submission, get_current());
      m_is_modified = false;
      return true;
    } else if(key_event.key() == Qt::Key_Down ||
        key_event.key() == Qt::Key_Up ||
        key_event.key() == Qt::Key_PageDown ||
        key_event.key() == Qt::Key_PageUp) {
      if(m_drop_down_window && m_drop_down_window->isVisible()) {
        QCoreApplication::sendEvent(m_combo_box, event);
        event->accept();
        return true;
      }
    }
  } else if(watched == m_input_box && event->type() == QEvent::FocusOut &&
      find_focus_state(*m_drop_down_window) != FocusObserver::State::NONE) {
    return true;
  } else if(watched == m_parent_window && event->type() == QEvent::Resize) {
    update_space();
    if(m_focus_observer.get_state() != FocusObserver::State::NONE) {
      m_alignment = Alignment::NONE;
      align();
      adjust_size();
    }
  } else if(watched == m_tag_box && event->type() == QEvent::LayoutRequest) {
    adjust_size();
  } else if(watched == parentWidget()) {
    if(event->type() == QEvent::Move) {
      if(!m_is_internal_move && m_parent_window) {
        update_position_and_space();
      }
    } else if(event->type() == QEvent::Resize) {
      adjust_size();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TagComboBox::showEvent(QShowEvent* event) {
  if(!m_input_box) {
    m_input_box = find_focus_proxy(*m_tag_box);
    m_input_box->installEventFilter(this);
  }
  if(!m_parent_window) {
    m_parent_window = window();
    m_parent_window->installEventFilter(this);
    m_bottom_left = mapToGlobal(rect().bottomLeft());
    update_position();
    update_space();
  }
  if(auto parent = parentWidget()) {
    if(auto layout = parent->layout()) {
      if(layout->indexOf(this) >= 0) {
        m_is_in_layout = true;
        parent->installEventFilter(this);
      }
    }
  }
  QWidget::showEvent(event);
}

void TagComboBox::moveEvent(QMoveEvent* event) {
  if(!m_is_internal_move && !m_is_in_layout && m_parent_window) {
    update_position_and_space();
  }
  QWidget::moveEvent(event);
}

void TagComboBox::resizeEvent(QResizeEvent* event) {
  if(m_focus_observer.get_state() == FocusObserver::State::NONE) {
    if(m_overflow == TagBoxOverflow::ELIDE) {
      auto old_min_height = m_min_height;
      m_min_height = event->size().height();
      if(old_min_height != m_min_height) {
        update_space();
      }
    }
  } else {
    align();
  }
  QWidget::resizeEvent(event);
}

void TagComboBox::on_combo_box_submit(const std::any& submission) {
  m_tag_box->get_current()->set("");
  get_current()->push(submission);
}

void TagComboBox::on_operation(const AnyListModel::Operation& operation) {
  visit(operation,
    [&] (const AnyListModel::AddOperation& operation) {
      m_is_modified = true;
    },
    [&] (const AnyListModel::RemoveOperation& operation) {
      m_is_modified = true;
    });
}

void TagComboBox::on_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::NONE) {
    m_tag_box->get_current()->set("");
    m_alignment = Alignment::NONE;
    set_position(m_position);
    update_space();
    if(m_is_modified && get_current()->get_size() > 0) {
      submit();
    }
  } else {
    align();
  }
}

void TagComboBox::on_tag_box_style() {
  auto& stylist = find_stylist(*m_tag_box);
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
      });
  }
  if(*has_update && m_overflow == TagBoxOverflow::WRAP) {
    m_min_height = sizeHint().height();
  }
}

void TagComboBox::align() {
  if(m_alignment == Alignment::ABOVE) {
    set_position({m_position.x(), m_position.y() + m_min_height - height()});
    return;
  }
  if(sizeHint().height() >= m_below_space && m_above_space > m_below_space) {
    m_alignment = Alignment::ABOVE;
    m_max_height = m_above_space;
    set_position({m_position.x(), m_position.y() + m_min_height - height()});
  } else {
    m_alignment = Alignment::BELOW;
    m_max_height = m_below_space;
    if(m_last_alignment != m_alignment) {
      set_position(m_position);
    }
  }
  m_last_alignment = m_alignment;
}

void TagComboBox::adjust_size() {
  if(minimumHeight() == maximumHeight()) {
    m_tag_box->setMaximumHeight(QWIDGETSIZE_MAX);
    setFixedHeight(maximumHeight());
    return;
  }
  auto height = [&] {
    auto max_height = std::min(m_max_height, maximumHeight());
    m_tag_box->setMaximumHeight(max_height);
    if(sizeHint().height() >= max_height) {
      return max_height;
    }
    return sizeHint().height();
  }();
  resize(width(), height);
}

void TagComboBox::set_position(const QPoint& pos) {
  m_is_internal_move = true;
  if(m_parent_window == parentWidget()) {
    move(pos);
  } else {
    move(parentWidget()->mapFromGlobal(m_parent_window->mapToGlobal(pos)));
  }
  m_is_internal_move = false;
}

void TagComboBox::submit() {
  copy_list_model(get_current(), m_submission);
  m_is_modified = false;
  m_submit_signal(m_submission);
}

void TagComboBox::update_position_and_space() {
  auto bottom_left = mapToGlobal(rect().bottomLeft());
  if(m_bottom_left != bottom_left) {
    m_bottom_left = bottom_left;
    update_position();
    update_space();
  }
}

void TagComboBox::update_position() {
  if(m_parent_window == parentWidget()) {
    m_position = pos();
  } else {
    m_position =
      m_parent_window->mapFromGlobal(parentWidget()->mapToGlobal(QPoint(0, 0)));
  }
}

void TagComboBox::update_space() {
  if(!m_parent_window) {
    return;
  }
  auto y = m_parent_window->mapToGlobal(m_position).y();
  auto& window_rect = m_parent_window->geometry();
  m_above_space = y + m_min_height - window_rect.y();
  m_below_space = window_rect.bottom() - y;
}
