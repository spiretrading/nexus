#include "Spire/Ui/ComboBox.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/EmptyState.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto trim_leading_whitespaces(const QString& text) {
    auto trimmed = text;
    return trimmed.remove(QRegularExpression("^\\s+"));
  }

  struct TextWrapperModel : TextModel {
    std::shared_ptr<AnyValueModel> m_current;
    LocalTextModel m_value;
    scoped_connection m_connection;

    TextWrapperModel(std::shared_ptr<AnyValueModel> current)
        : m_current(std::move(current)),
          m_value(to_text(m_current->get())) {
      m_connection = m_current->connect_update_signal(
        std::bind_front(&TextWrapperModel::on_update, this));
    }

    const QString& get() const {
      return m_value.get();
    }

    QValidator::State test(const QString& value) const {
      return m_value.test(value);
    }

    QValidator::State set(const QString& value) {
      return m_value.set(value);
    }

    connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const {
      return m_value.connect_update_signal(slot);
    }

    void on_update(const AnyRef& current) {
      auto text = to_text(current);
      if(m_value.get() != text) {
        m_value.set(text);
      }
    }
  };
}

AnyComboBox::DeferredData::DeferredData(AnyComboBox& box)
  : m_submission(box.m_current->get()),
    m_submission_text(to_text(m_submission)),
    m_input_focus_proxy(nullptr),
    m_focus_observer(box),
    m_key_observer(*box.m_input_box),
    m_matches(box.m_matches_builder()),
    m_drop_down_list(nullptr),
    m_empty_state(nullptr),
    m_completion_tag(0),
    m_has_autocomplete_selection(false),
    m_is_querying(false),
    m_current_connection(box.m_current->connect_update_signal(
      std::bind_front(&AnyComboBox::on_current, &box))) {}

AnyComboBox::AnyComboBox(std::shared_ptr<AnyQueryModel> query_model,
    std::shared_ptr<AnyValueModel> current, AnyInputBox* input_box,
    ListViewItemBuilder<> item_builder,
    std::function<std::shared_ptr<AnyListModel> ()> matches_builder,
    QWidget* parent)
    : QWidget(parent),
      m_query_model(std::move(query_model)),
      m_current(std::move(current)),
      m_input_box(input_box),
      m_item_builder(std::move(item_builder)),
      m_matches_builder(std::move(matches_builder)) {
  setFocusProxy(m_input_box);
  proxy_style(*this, *m_input_box);
  enclose(*this, *m_input_box);
  update_style(*this, [] (auto& style) {
    style.get(PopUp()).set(border_color(QColor(0x4B23A0)));
  });
}

const std::shared_ptr<AnyQueryModel>& AnyComboBox::get_query_model() const {
  return m_query_model;
}

const std::shared_ptr<AnyValueModel>& AnyComboBox::get_current() const {
  return m_current;
}

const std::any& AnyComboBox::get_submission() const {
  initialize_deferred_data();
  return m_data->m_submission;
}

void AnyComboBox::set_placeholder(const QString& placeholder) {
  m_input_box->set_placeholder(placeholder);
}

bool AnyComboBox::is_read_only() const {
  return m_input_box->is_read_only();
}

void AnyComboBox::set_read_only(bool is_read_only) {
  if(!is_read_only) {
    initialize_deferred_data();
  }
  m_input_box->set_read_only(is_read_only);
}

connection AnyComboBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  initialize_deferred_data();
  return m_data->m_submit_signal.connect(slot);
}

