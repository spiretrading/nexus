#include "Spire/Ui/DurationBox.hpp"
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <boost/signals2/shared_connection_block.hpp>
#include <QCoreApplication>
#include <QKeyEvent>
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using NullCurrent = StateSelector<void, struct NullCurrentTag>;

  const auto HOUR = 0;
  const auto MINUTE = 1;
  const auto SECOND = 2;
  const auto FRACTIONAL_SECOND = 3;
  const auto FIELD_COUNT = 4;
  const auto DEFAULT_FORMAT = "hh:mm:ss.fff";
  const auto MAX_FRACTIONAL_DIGITS = 9;

  struct FormatFields {
    bool m_has_hours;
    bool m_has_minutes;
    bool m_has_seconds;
    int m_fractional_digits;
  };

  optional<FormatFields> match_format(const QString& format) {
    static const auto HEADS =
      QStringList{"hh", "hh:mm", "hh:mm:ss", "mm", "mm:ss", "ss"};
    auto fields = FormatFields();
    auto head = format;
    while(head.endsWith('f')) {
      head.chop(1);
      ++fields.m_fractional_digits;
    }
    if(fields.m_fractional_digits != 0) {
      if(fields.m_fractional_digits > MAX_FRACTIONAL_DIGITS) {
        return none;
      }
      if(head.isEmpty()) {
        return fields;
      }
      if(!head.endsWith('.')) {
        return none;
      }
      head.chop(1);
      if(head.isEmpty()) {
        return none;
      }
    }
    if(!HEADS.contains(head)) {
      return none;
    }
    fields.m_has_hours = head.startsWith("hh");
    fields.m_has_minutes = head.contains("mm");
    fields.m_has_seconds = head.endsWith("ss");
    if(fields.m_fractional_digits != 0 && !fields.m_has_seconds) {
      return none;
    }
    return fields;
  }

  time_duration::tick_type get_units_per_second(int fractional_digits) {
    auto units = time_duration::tick_type(1);
    for(auto i = 0; i < fractional_digits; ++i) {
      units *= 10;
    }
    return units;
  }

  time_duration to_fractional_seconds(int units, int fractional_digits) {
    return time_duration(0, 0, 0,
      static_cast<time_duration::fractional_seconds_type>(
        units * time_duration::ticks_per_second() /
          get_units_per_second(fractional_digits)));
  }

  int to_fraction_units(const time_duration& duration, int fractional_digits) {
    return static_cast<int>(duration.fractional_seconds() *
      get_units_per_second(fractional_digits) /
        time_duration::ticks_per_second());
  }

  int get_text_width(const QWidget& editor, const QString& text) {
    return editor.fontMetrics().horizontalAdvance(text);
  }

  struct FieldModel : ScalarValueModel<optional<int>> {
    mutable UpdateSignal m_update_signal;
    std::shared_ptr<OptionalDurationModel> m_source;
    std::array<std::weak_ptr<FieldModel>, FIELD_COUNT - 1> m_siblings;
    optional<int> m_current;
    QValidator::State m_state;
    bool m_is_active;
    scoped_connection m_source_connection;

    explicit FieldModel(std::shared_ptr<OptionalDurationModel> source)
        : m_source(std::move(source)),
          m_state(m_source->get_state()),
          m_is_active(true) {
      m_source_connection = m_source->connect_update_signal(
        std::bind_front(&FieldModel::on_current, this));
    }

    virtual optional<time_duration> to_duration(
      const optional<int>& value) const = 0;

    virtual int to_units(const time_duration& duration) const = 0;

    optional<time_duration> compose(const optional<int>& value) const {
      if(!value) {
        auto is_empty = std::none_of(m_siblings.begin(), m_siblings.end(),
          [] (const auto& sibling) {
            auto model = sibling.lock();
            return model && model->m_is_active && model->get();
          });
        if(is_empty) {
          return none;
        }
      }
      return to_duration(value);
    }

    optional<int> get_minimum() const override {
      return 0;
    }

    QValidator::State get_state() const override {
      return m_state;
    }

    const optional<int>& get() const override {
      return m_current;
    }

    QValidator::State test(const Type& value) const override {
      return m_source->test(compose(value));
    }

    QValidator::State set(const Type& value) override {
      auto current = compose(value);
      auto blocker = shared_connection_block(m_source_connection);
      auto blockers = std::array<shared_connection_block, m_siblings.size()>();
      for(auto i = std::size_t(0); i < m_siblings.size(); ++i) {
        if(auto sibling = m_siblings[i].lock()) {
          blockers[i] = shared_connection_block(sibling->m_source_connection);
        }
      }
      if(m_source->set(current) == QValidator::State::Invalid) {
        return QValidator::State::Invalid;
      }
      m_state = QValidator::State::Acceptable;
      m_current = value;
      m_update_signal(m_current);
      return m_state;
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override {
      return m_update_signal.connect(slot);
    }

    void on_current(const optional<time_duration>& current) {
      if(current) {
        m_current = to_units(*current);
      } else {
        m_current = none;
      }
      m_update_signal(m_current);
    }
  };

  struct HourModel : FieldModel {
    explicit HourModel(std::shared_ptr<OptionalDurationModel> source)
        : FieldModel(std::move(source)) {
      on_current(m_source->get());
    }

    optional<int> get_maximum() const override {
      if(auto maximum = m_source->get_maximum()) {
        return static_cast<int>(maximum->hours());
      }
      return none;
    }

    optional<time_duration> to_duration(
        const optional<int>& value) const override {
      return m_source->get().get_value_or(hours(0)) +
        hours(value.get_value_or(0)) - hours(m_current.get_value_or(0));
    }

    int to_units(const time_duration& duration) const override {
      return static_cast<int>(duration.hours());
    }
  };

  struct MinuteModel : FieldModel {
    explicit MinuteModel(std::shared_ptr<OptionalDurationModel> source)
        : FieldModel(std::move(source)) {
      on_current(m_source->get());
    }

    optional<int> get_maximum() const override {
      return 59;
    }

    optional<time_duration> to_duration(
        const optional<int>& value) const override {
      return m_source->get().get_value_or(minutes(0)) +
        minutes(value.get_value_or(0)) - minutes(m_current.get_value_or(0));
    }

    int to_units(const time_duration& duration) const override {
      return static_cast<int>(duration.minutes());
    }
  };

  struct SecondModel : FieldModel {
    explicit SecondModel(std::shared_ptr<OptionalDurationModel> source)
        : FieldModel(std::move(source)) {
      on_current(m_source->get());
    }

    optional<int> get_maximum() const override {
      return 59;
    }

    optional<time_duration> to_duration(
        const optional<int>& value) const override {
      return m_source->get().get_value_or(seconds(0)) +
        seconds(value.get_value_or(0)) - seconds(m_current.get_value_or(0));
    }

    int to_units(const time_duration& duration) const override {
      return static_cast<int>(duration.seconds());
    }
  };

  struct FractionalSecondModel : FieldModel {
    int m_fractional_digits;

    FractionalSecondModel(std::shared_ptr<OptionalDurationModel> source,
        int fractional_digits)
        : FieldModel(std::move(source)),
          m_fractional_digits(fractional_digits) {
      on_current(m_source->get());
    }

    void set_fractional_digits(int fractional_digits) {
      if(fractional_digits == m_fractional_digits) {
        return;
      }
      m_fractional_digits = fractional_digits;
      on_current(m_source->get());
    }

    optional<int> get_maximum() const override {
      return static_cast<int>(get_units_per_second(m_fractional_digits) - 1);
    }

    optional<time_duration> to_duration(
        const optional<int>& value) const override {
      return m_source->get().get_value_or(seconds(0)) +
        to_fractional_seconds(value.get_value_or(0), m_fractional_digits) -
          to_fractional_seconds(
            m_current.get_value_or(0), m_fractional_digits);
    }

    int to_units(const time_duration& duration) const override {
      return to_fraction_units(duration, m_fractional_digits);
    }
  };

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(TextAlign(Qt::AlignLeft)).
      set(Format(QString(DEFAULT_FORMAT)));
    style.get(Any() > Separator()).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    style.get(NullCurrent() > Separator()).
      set(TextColor(QColor(0xA0A0A0)));
    style.get(Disabled() > Separator()).
      set(TextColor(QColor(0xC8C8C8)));
    return style;
  }

  auto make_field(std::shared_ptr<OptionalIntegerModel> current,
      QHash<Qt::KeyboardModifier, int> modifiers, const QString& placeholder,
        int leading_zeros, QWidget& event_filter) {
    auto field = new IntegerBox(std::move(current), std::move(modifiers));
    field->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    field->set_placeholder(placeholder);
    update_style(*field, [&] (auto& style) {
      style.get(Any()).
        set(BackgroundColor(QColor(Qt::transparent))).
        set(border_size(0)).
        set(padding(0)).
        set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
        set(LeadingZeros(leading_zeros));
      style.get(Any() > (DownButton() || UpButton())).set(Visibility::NONE);
    });
    find_focus_proxy(*field)->installEventFilter(&event_filter);
    return field;
  }

  auto make_two_digit_field(std::shared_ptr<OptionalIntegerModel> current,
      QHash<Qt::KeyboardModifier, int> modifiers, const QString& placeholder,
        QWidget& event_filter) {
    auto field = make_field(
      std::move(current), std::move(modifiers), placeholder, 2, event_filter);
    auto& editor = *find_focus_proxy(*field);
    field->setMinimumWidth(std::max(2 * get_character_width(editor.font()),
      get_text_width(editor, placeholder)));
    return field;
  }

  auto make_hour_field(std::shared_ptr<OptionalIntegerModel> current,
      QWidget& event_filter) {
    auto modifiers = QHash<Qt::KeyboardModifier, int>(
      {{Qt::NoModifier, 1}, {Qt::AltModifier, 5}, {Qt::ControlModifier, 10}});
    return make_two_digit_field(
      std::move(current), std::move(modifiers), "hh", event_filter);
  }

  auto make_minute_field(std::shared_ptr<OptionalIntegerModel> current,
      QWidget& event_filter) {
    auto modifiers = QHash<Qt::KeyboardModifier, int>({{Qt::NoModifier, 1},
      {Qt::AltModifier, 10}, {Qt::ControlModifier, 15},
      {Qt::ShiftModifier, 30}});
    return make_two_digit_field(
      std::move(current), std::move(modifiers), "mm", event_filter);
  }

  auto make_second_field(std::shared_ptr<OptionalIntegerModel> current,
      QWidget& event_filter) {
    auto modifiers = QHash<Qt::KeyboardModifier, int>({{Qt::NoModifier, 1},
      {Qt::AltModifier, 10}, {Qt::ControlModifier, 15},
      {Qt::ShiftModifier, 30}});
    return make_two_digit_field(
      std::move(current), std::move(modifiers), "ss", event_filter);
  }

  auto make_fractional_second_field(
      std::shared_ptr<OptionalIntegerModel> current, QWidget& event_filter) {
    auto modifiers = QHash<Qt::KeyboardModifier, int>({{Qt::NoModifier, 1},
      {Qt::AltModifier, 10}, {Qt::ControlModifier, 100},
      {Qt::ShiftModifier, 1000}});
    auto field = make_field(std::move(current), std::move(modifiers),
      QString(MAX_FRACTIONAL_DIGITS, 'f'), MAX_FRACTIONAL_DIGITS,
      event_filter);
    field->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    return field;
  }

  auto make_separator(const QString& text) {
    auto separator = make_label(text);
    separator->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    match(*separator, Separator());
    return separator;
  }

  class TimeBox : public DurationBox {
    public:
      using DurationBox::DurationBox;

    protected:
      bool test_format(const QString& format) const override {
        auto fields = match_format(format);
        return fields && fields->m_has_hours;
      }
  };
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
      m_is_null(false),
      m_has_update(false) {
  auto container = new QWidget(this);
  container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto hour_model = std::make_shared<HourModel>(m_current);
  auto minute_model = std::make_shared<MinuteModel>(m_current);
  auto second_model = std::make_shared<SecondModel>(m_current);
  auto fractional_second_model =
    std::make_shared<FractionalSecondModel>(m_current, 0);
  auto models = std::array<std::shared_ptr<FieldModel>, FIELD_COUNT>{
    hour_model, minute_model, second_model, fractional_second_model};
  for(auto i = std::size_t(0); i < models.size(); ++i) {
    auto sibling = std::size_t(0);
    for(auto j = std::size_t(0); j < models.size(); ++j) {
      if(i != j) {
        models[i]->m_siblings[sibling] = models[j];
        ++sibling;
      }
    }
  }
  m_fields = {make_hour_field(std::move(hour_model), *this),
    make_minute_field(std::move(minute_model), *this),
    make_second_field(std::move(second_model), *this),
    make_fractional_second_field(std::move(fractional_second_model), *this)};
  m_separators = {make_separator(":"), make_separator(":"), make_separator(".")};
  auto container_layout = make_hbox_layout(container);
  for(auto i = 0; i < FIELD_COUNT; ++i) {
    if(i != 0) {
      container_layout->addWidget(m_separators[i - 1]);
    }
    container_layout->addWidget(m_fields[i]);
  }
  m_input_box = make_input_box(container);
  enclose(*this, *m_input_box);
  for(auto field : m_fields) {
    link(*this, *field);
  }
  for(auto separator : m_separators) {
    link(*this, *separator);
  }
  setFocusPolicy(Qt::StrongFocus);
  setFocusProxy(m_input_box);
  proxy_style(*this, *m_input_box);
  set_style(*this, DEFAULT_STYLE());
  set_format(DEFAULT_FORMAT);
  on_current(m_current->get());
  on_style();
  m_style_connection = connect_style_signal(*this,
    std::bind_front(&DurationBox::on_style, this));
  for(auto field : m_fields) {
    field->connect_submit_signal([=] (const auto& submission) {
      if(field->hasFocus()) {
        on_submit();
      }
    });
    field->connect_reject_signal([=] (const auto&) { on_reject(); });
  }
  m_current->connect_update_signal(
    std::bind_front(&DurationBox::on_current, this));
}

