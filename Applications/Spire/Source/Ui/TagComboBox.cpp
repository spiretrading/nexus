#include "Spire/Ui/TagComboBox.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  QWidget* find_drop_down_window(QWidget& widget) {
    auto& children = widget.children();
    for(auto child : children) {
      if(!child->isWidgetType()) {
        continue;
      }
      auto& widget = *static_cast<QWidget*>(child);
      if(widget.windowFlags() & Qt::Tool) {
        return &widget;
      }
    }
    return nullptr;
  }

  void copy_list_model(const std::shared_ptr<AnyListModel>& from,
      const std::shared_ptr<AnyListModel>& to) {
    while(to->get_size() != 0) {
      to->remove(to->get_size() - 1);
    }
    for(auto i = 0; i < from->get_size(); ++i) {
      to->push(from->get(i));
    }
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
      return std::any();
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
      std::erase_if(matches, [&] (auto& value) {
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
      std::make_shared<LocalValueModel<std::any>>(),
      std::move(view_builder), parent) {}

TagComboBox::TagComboBox(std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<AnyListModel> list, std::shared_ptr<CurrentModel> current,
    ViewBuilder view_builder, QWidget* parent)
    : QWidget(parent),
      m_submission(std::move(list)),
      m_focus_proxy(nullptr),
      m_window(nullptr),
      m_focus_observer(*this),
      m_alignment(Alignment::NONE),
      m_is_modified(false),
      m_is_internal_move(false),
      m_min_height(0),
      m_above_space(0),
      m_below_space(0),
      m_focus_connection(m_focus_observer.connect_state_signal(
        std::bind_front(&TagComboBox::on_focus, this))) {
  m_tag_box = new TagBox(std::make_shared<ArrayListModel<std::any>>(),
    std::make_shared<LocalTextModel>());
  m_list_connection = m_tag_box->get_list()->connect_operation_signal(
    std::bind_front(&TagComboBox::on_operation, this));
  m_tag_box_style_connection = connect_style_signal(*m_tag_box,
    std::bind_front(&TagComboBox::on_tag_box_style, this));
  m_combo_box = new ComboBox(
    std::make_shared<TagComboBoxQueryModel>(std::move(query_model),
    m_tag_box->get_list()), std::move(current),
    new AnyInputBox(*m_tag_box), std::move(view_builder));
  m_drop_down_window = find_drop_down_window(*m_combo_box);
  m_combo_box->connect_submit_signal(
    std::bind_front(&TagComboBox::on_combo_box_submit, this));
  enclose(*this, *m_combo_box);
  setFocusProxy(m_combo_box);
}

const std::shared_ptr<ComboBox::QueryModel>&
    TagComboBox::get_query_model() const {
  return m_combo_box->get_query_model();
}

const std::shared_ptr<AnyListModel>& TagComboBox::get_list() const {
  return m_submission;
}

const std::shared_ptr<TagComboBox::CurrentModel>&
    TagComboBox::get_current() const {
  return m_combo_box->get_current();
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
  if(watched == m_focus_proxy && event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(key_event.key() == Qt::Key_Enter || key_event.key() == Qt::Key_Return) {
      if(key_event.text() == " ") {
        return QWidget::eventFilter(watched, event);
      }
      if(!m_tag_box->get_current()->get().isEmpty()) {
        if(auto value = m_combo_box->get_query_model()->parse(
            m_tag_box->get_current()->get()); value.has_value()) {
          if(!is_equal(m_combo_box->get_current()->get(), value)) {
            m_combo_box->get_current()->set(value);
          }
          m_tag_box->get_list()->push(value);
        }
        m_tag_box->get_current()->set("");
      }
      submit();
      return true;
    } else if(key_event.key() == Qt::Key_Space) {
      if(m_combo_box->get_query_model()->parse(
          m_tag_box->get_current()->get()).has_value()) {
        auto new_event = QKeyEvent(key_event.type(), Qt::Key_Enter,
          key_event.modifiers(), key_event.text());
        QCoreApplication::sendEvent(m_focus_proxy, &new_event);
        return true;
      }
    } else if(key_event.key() == Qt::Key_Escape) {
      m_tag_box->get_current()->set("");
      copy_list_model(m_submission, m_tag_box->get_list());
      m_is_modified = false;
      return true;
    } else if(key_event.key() == Qt::Key_Down ||
        key_event.key() == Qt::Key_Up ||
        key_event.key() == Qt::Key_PageDown ||
        key_event.key() == Qt::Key_PageUp) {
      if(m_drop_down_window->isVisible()) {
        QCoreApplication::sendEvent(m_combo_box, event);
        event->accept();
        return true;
      }
    }
  } else if(watched == m_window && event->type() == QEvent::Resize) {
    update_space();
    if(m_focus_observer.get_state() != FocusObserver::State::NONE) {
      m_alignment = Alignment::NONE;
      align();
    }
  }
  return QWidget::eventFilter(watched, event);
}

bool TagComboBox::event(QEvent* event) {
  if(event->type() == QEvent::LayoutRequest) {
    adjustSize();
  }
  return QWidget::event(event);
}

void TagComboBox::showEvent(QShowEvent* event) {
  if(!m_focus_proxy) {
    m_focus_proxy = find_focus_proxy(*m_tag_box);
    m_focus_proxy->installEventFilter(this);
  }
  if(!m_window) {
    m_window = window();
    m_window->installEventFilter(this);
  }
  update_space();
  QWidget::showEvent(event);
}

void TagComboBox::moveEvent(QMoveEvent* event) {
  if(!m_is_internal_move) {
    m_pos = event->pos();
  }
  m_is_internal_move = false;
  QWidget::moveEvent(event);
}

void TagComboBox::resizeEvent(QResizeEvent* event) {
  if(m_focus_observer.get_state() == FocusObserver::State::NONE) {
    if(m_overflow == TagBoxOverflow::ELIDE) {
      m_min_height = event->size().height();
    }
  } else {
    align();
  }
  QWidget::resizeEvent(event);
}

void TagComboBox::on_combo_box_submit(const std::any& submission) {
  m_tag_box->get_current()->set("");
  m_tag_box->get_list()->push(submission);
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
    m_alignment = Alignment::NONE;
    m_tag_box->get_current()->set("");
    update_space();
    move(m_pos);
    if(m_is_modified) {
      submit();
    }
  } else {
    if(m_overflow == TagBoxOverflow::WRAP) {
      align();
    }
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
    auto tag_box = TagBox(std::make_shared<ArrayListModel<std::any>>(),
      std::make_shared<LocalTextModel>());
    m_min_height = tag_box.sizeHint().height();
  }
}

void TagComboBox::align() {
  if(m_alignment == Alignment::ABOVE) {
    m_is_internal_move = true;
    move(x(), m_pos.y() + m_min_height - height());
    return;
  }
  if(sizeHint().height() > m_below_space && m_above_space > m_below_space) {
    m_alignment = Alignment::ABOVE;
    m_is_internal_move = true;
    move(x(), m_pos.y() + m_min_height - height());
    if(m_overflow == TagBoxOverflow::ELIDE) {
      setMaximumHeight(m_above_space);
    }
  } else {
    m_alignment = Alignment::BELOW;
    m_is_internal_move = true;
    move(m_pos);
    if(m_overflow == TagBoxOverflow::ELIDE) {
      setMaximumHeight(m_below_space);
    }
  }
  adjustSize();
}

void TagComboBox::submit() {
  copy_list_model(m_tag_box->get_list(), m_submission);
  m_is_modified = false;
  m_submit_signal(m_submission);
}

void TagComboBox::update_space() {
  auto y = parentWidget()->mapToGlobal(m_pos).y();
  auto window_y = m_window->geometry().y();
  m_above_space = y + m_min_height - window_y;
  m_below_space = window_y + m_window->height() - y;
}