bool AnyComboBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_data->m_drop_down_list || watched == m_data->m_empty_state) {
    if(event->type() == QEvent::Show) {
      match(*this, PopUp());
    } else if(event->type() == QEvent::Hide) {
      unmatch(*this, PopUp());
    }
  } else if(watched == m_input_box) {
    if(event->type() == QEvent::Show) {
      update_focus_proxy();
    }
  } else if(watched == m_data->m_input_focus_proxy) {
    if(event->type() == QEvent::FocusOut &&
        (m_data->m_drop_down_list->isVisible() ||
          m_data->m_empty_state->isVisible())) {
      return true;
    }
  } else if(watched == &m_data->m_drop_down_list->get_list_view()) {
    if(event->type() == QEvent::KeyPress) {
      auto key = static_cast<QKeyEvent*>(event)->key();
      if((key >= Qt::Key_0 && key <= Qt::Key_9) ||
          (key >= Qt::Key_A && key <= Qt::Key_Z) || key == Qt::Key_Underscore) {
        return QCoreApplication::sendEvent(m_data->m_input_focus_proxy, event);
      }
    }
  } else if(watched == m_data->m_drop_down_list->window() ||
      watched == m_data->m_empty_state->window()) {
    if(event->type() == QEvent::KeyPress) {
      auto& key_event = *static_cast<QKeyEvent*>(event);
      if(key_event.key() == Qt::Key_Escape &&
          m_data->m_empty_state->isVisible()) {
        m_data->m_empty_state->hide();
      } else if(!(key_event.key() == Qt::Key_Space &&
          m_data->m_drop_down_list->isVisible() &&
          m_data->m_drop_down_list->get_list_view().get_current()->get())) {
        return QCoreApplication::sendEvent(m_data->m_input_focus_proxy, event);
      }
    } else if(event->type() == QEvent::ContextMenu) {
      auto& menu_event = *static_cast<QContextMenuEvent*>(event);
      if(rect().contains(mapFromGlobal(menu_event.globalPos()))) {
        auto new_event = QContextMenuEvent(menu_event.reason(),
          m_data->m_input_focus_proxy->mapFromGlobal(menu_event.globalPos()),
          menu_event.globalPos(), menu_event.modifiers());
        return QCoreApplication::sendEvent(
          m_data->m_input_focus_proxy, &new_event);
      }
    } else if(event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::MouseMove) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(rect().contains(mapFromGlobal(mouse_event.globalPos()))) {
        auto new_event = QMouseEvent(mouse_event.type(),
          m_data->m_input_focus_proxy->mapFromGlobal(mouse_event.globalPos()),
          mouse_event.windowPos(), mouse_event.screenPos(),
          mouse_event.button(), mouse_event.buttons(), mouse_event.modifiers(),
          mouse_event.source());
        return QCoreApplication::sendEvent(
          m_data->m_input_focus_proxy, &new_event);
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void AnyComboBox::keyPressEvent(QKeyEvent* event) {
  if(is_read_only()) {
    return QWidget::keyPressEvent(event);
  }
  if(m_data->m_drop_down_list->isVisible() &&
      (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up ||
      event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown)) {
    auto is_top_current =
      m_data->m_drop_down_list->get_list_view().get_current()->get() == 0;
    QCoreApplication::sendEvent(
      &m_data->m_drop_down_list->get_list_view(), event);
    if(event->key() == Qt::Key_Up && is_top_current &&
        m_data->m_drop_down_list->get_list_view().get_current()->get() == 0) {
      revert_current();
    }
  }
  if(event->key() == Qt::Key_Down || event->key() == Qt::Key_Up) {
    m_data->m_drop_down_list->setVisible(
      m_data->m_drop_down_list->get_list_view().get_list()->get_size() != 0);
    event->accept();
    return;
  } else if(event->key() == Qt::Key_Escape) {
    if(m_data->m_drop_down_list->get_list_view().get_selection().get() == 0) {
      m_data->m_drop_down_list->hide();
      return;
    }
  }
  return QWidget::keyPressEvent(event);
}

void AnyComboBox::showEvent(QShowEvent* event) {
  if(!is_read_only()) {
    initialize_deferred_data();
  }
  QWidget::showEvent(event);
}

std::shared_ptr<TextModel> AnyComboBox::make_text_wrapper_model(
    std::shared_ptr<AnyValueModel> current) {
  return std::make_shared<TextWrapperModel>(std::move(current));
}

void AnyComboBox::update_focus_proxy() {
  auto proxy = find_focus_proxy(*m_input_box);
  if(proxy != m_data->m_input_focus_proxy) {
    if(m_data->m_input_focus_proxy) {
      m_data->m_input_focus_proxy->removeEventFilter(this);
    }
    m_data->m_input_focus_proxy = proxy;
    if(proxy) {
      proxy->installEventFilter(this);
    }
  }
}

void AnyComboBox::initialize_deferred_data() const {
  if(m_data) {
    return;
  }
  auto self = const_cast<AnyComboBox*>(this);
  self->m_data = std::make_unique<DeferredData>(*self);
  m_input_box->installEventFilter(self);
  m_input_box->connect_submit_signal(
    std::bind_front(&AnyComboBox::on_submit, self));
  m_data->m_input_connection =
    m_input_box->get_current()->connect_update_signal(
      std::bind_front(&AnyComboBox::on_input, self));
  m_data->m_highlight_connection =
    m_input_box->get_highlight()->connect_update_signal(
      std::bind_front(&AnyComboBox::on_highlight, self));
  auto list_view =
    new ListView(m_data->m_matches, std::move(m_item_builder));
  list_view->setFocusPolicy(Qt::NoFocus);
  m_data->m_drop_down_list = new DropDownList(*list_view, *self);
  m_data->m_drop_down_list->setFocusPolicy(Qt::NoFocus);
  auto panel = m_data->m_drop_down_list->window();
  panel->setFocusPolicy(Qt::NoFocus);
  panel->setWindowFlags(Qt::Popup | (panel->windowFlags() & ~Qt::Tool));
  panel->installEventFilter(self);
  list_view->installEventFilter(self);
  m_data->m_drop_down_list->installEventFilter(self);
  m_data->m_empty_state = new EmptyState(tr("No matches"), *self);
  m_data->m_empty_state->installEventFilter(self);
  m_data->m_empty_state->window()->installEventFilter(self);
  self->update_focus_proxy();
  m_data->m_drop_down_current_connection = m_data->m_drop_down_list->
    get_list_view().get_current()->connect_update_signal(
      std::bind_front(&AnyComboBox::on_drop_down_current, self));
  m_data->m_drop_down_list->get_list_view().connect_submit_signal(
    std::bind_front(&AnyComboBox::on_drop_down_submit, self));
  m_data->m_focus_observer.connect_state_signal(
    std::bind_front(&AnyComboBox::on_focus, self));
  m_data->m_key_observer.connect_filtered_key_press_signal(
    std::bind_front(&AnyComboBox::on_input_key_press, self));
}

void AnyComboBox::update_completion() {
  if(m_data->m_matches->get_size() != 0) {
    auto& highlight = *m_input_box->get_highlight();
    auto query = trim_leading_whitespaces(
      any_cast<QString>(m_input_box->get_current()->get()));
    auto top_match = to_text(m_data->m_matches->get(0));
    if(!top_match.toLower().startsWith(query.toLower())) {
      m_data->m_prefix = query;
      m_data->m_completion.clear();
      return;
    }
    auto prefix = top_match.mid(0, query.size()).toLower();
    auto completion = top_match.mid(query.size());
    if(m_data->m_last_completion.size() < query.size()) {
      m_data->m_last_completion = query;
      auto selection_start = query.size();
      {
        auto blocker = shared_connection_block(m_data->m_input_connection);
        m_input_box->get_current()->set(query + completion);
      }
      m_data->m_has_autocomplete_selection = false;
      highlight.set({selection_start + completion.size(), selection_start});
      m_data->m_has_autocomplete_selection = true;
    } else {
      m_data->m_last_completion = query;
    }
    m_data->m_prefix = std::move(prefix);
    m_data->m_completion = std::move(completion);
  } else {
    m_data->m_last_completion.clear();
    m_data->m_prefix = any_cast<QString>(m_input_box->get_current()->get());
    m_data->m_completion.clear();
  }
}

void AnyComboBox::revert_to(const QString& query, bool autocomplete) {
  auto blocker = shared_connection_block(m_data->m_input_connection);
  m_input_box->get_current()->set(query);
  m_data->m_has_autocomplete_selection = false;
  m_data->m_last_completion.clear();
  m_data->m_prefix = query;
  m_data->m_completion.clear();
  if(autocomplete) {
    update_completion();
  }
}

void AnyComboBox::revert_current() {
  auto& list_view = m_data->m_drop_down_list->get_list_view();
  list_view.get_current()->set(none);
  clear(*list_view.get_selection());
  if(m_data->m_user_query) {
    revert_to(*m_data->m_user_query, true);
  }
}

void AnyComboBox::submit(const QString& query, bool is_passive) {
  auto value = m_query_model->parse(query);
  if(!value.has_value() ||
      is_passive && to_text(value) == to_text(m_data->m_submission)) {
    return;
  }
  if(!m_data->m_completion.isEmpty()) {
    auto blocker = shared_connection_block(m_data->m_input_connection);
    m_input_box->get_current()->set(query);
    m_data->m_has_autocomplete_selection = false;
    auto current_blocker =
      shared_connection_block(m_data->m_current_connection);
    m_current->set(value);
  }
  m_data->m_last_completion = query;
  m_data->m_prefix = query;
  m_data->m_completion.clear();
  m_data->m_submission = value;
  m_data->m_submission_text = query;
  m_input_box->get_highlight()->set(Highlight(query.size()));
  m_data->m_drop_down_list->hide();
  m_data->m_query_result = m_query_model->submit(query).then(
    std::bind_front(
      &AnyComboBox::on_query, this, ++m_data->m_completion_tag, false));
  m_data->m_submit_signal(value);
}

void AnyComboBox::on_current(const std::any& current) {
  auto input = any_cast<QString>(m_input_box->get_current()->get());
  if(!is_equal(current, m_query_model->parse(input))) {
    auto text = to_text(current);
    if(input != text) {
      m_input_box->get_current()->set(text);
    }
  }
}

void AnyComboBox::on_input(const AnyRef& current) {
  if(is_read_only()) {
    return;
  }
  auto query = trim_leading_whitespaces(any_cast<QString>(current));
  if(!m_data->m_last_completion.toLower().startsWith(query.toLower())) {
    m_data->m_last_completion.clear();
  }
  m_data->m_user_query = query;
  m_data->m_has_autocomplete_selection = false;
  if(query.isEmpty()) {
    on_query(++m_data->m_completion_tag, false, std::vector<std::any>());
  } else {
    m_data->m_query_result = m_query_model->submit(query).then(std::bind_front(
      &AnyComboBox::on_query, this, ++m_data->m_completion_tag, true));
    auto value = m_query_model->parse(query);
    if(value.has_value()) {
      auto blocker = std::array{
        shared_connection_block(m_data->m_input_connection),
        shared_connection_block(m_data->m_current_connection)};
      m_current->set(value);
    }
  }
}

void AnyComboBox::on_highlight(const Highlight& highlight) {
  if(!m_data->m_has_autocomplete_selection) {
    return;
  }
  m_data->m_has_autocomplete_selection = false;
  auto query = trim_leading_whitespaces(
    any_cast<QString>(m_input_box->get_current()->get()));
  auto value = m_query_model->parse(query);
  if(value.has_value()) {
    m_data->m_prefix = query;
    m_data->m_completion.clear();
    auto current_blocker =
      shared_connection_block(m_data->m_current_connection);
    m_current->set(value);
  }
}

void AnyComboBox::on_submit(const AnyRef& query) {
  if(find_focus_state(*m_input_box) == FocusObserver::State::NONE) {
    return;
  }
  m_data->m_user_query = any_cast<QString>(query);
  submit(*m_data->m_user_query);
}

void AnyComboBox::on_query(
    std::uint32_t tag, bool show, Expect<std::vector<std::any>>&& result) {
  if(m_data->m_completion_tag != tag) {
    return;
  }
  m_data->m_is_querying = true;
  auto selection = [&] {
    try {
      return result.get();
    } catch(const std::exception&) {
      return std::vector<std::any>();
    }
  }();
  {
    if(m_data->m_matches->get_size() > 0) {
      m_data->m_drop_down_list->hide();
    }
    auto blocker =
      shared_connection_block(m_data->m_drop_down_current_connection);
    m_data->m_matches->transact([&] {
      while(m_data->m_matches->get_size() != 0) {
        m_data->m_matches->remove(m_data->m_matches->get_size() - 1);
      }
      for(auto& item : selection) {
        m_data->m_matches->push(item);
        auto& list_view = m_data->m_drop_down_list->get_list_view();
        auto list_item = list_view.get_list_item(
          list_view.get_list()->get_size() - 1);
        list_item->setFocusPolicy(Qt::NoFocus);
        if(list_item->is_mounted()) {
          list_item->layout()->itemAt(0)->widget()->setFocusPolicy(Qt::NoFocus);
        }
      }
    });
  }
  m_data->m_is_querying = false;
  update_completion();
  if(show) {
    if(selection.empty()) {
      m_data->m_drop_down_list->hide();
      m_data->m_empty_state->show();
    } else if(m_data->m_focus_observer.get_state() !=
        FocusObserver::State::NONE && !m_data->m_drop_down_list->isVisible()) {
      m_data->m_empty_state->hide();
      m_data->m_drop_down_list->show();
    }
  } else {
    m_data->m_drop_down_list->hide();
    m_data->m_empty_state->hide();
  }
}

void AnyComboBox::on_drop_down_current(optional<int> index) {
  if(index) {
    auto value =
      m_data->m_drop_down_list->get_list_view().get_list()->get(*index);
    auto text = to_text(value);
    {
      auto input_blocker = shared_connection_block(m_data->m_input_connection);
      auto highlight_blocker =
        shared_connection_block(m_data->m_highlight_connection);
      m_input_box->get_current()->set(text);
    }
    m_data->m_last_completion = text;
    m_data->m_completion.clear();
    m_data->m_prefix.clear();
    m_data->m_has_autocomplete_selection = false;
    auto current_blocker =
      shared_connection_block(m_data->m_current_connection);
    m_current->set(value);
  }
}

void AnyComboBox::on_drop_down_submit(const std::any& submission) {
  auto text = to_text(submission);
  {
    auto input_blocker = shared_connection_block(m_data->m_input_connection);
    auto highlight_blocker =
      shared_connection_block(m_data->m_highlight_connection);
    m_input_box->get_current()->set(text);
    m_data->m_has_autocomplete_selection = false;
    m_input_box->get_highlight()->set(Highlight(text.size()));
  }
  m_data->m_submission = submission;
  m_data->m_submission_text = text;
  m_data->m_drop_down_list->hide();
  auto input_blocker = shared_connection_block(m_data->m_input_connection);
  m_data->m_submit_signal(submission);
}

void AnyComboBox::on_focus(FocusObserver::State state) {
  if(state == FocusObserver::State::NONE && m_data && !m_data->m_is_querying) {
    m_data->m_drop_down_list->hide();
    m_data->m_empty_state->hide();
    if(m_data->m_input_focus_proxy) {
      auto focus_out_event = QFocusEvent(QEvent::FocusOut);
      QCoreApplication::sendEvent(
        m_data->m_input_focus_proxy, &focus_out_event);
    }
    submit(any_cast<QString>(m_input_box->get_current()->get()), true);
  } else if(state != FocusObserver::State::NONE && m_data) {
    m_data->m_submission = to_any(m_current->get());
    if(m_data->m_submission.has_value()) {
      m_data->m_submission_text = to_text(m_data->m_submission);
    }
  }
}

bool AnyComboBox::on_input_key_press(QWidget& target, QKeyEvent& event) {
  if(is_read_only()) {
    return false;
  } else if(event.key() == Qt::Key_Escape) {
    if(m_data->m_drop_down_list->isVisible()) {
      m_data->m_drop_down_list->hide();
      revert_current();
    } else if(any_cast<QString>(m_input_box->get_current()->get()) !=
        m_data->m_submission_text &&
        m_query_model->parse(m_data->m_submission_text).has_value()) {
      revert_to(m_data->m_submission_text, false);
    } else {
      event.ignore();
    }
    return true;
  }
  return false;
}
