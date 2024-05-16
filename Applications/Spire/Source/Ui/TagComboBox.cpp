#include "Spire/Ui/TagComboBox.hpp"
#include <QKeyEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TagBox.hpp"

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
      auto find = [&] (int index) {
        for(auto j = index + 1; j != to->get_size(); ++j) {
          if(is_equal(from->get(index), to->get(j))) {
            return j;
          }
        }
        return -1;
      };
      for(auto i = 0; i != from->get_size(); ++i) {
        if(i < to->get_size()) {
          if(!is_equal(from->get(i), to->get(i))) {
            auto j = find(i);
            if(j >= 0) {
              to->move(j, i);
            } else {
              to->insert(from->get(i), i);
            }
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
      m_exclusion_set.insert(to_text(m_exclusions->get(i)));
    }
  }

  std::any parse(const QString& query) override {
    auto value = m_source->parse(query);
    if(!value.has_value()) {
      return value;
    }
    if(m_exclusion_set.contains(to_text(value))) {
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
        return m_exclusion_set.contains(to_text(value));
      });
      return matches;
    });
  }

  void on_operation(const AnyListModel::Operation& operation) {
    visit(operation,
      [&] (const AnyListModel::AddOperation& operation) {
        m_exclusion_set.insert(to_text(m_exclusions->get(operation.m_index)));
      },
      [&] (const AnyListModel::RemoveOperation& operation) {
        m_exclusion_set.erase(to_text(m_exclusions->get(operation.m_index)));
      });
  }
};

TagComboBox::TagComboBox(std::shared_ptr<ComboBox::QueryModel> query_model,
  QWidget* parent)
  : TagComboBox(std::move(query_model), &ListView::default_view_builder,
      parent) {}

TagComboBox::TagComboBox(std::shared_ptr<ComboBox::QueryModel> query_model,
  ListViewBuilder<> view_builder, QWidget* parent)
  : TagComboBox(std::move(query_model),
      std::make_shared<ArrayListModel<std::any>>(),
      std::move(view_builder), parent) {}

TagComboBox::TagComboBox(std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<AnyListModel> current, ListViewBuilder<> view_builder,
    QWidget* parent)
    : QWidget(parent),
      m_submission(std::make_shared<ArrayListModel<std::any>>()),
      m_focus_observer(*this),
      m_input_box(nullptr),
      m_drop_down_window(nullptr),
      m_is_modified(false) {
  copy_list_model(current, m_submission);
  m_tag_box =
    new TagBox(std::move(current), std::make_shared<LocalTextModel>());
  m_list_connection = m_tag_box->get_tags()->connect_operation_signal(
    std::bind_front(&TagComboBox::on_operation, this));
  m_any_input_box = new AnyInputBox(*m_tag_box);
  m_combo_box = new ComboBox(std::make_shared<TagComboBoxQueryModel>(
    std::move(query_model), m_tag_box->get_tags()),
    std::make_shared<LocalValueModel<std::any>>(), m_any_input_box,
    std::move(view_builder));
  m_combo_box->connect_submit_signal(
    std::bind_front(&TagComboBox::on_combo_box_submit, this));
  enclose(*this, *m_combo_box);
  proxy_style(*this, *m_combo_box);
  setFocusProxy(m_combo_box);
  m_focus_observer.connect_state_signal(
    std::bind_front(&TagComboBox::on_focus, this));
  m_any_input_box->installEventFilter(this);
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
  if(!is_read_only) {
    install_text_proxy_event_filter();
  }
}

connection TagComboBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool TagComboBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_input_box && event->type() == QEvent::KeyPress) {
    if(is_read_only()) {
      return QWidget::eventFilter(watched, event);
    }
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(key_event.key() == Qt::Key_Enter || key_event.key() == Qt::Key_Return) {
      if(key_event.text() != "\r") {
        return QWidget::eventFilter(watched, event);
      }
      push_combo_box();
      return true;
    } else if(key_event.key() == Qt::Key_Space) {
      if(m_combo_box->get_query_model()->parse(
          m_tag_box->get_current()->get()).has_value()) {
        push_combo_box();
        return true;
      }
    } else if(key_event.key() == Qt::Key_Escape) {
      event->ignore();
      return true;
    } else if(key_event.key() == Qt::Key_Down ||
        key_event.key() == Qt::Key_Up ||
        key_event.key() == Qt::Key_PageDown ||
        key_event.key() == Qt::Key_PageUp) {
      auto drop_down_window = find_drop_down_window();
      if(drop_down_window && drop_down_window->isVisible()) {
        QCoreApplication::sendEvent(m_combo_box, event);
        event->accept();
        return true;
      }
    }
  } else if(watched == m_input_box && event->type() == QEvent::FocusOut) {
    auto drop_down_window = find_drop_down_window();
    if(drop_down_window && find_focus_state(*drop_down_window) !=
        FocusObserver::State::NONE) {
      return true;
    }
  } else if(watched == m_any_input_box && event->type() == QEvent::KeyPress) {
    if(static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape) {
      event->ignore();
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TagComboBox::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    event->ignore();
    if(!m_tag_box->get_current()->get().isEmpty()) {
      m_tag_box->get_current()->set("");
      event->accept();
    }
    if(m_is_modified) {
      copy_list_model(m_submission, get_current());
      m_is_modified = false;
      event->accept();
    }
    return;
  }
  QWidget::keyPressEvent(event);
}

void TagComboBox::showEvent(QShowEvent* event) {
  install_text_proxy_event_filter();
  QWidget::showEvent(event);
}

void TagComboBox::install_text_proxy_event_filter() {
  if(auto input = find_focus_proxy(*m_tag_box); input != m_input_box) {
    m_input_box = input;
    m_input_box->installEventFilter(this);
  }
}

void TagComboBox::push_combo_box() {
  if(m_tag_box->get_current()->get().isEmpty()) {
    return;
  }
  auto value =
    m_combo_box->get_query_model()->parse(m_tag_box->get_current()->get());
  if(value.has_value()) {
    if(!is_equal(m_combo_box->get_current()->get(), value)) {
      m_combo_box->get_current()->set(value);
    }
    get_current()->push(value);
    m_tag_box->get_current()->set("");
    submit();
  } else {
    m_tag_box->get_current()->set("");
  }
}

void TagComboBox::submit() {
  copy_list_model(get_current(), m_submission);
  m_is_modified = false;
  m_submit_signal(m_submission);
}

QWidget* TagComboBox::find_drop_down_window() {
  if(m_drop_down_window) {
    return m_drop_down_window;
  }
  m_drop_down_window = find_pop_up_window(*m_combo_box);
  return m_drop_down_window;
}

void TagComboBox::on_combo_box_submit(const std::any& submission) {
  m_tag_box->get_current()->set("");
  get_current()->push(submission);
}

void TagComboBox::on_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::NONE) {
    if(m_tag_box->get_current()->get().isEmpty()) {
      if(m_is_modified && get_current()->get_size() > 0) {
        submit();
      }
    } else {
      push_combo_box();
    }
  }
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