const std::shared_ptr<OptionalDurationModel>& DurationBox::get_current() const {
  return m_current;
}

bool DurationBox::is_read_only() const {
  return m_is_read_only;
}

void DurationBox::set_read_only(bool is_read_only) {
  if(is_read_only == m_is_read_only) {
    return;
  }
  m_is_read_only = is_read_only;
  for(auto field : m_fields) {
    field->set_read_only(m_is_read_only);
  }
  if(m_is_read_only) {
    match(*m_input_box, ReadOnly());
  } else {
    unmatch(*m_input_box, ReadOnly());
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

bool DurationBox::test_format(const QString& format) const {
  return match_format(format).is_initialized();
}

bool DurationBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::FocusOut) {
    if(!m_is_read_only && std::none_of(m_fields.begin(), m_fields.end(),
        [] (auto field) { return field->hasFocus(); })) {
      if(m_has_update) {
        on_submit();
      } else {
        update_empty_fields();
      }
    }
  } else if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    auto& fields = m_fields;
    auto field_count = static_cast<int>(fields.size());
    auto is_set = [] (auto field) {
      return field->get_current()->get().is_initialized();
    };
    auto focused = [&] {
      for(auto i = 0; i < field_count; ++i) {
        if(!fields[i]->isHidden() && fields[i]->hasFocus()) {
          return i;
        }
      }
      return -1;
    }();
    auto find_previous = [&] (int index) {
      for(auto i = index - 1; i >= 0; --i) {
        if(!fields[i]->isHidden()) {
          return i;
        }
      }
      return -1;
    };
    auto find_next = [&] (int index) {
      for(auto i = index + 1; i < field_count; ++i) {
        if(!fields[i]->isHidden()) {
          return i;
        }
      }
      return -1;
    };
    auto focus_field = [&] (int index) {
      fields[index]->setFocus();
      if(auto editor = fields[index]->findChild<QLineEdit*>()) {
        editor->selectAll();
      }
    };
    auto previous = focused < 0 ? -1 : find_previous(focused);
    auto next = focused < 0 ? -1 : find_next(focused);
    auto is_field_empty = focused < 0 || !is_set(fields[focused]);
    auto has_value = std::any_of(fields.begin(), fields.end(), is_set);
    if(key_event.key() == Qt::Key_Enter || key_event.key() == Qt::Key_Return) {
      if(is_field_empty && has_value) {
        on_submit();
        return true;
      }
    } else if(focused < 0) {
      return QWidget::eventFilter(watched, event);
    } else if(key_event.key() == Qt::Key_Left && previous >= 0) {
      if(auto editor = fields[focused]->findChild<QLineEdit*>()) {
        if(editor->cursorPosition() == 0) {
          focus_field(previous);
        }
      }
    } else if(key_event.key() == Qt::Key_Right && next >= 0) {
      if(auto editor = fields[focused]->findChild<QLineEdit*>()) {
        if(editor->cursorPosition() == editor->text().size()) {
          focus_field(next);
        }
      }
    } else if(key_event.key() == Qt::Key_Backspace && previous >= 0) {
      if(auto editor = fields[focused]->findChild<QLineEdit*>()) {
        if(editor->cursorPosition() == 0 && !editor->hasSelectedText()) {
          auto previous_field = fields[previous];
          previous_field->setFocus();
          if(auto previous_editor =
              previous_field->findChild<QLineEdit*>()) {
            previous_editor->setCursorPosition(
              previous_editor->text().length());
            QCoreApplication::sendEvent(previous_editor, &key_event);
          }
          return true;
        }
      }
    } else if(!m_is_read_only && next >= 0 && key_event.text().size() == 1 &&
        key_event.text().front().isDigit()) {
      if(auto editor = fields[focused]->findChild<QLineEdit*>()) {
        auto text = editor->text();
        auto is_complete = !text.isEmpty() && !editor->hasSelectedText() &&
          editor->cursorPosition() == text.size();
        auto is_terminal = focused != 0 &&
          key_event.text().front() >= '6' &&
          (text.isEmpty() || editor->selectedText() == text);
        if(is_complete || is_terminal) {
          editor->insert(key_event.text());
          focus_field(next);
          return true;
        }
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DurationBox::set_format(const QString& format) {
  if(test_format(format)) {
    m_format = format;
  } else {
    m_format = DEFAULT_FORMAT;
  }
  auto format_fields = *match_format(m_format);
  auto has_fractional_seconds = format_fields.m_fractional_digits != 0;
  auto visibility = std::array<bool, FIELD_COUNT>{format_fields.m_has_hours,
    format_fields.m_has_minutes, format_fields.m_has_seconds,
    has_fractional_seconds};
  for(auto i = std::size_t(0); i < m_fields.size(); ++i) {
    m_fields[i]->setVisible(visibility[i]);
    std::static_pointer_cast<FieldModel>(
      m_fields[i]->get_current())->m_is_active = visibility[i];
  }
  for(auto i = 0; i < FIELD_COUNT - 1; ++i) {
    m_separators[i]->setVisible(visibility[i] && visibility[i + 1]);
  }
  auto is_truncated = false;
  auto is_valid = true;
  for(auto i = std::size_t(0); i < m_fields.size(); ++i) {
    auto& current = m_fields[i]->get_current();
    if(!visibility[i] && current->get() && *current->get() != 0) {
      if(current->set(0) == QValidator::State::Invalid) {
        is_valid = false;
      } else {
        is_truncated = true;
      }
    }
  }
  if(!is_valid) {
    if(!m_is_rejected) {
      m_is_rejected = true;
      match(*m_input_box, Rejected());
    }
  } else if(is_truncated) {
    m_submission = m_current->get();
    m_has_update = false;
  }
  if(!has_fractional_seconds) {
    return;
  }
  auto fractional_digits = format_fields.m_fractional_digits;
  std::static_pointer_cast<FractionalSecondModel>(
    m_fields[FRACTIONAL_SECOND]->get_current())->set_fractional_digits(
      fractional_digits);
  auto placeholder = QString(fractional_digits, 'f');
  m_fields[FRACTIONAL_SECOND]->set_placeholder(placeholder);
  m_fields[FRACTIONAL_SECOND]->setMinimumWidth(get_text_width(
    *find_focus_proxy(*m_fields[FRACTIONAL_SECOND]), placeholder));
  update_style(*m_fields[FRACTIONAL_SECOND], [&] (auto& style) {
    style.get(Any()).set(LeadingZeros(fractional_digits));
  });
}

void DurationBox::on_current(const optional<time_duration>& current) {
  m_has_update = current != m_submission;
  if(m_is_null != !current) {
    m_is_null = !current;
    if(m_is_null) {
      match(*this, NullCurrent());
    } else {
      unmatch(*this, NullCurrent());
    }
  }
  if(m_is_rejected) {
    m_is_rejected = false;
    unmatch(*m_input_box, Rejected());
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
    match(*m_input_box, Rejected());
  }
}

void DurationBox::on_style() {
  auto& stylist = find_stylist(*this);
  auto& block = stylist.get_computed_block();
  for(auto& property : block) {
    property.visit(
      [&] (const TextAlign& alignment) {
        stylist.evaluate(alignment, [=] (auto alignment) {
          auto body_layout = m_input_box->get_body()->layout();
          body_layout->setAlignment(alignment);
          body_layout->update();
        });
      },
      [&] (const Format& format) {
        stylist.evaluate(format, [=] (const auto& value) {
          if(value != m_format) {
            set_format(value);
          }
        });
      });
  }
}

void DurationBox::update_empty_fields() {
  if(m_submission) {
    for(auto field : m_fields) {
      if(!field->get_current()->get()) {
        field->get_current()->set(0);
      }
    }
  }
}

DurationBox* Spire::make_time_box(const optional<time_duration>& time,
    QWidget* parent) {
  return new TimeBox(make_time_of_day_model(time), parent);
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
