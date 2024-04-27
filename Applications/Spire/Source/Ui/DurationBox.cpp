#include "Spire/Ui/DurationBox.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  template<typename Model, typename M1, typename M2>
  QValidator::State test(Model& model, const typename Model::Type& value,
      std::weak_ptr<M1> m1, std::weak_ptr<M2> m2,
        const optional<time_duration>& update) {
    auto current = [&] () -> optional<time_duration> {
      if(!value) {
        auto s1 = m1.lock();
        auto s2 = m2.lock();
        if((!s1 || !s1->get()) && (!s2 || !s2->get())) {
          return none;
        }
      }
      return update;
    }();
    return model.m_source->test(current);
  }

  template<typename Model, typename M1, typename M2>
  QValidator::State set(Model& model, const typename Model::Type& value,
      std::weak_ptr<M1> m1, std::weak_ptr<M2> m2,
        const optional<time_duration>& update) {
    auto current = [&] () -> optional<time_duration> {
      if(!value) {
        auto s1 = m1.lock();
        auto s2 = m2.lock();
        if((!s1 || !s1->get()) && (!s2 || !s2->get())) {
          return none;
        }
      }
      return update;
    }();
    auto blocker = shared_connection_block(model.m_source_connection);
    auto blocker1 = [&] {
      if(auto s1 = m1.lock()) {
        return shared_connection_block(s1->m_source_connection);
      }
      return shared_connection_block();
    }();
    auto blocker2 = [&] {
      if(auto s2 = m2.lock()) {
        return shared_connection_block(s2->m_source_connection);
      }
      return shared_connection_block();
    }();
    if(model.m_source->set(current) != QValidator::State::Invalid) {
      auto state = QValidator::State::Acceptable;
      model.m_state = state;
      model.m_current = value;
      model.m_update_signal(model.m_current);
      return state;
    }
    return QValidator::State::Invalid;
  }

  template<typename Model, typename F>
  void on_current(Model& model, const optional<time_duration>& current, F&& f) {
    if(current) {
      model.m_current = std::forward<F>(f)(*current);
    } else {
      model.m_current = none;
    }
    model.m_update_signal(model.m_current);
  }

  struct HourModel;
  struct MinuteModel;
  struct SecondModel;

  template<typename T, typename M>
  auto to_shared_model(std::weak_ptr<M> model) {
    if(auto s = model.lock()) {
      return std::static_pointer_cast<T>(s);
    }
    return std::shared_ptr<T>();
  }

  struct HourModel : ScalarValueModel<optional<int>> {
    mutable UpdateSignal m_update_signal;
    std::shared_ptr<OptionalDurationModel> m_source;
    std::weak_ptr<OptionalIntegerModel> m_minutes;
    std::weak_ptr<ScalarValueModel<optional<Decimal>>> m_seconds;
    optional<int> m_current;
    QValidator::State m_state;
    scoped_connection m_source_connection;

    HourModel(std::shared_ptr<OptionalDurationModel> source)
        : m_source(std::move(source)),
          m_state(m_source->get_state()),
          m_source_connection(m_source->connect_update_signal(
            [=] (const auto& current) { on_current(current); })) {
      on_current(m_source->get());
    }

    optional<int> get_minimum() const {
      return 0;
    }

    optional<int> get_maximum() const {
      if(auto maximum = m_source->get_maximum()) {
        return static_cast<int>(maximum->hours());
      }
      return none;
    }

    QValidator::State get_state() const {
      return m_state;
    }

    const optional<int>& get() const {
      return m_current;
    }

    QValidator::State test(const Type& value) const {
      return ::test(*this, value, m_minutes, m_seconds,
        m_source->get().get_value_or(hours(0)) +
          hours(value.get_value_or(0)) - hours(m_current.get_value_or(0)));
    }

    QValidator::State set(const Type& value) {
      return ::set<HourModel, MinuteModel, SecondModel>(*this, value,
        to_shared_model<MinuteModel>(m_minutes),
        to_shared_model<SecondModel>(m_seconds),
        m_source->get().get_value_or(hours(0)) +
          hours(value.get_value_or(0)) - hours(m_current.get_value_or(0)));
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const {
      return m_update_signal.connect(slot);
    }

    void on_current(const optional<time_duration>& current) {
      ::on_current(*this, current,
        [] (auto current) { return static_cast<int>(current.hours()); });
    }
  };

  struct MinuteModel : ScalarValueModel<optional<int>> {
    mutable UpdateSignal m_update_signal;
    std::shared_ptr<OptionalDurationModel> m_source;
    std::weak_ptr<OptionalIntegerModel> m_hours;
    std::weak_ptr<ScalarValueModel<optional<Decimal>>> m_seconds;
    optional<int> m_current;
    QValidator::State m_state;
    scoped_connection m_source_connection;

    MinuteModel(std::shared_ptr<OptionalDurationModel> source)
        : m_source(std::move(source)),
          m_state(m_source->get_state()),
          m_source_connection(m_source->connect_update_signal(
            [=] (const auto& current) { on_current(current); })) {
      on_current(m_source->get());
    }

    optional<int> get_minimum() const {
      return 0;
    }

    optional<int> get_maximum() const {
      return 59;
    }

    QValidator::State get_state() const {
      return m_state;
    }

    const optional<int>& get() const {
      return m_current;
    }

    QValidator::State test(const Type& value) const {
      return ::test(*this, value, m_hours, m_seconds,
        m_source->get().get_value_or(minutes(0)) +
          minutes(value.get_value_or(0)) - minutes(m_current.get_value_or(0)));
    }

    QValidator::State set(const Type& value) {
      return ::set<MinuteModel, HourModel, SecondModel>(*this, value,
        to_shared_model<HourModel>(m_hours),
        to_shared_model<SecondModel>(m_seconds),
        m_source->get().get_value_or(minutes(0)) +
          minutes(value.get_value_or(0)) - minutes(m_current.get_value_or(0)));
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const {
      return m_update_signal.connect(slot);
    }

    void on_current(const optional<time_duration>& current) {
      ::on_current(*this, current,
        [] (auto current) { return static_cast<int>(current.minutes()); });
    }
  };

  struct SecondModel : ScalarValueModel<optional<Decimal>> {
    mutable UpdateSignal m_update_signal;
    std::shared_ptr<OptionalDurationModel> m_source;
    std::weak_ptr<OptionalIntegerModel> m_hours;
    std::weak_ptr<OptionalIntegerModel> m_minutes;
    optional<Decimal> m_current;
    QValidator::State m_state;
    scoped_connection m_source_connection;

    static auto to_seconds(const Decimal& decimal) {
      return milliseconds(static_cast<time_duration::sec_type>(
        (1000 * decimal).convert_to<double>()));
    }

    SecondModel(std::shared_ptr<OptionalDurationModel> source)
        : m_source(std::move(source)),
          m_state(m_source->get_state()),
          m_source_connection(m_source->connect_update_signal(
            [=] (const auto& current) { on_current(current); })) {
      on_current(m_source->get());
    }

    optional<Decimal> get_minimum() const {
      return Decimal(0);
    }

    optional<Decimal> get_maximum() const {
      return Decimal("59.999");
    }

    optional<Decimal> get_increment() const {
      return Decimal("0.001");
    }

    QValidator::State get_state() const {
      return m_state;
    }

    const optional<Decimal>& get() const {
      return m_current;
    }

    QValidator::State test(const Type& value) const {
      return ::test(*this, value, m_hours, m_minutes,
        m_source->get().get_value_or(seconds(0)) +
          to_seconds(value.get_value_or(0)) -
            to_seconds(m_current.get_value_or(0)));
    }

    QValidator::State set(const Type& value) {
      return ::set<SecondModel, HourModel, MinuteModel>(*this, value,
        to_shared_model<HourModel>(m_hours),
        to_shared_model<MinuteModel>(m_minutes),
        m_source->get().get_value_or(seconds(0)) +
          to_seconds(value.get_value_or(0)) -
            to_seconds(m_current.get_value_or(0)));
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const {
      return m_update_signal.connect(slot);
    }

    void on_current(const optional<time_duration>& current) {
      ::on_current(*this, current, [] (auto current) -> Decimal {
        return Decimal((current - hours(current.hours()) -
          minutes(current.minutes())).total_milliseconds()) / 1000;
      });
    }
  };

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(BodyAlign(Qt::AlignCenter)).
      set(border(scale_width(1), QColor(0xC8C8C8))).
      set(horizontal_padding(scale_width(4)));
    style.get(Hover() || FocusIn()).
      set(border_color(QColor(0x4B23A0)));
    style.get(ReadOnly()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_color(QColor(Qt::transparent)));
    style.get(Disabled()).
      set(BackgroundColor(QColor(0xF5F5F5))).
      set(border_color(QColor(0xC8C8C8)));
    style.get(ReadOnly() && Disabled()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_color(QColor(Qt::transparent)));
    style.get(Rejected()).
      set(BackgroundColor(chain(timeout(QColor(0xFFF1F1), milliseconds(250)),
        linear(QColor(0xFFF1F1), revert, milliseconds(300))))).
      set(border_color(
        chain(timeout(QColor(0xB71C1C), milliseconds(550)), revert)));
    style.get(Any() > Colon()).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    style.get(Disabled() > Colon()).
      set(TextColor(QColor(0xC8C8C8)));
    return style;
  }

  auto HOUR_FIELD_STYLE(StyleSheet style) {
    style.get(Any()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_size(0)).
      set(horizontal_padding(scale_width(0))).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    style.get(Any() > is_a<Button>()).set(Visibility::NONE);
    return style;
  }

  auto MINUTE_FIELD_STYLE(StyleSheet style) {
    style.get(Any()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_size(0)).
      set(horizontal_padding(scale_width(0))).
      set(LeadingZeros(2)).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    style.get(Any() > is_a<Button>()).set(Visibility::NONE);
    return style;
  }

  auto SECOND_FIELD_STYLE(StyleSheet style) {
    style.get(Any()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_size(0)).
      set(horizontal_padding(scale_width(0))).
      set(LeadingZeros(2)).set(TrailingZeros(3)).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    style.get(Any() > is_a<Button>()).set(Visibility::NONE);
    return style;
  }

  auto make_hour_field(std::shared_ptr<OptionalIntegerModel> current,
      QWidget& event_filter) {
    auto field = new IntegerBox(std::move(current));
    field->setMinimumWidth(scale_width(24));
    field->set_placeholder("hh");
    update_style(*field, [&] (auto& style) {
      style = HOUR_FIELD_STYLE(style);
    });
    find_focus_proxy(*field)->installEventFilter(&event_filter);
    return field;
  }

  auto make_minute_field(std::shared_ptr<OptionalIntegerModel> current,
      QWidget& event_filter) {
    auto field = new IntegerBox(std::move(current));
    field->setMinimumWidth(scale_width(28));
    field->set_placeholder("mm");
    update_style(*field, [&] (auto& style) {
      style = MINUTE_FIELD_STYLE(style);
    });
    find_focus_proxy(*field)->installEventFilter(&event_filter);
    return field;
  }

  auto make_second_field(
      std::shared_ptr<ScalarValueModel<optional<Decimal>>> current,
        QWidget& event_filter) {
    auto field = new DecimalBox(std::move(current));
    field->setMinimumWidth(scale_width(44));
    field->set_placeholder("ss.sss");
    update_style(*field, [&] (auto& style) {
      style = SECOND_FIELD_STYLE(style);
    });
    find_focus_proxy(*field)->installEventFilter(&event_filter);
    return field;
  }

  auto make_colon() {
    auto colon = make_label(":");
    colon->setFixedWidth(scale_width(10));
    match(*colon, Colon());
    return colon;
  }
}

DurationBox::DurationBox(QWidget* parent)
  : DurationBox(std::make_shared<LocalOptionalDurationModel>(), parent) {}

DurationBox::DurationBox(std::shared_ptr<OptionalDurationModel> current,
    QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_submission(m_current->get()),
      m_is_read_only(false),
      m_is_rejected(false),
      m_has_update(false) {
  auto container = new QWidget(this);
  container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto hour_model = std::make_shared<HourModel>(m_current);
  auto minute_model = std::make_shared<MinuteModel>(m_current);
  auto second_model = std::make_shared<SecondModel>(m_current);
  hour_model->m_minutes = minute_model;
  hour_model->m_seconds = second_model;
  minute_model->m_hours = hour_model;
  minute_model->m_seconds = second_model;
  second_model->m_hours = hour_model;
  second_model->m_minutes = minute_model;
  m_hour_field = make_hour_field(std::move(hour_model), *this);
  m_minute_field = make_minute_field(std::move(minute_model), *this);
  m_second_field = make_second_field(std::move(second_model), *this);
  auto hour_minute_colon = make_colon();
  auto minute_second_colon = make_colon();
  auto container_layout = make_hbox_layout(container);
  container_layout->addWidget(m_hour_field, 6);
  container_layout->addWidget(hour_minute_colon);
  container_layout->addWidget(m_minute_field, 7);
  container_layout->addWidget(minute_second_colon);
  container_layout->addWidget(m_second_field, 11);
  auto box = new Box(container);
  enclose(*this, *box);
  setFocusPolicy(Qt::StrongFocus);
  setFocusProxy(box);
  proxy_style(*this, *box);
  set_style(*this, DEFAULT_STYLE());
  m_hour_field->connect_submit_signal([=] (const auto& submission) {
    if(m_hour_field->hasFocus()) {
      on_submit();
    }
  });
  m_minute_field->connect_submit_signal([=] (const auto& submission) {
    if(m_minute_field->hasFocus()) {
      on_submit();
    }
  });
  m_second_field->connect_submit_signal([=] (const auto& submission) {
    if(m_second_field->hasFocus()) {
      on_submit();
    }
  });
  m_hour_field->connect_reject_signal([=] (const auto& value) { on_reject(); });
  m_minute_field->connect_reject_signal(
    [=] (const auto& value) { on_reject(); });
  m_second_field->connect_reject_signal(
    [=] (const auto& value) { on_reject(); });
  m_current->connect_update_signal(
    [=] (const auto& value) { on_current(value); });
}

const std::shared_ptr<OptionalDurationModel>& DurationBox::get_current() const {
  return m_current;
}

bool DurationBox::is_read_only() const {
  return m_is_read_only;
}

void DurationBox::set_read_only(bool is_read_only) {
  m_is_read_only = is_read_only;
  m_hour_field->set_read_only(m_is_read_only);
  m_minute_field->set_read_only(m_is_read_only);
  m_second_field->set_read_only(m_is_read_only);
  if(m_is_read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
}

connection DurationBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}

connection DurationBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool DurationBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::FocusOut) {
    if(!m_is_read_only && !m_hour_field->hasFocus() &&
        !m_minute_field->hasFocus() && !m_second_field->hasFocus()) {
      if(m_has_update) {
        on_submit();
      } else {
        update_empty_fields();
      }
    }
  } else if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    auto [field, is_field_empty] = [&] () -> std::pair<QWidget*, bool> {
      if(m_minute_field->hasFocus()) {
        return {m_minute_field, !m_minute_field->get_current()->get()};
      } else if(m_second_field->hasFocus()) {
        return {m_second_field, !m_second_field->get_current()->get()};
      } else if(m_hour_field->hasFocus()) {
        return {m_hour_field, !m_hour_field->get_current()->get()};
      }
      return {nullptr, true};
    }();
    if(key_event.key() == Qt::Key_Enter || key_event.key() == Qt::Key_Return) {
      if(is_field_empty && (m_hour_field->get_current()->get() ||
          m_minute_field->get_current()->get() ||
          m_second_field->get_current()->get())) {
        on_submit();
        return true;
      }
    } else if(key_event.key() == Qt::Key_Left &&
        (field == m_minute_field || field == m_second_field)) {
      if(auto editor = field->findChild<QLineEdit*>()) {
        if(editor->cursorPosition() == 0) {
          if(field == m_minute_field) {
            m_hour_field->setFocus();
          } else {
            m_minute_field->setFocus();
          }
        }
      }
    } else if(key_event.key() == Qt::Key_Right &&
        (field == m_hour_field || field == m_minute_field)) {
      if(auto editor = field->findChild<QLineEdit*>()) {
        if(editor->cursorPosition() == editor->text().size()) {
          if(field == m_hour_field) {
            m_minute_field->setFocus();
          } else {
            m_second_field->setFocus();
          }
        }
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DurationBox::on_current(const optional<time_duration>& current) {
  m_has_update = current != m_submission;
  if(m_is_rejected) {
    m_is_rejected = false;
    unmatch(*this, Rejected());
  }
}

void DurationBox::on_submit() {
  if(m_current->get_state() != QValidator::State::Acceptable) {
    on_reject();
  } else {
    m_submission = m_current->get();
    update_empty_fields();
    m_has_update = false;
    auto submission = m_submission;
    m_submit_signal(submission);
  }
}

void DurationBox::on_reject() {
  auto current = m_current->get();
  auto submission = m_submission;
  m_reject_signal(current);
  m_current->set(submission);
  if(!m_is_rejected) {
    m_is_rejected = true;
    match(*this, Rejected());
  }
}

void DurationBox::update_empty_fields() {
  if(m_submission) {
    if(!m_hour_field->get_current()->get()) {
      m_hour_field->get_current()->set(0);
    }
    if(!m_minute_field->get_current()->get()) {
      m_minute_field->get_current()->set(0);
    }
    if(!m_second_field->get_current()->get()) {
      m_second_field->get_current()->set(Decimal(0));
    }
  }
}

DurationBox* Spire::make_time_box(const optional<time_duration>& time,
    QWidget* parent) {
  return new DurationBox(make_time_of_day_model(time), parent);
}

DurationBox* Spire::make_time_box(QWidget* parent) {
  return make_time_box(none, parent);
}

std::shared_ptr<OptionalDurationModel> Spire::make_time_of_day_model() {
  return make_time_of_day_model(none);
}

std::shared_ptr<OptionalDurationModel> Spire::make_time_of_day_model(
    const optional<time_duration>& time) {
  auto model = std::make_shared<LocalOptionalDurationModel>(time);
  model->set_maximum(hours(23) + minutes(59) + seconds(59) + millisec(999));
  return model;
}
