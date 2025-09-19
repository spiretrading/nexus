#include "Spire/Ui/DateBox.hpp"
#include <QCoreApplication>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ScalarValueModelDecorator.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OverlayPanel.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using Separator = StateSelector<void, struct SeparatorTag>;
  using InvalidCurrent = StateSelector<void, struct InvalidCurrentTag>;

  void apply_integer_box_style(StyleSheet& style) {
    style.get(Any()).
      set(BackgroundColor(Qt::transparent)).
      set(border_size(0)).
      set(horizontal_padding(0));
    style.get(Any() > (DownButton() || UpButton())).set(Visibility::NONE);
    style.get(Any() > is_a<TextBox>()).set(TextAlign(Qt::AlignCenter));
  }

  void apply_date_box_style(StyleSheet& style) {
    style.get(Any()).
      set(PaddingRight(0)).
      set(vertical_padding(0));
    style.get(ReadOnly() > is_a<Button>()).set(Visibility::NONE);
    style.get((InvalidCurrent() || Disabled()) > Separator()).
      set(TextColor(0xC8C8C8));
  }

  auto make_year_box(const std::shared_ptr<OptionalIntegerModel>& current) {
    auto modifiers = QHash<Qt::KeyboardModifier, int>(
      {{Qt::NoModifier, 1}, {Qt::AltModifier, 5}, {Qt::ControlModifier, 10}});
    auto box = new IntegerBox(current, std::move(modifiers));
    box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    box->set_placeholder("YYYY");
    box->setFixedWidth(scale_width(32));
    update_style(*box, [] (auto& style) {
      apply_integer_box_style(style);
      style.get(Any()).set(LeadingZeros(4));
    });
    return box;
  }

  auto make_month_box(const std::shared_ptr<OptionalIntegerModel>& current) {
    auto modifiers = QHash<Qt::KeyboardModifier, int>(
      {{Qt::NoModifier, 1}, {Qt::AltModifier, 3}, {Qt::ControlModifier, 6}});
    auto box = new IntegerBox(current, std::move(modifiers));
    box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    box->set_placeholder("MM");
    box->setFixedWidth(scale_width(22));
    update_style(*box, [] (auto& style) {
      apply_integer_box_style(style);
      style.get(Any()).set(LeadingZeros(2));
    });
    return box;
  }

  auto make_day_box(const std::shared_ptr<OptionalIntegerModel>& current) {
    auto modifiers = QHash<Qt::KeyboardModifier, int>(
      {{Qt::NoModifier, 1}, {Qt::AltModifier, 7}, {Qt::ControlModifier, 14}});
    auto box = new IntegerBox(current, std::move(modifiers));
    box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    box->set_placeholder("DD");
    box->setFixedWidth(scale_width(20));
    update_style(*box, [] (auto& style) {
      apply_integer_box_style(style);
      style.get(Any()).set(LeadingZeros(2));
    });
    return box;
  }

  auto make_dash() {
    auto label = make_label("-");
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    update_style(*label, [] (auto& style) {
      style.get(Any()).set(TextAlign(Qt::AlignCenter));
    });
    return label;
  }

  auto make_calendar_button() {
    auto button = make_icon_button(imageFromSvg(":/Icons/calendar.svg",
      scale(20, 20)));
    button->setFixedSize(scale(20, 20));
    update_style(*button, [] (auto& style) {
      style.get(Any() > is_a<Icon>()).set(Fill(QColor(0x333333)));
      style.get(FocusVisible() > Body()).
        set(BackgroundColor(QColor(0xE0E0E0))).
        set(border_color(QColor(Qt::transparent)));
      style.get(FocusVisible() > is_a<Icon>()).set(Fill(QColor(0x4B23A0)));
    });
    return button;
  }
}

struct DateBox::DateComposerModel : ValueModel<optional<date>> {
  std::shared_ptr<OptionalDateModel> m_source;
  std::shared_ptr<LocalOptionalIntegerModel> m_year;
  std::shared_ptr<LocalOptionalIntegerModel> m_month;
  std::shared_ptr<LocalOptionalIntegerModel> m_day;
  QValidator::State m_state;
  LocalValueModel<optional<date>> m_current;
  scoped_connection m_source_connection;
  scoped_connection m_year_connection;
  scoped_connection m_month_connection;
  scoped_connection m_day_connection;

  DateComposerModel(std::shared_ptr<OptionalDateModel> source)
      : m_source(std::move(source)),
        m_year(std::make_shared<LocalOptionalIntegerModel>()),
        m_month(std::make_shared<LocalOptionalIntegerModel>()),
        m_day(std::make_shared<LocalOptionalIntegerModel>()),
        m_state(m_source->get_state()),
        m_current(m_source->get()),
        m_source_connection(m_source->connect_update_signal(
          std::bind_front(&DateComposerModel::on_current, this))),
        m_year_connection(m_year->connect_update_signal(
          std::bind_front(&DateComposerModel::on_update, this))),
        m_month_connection(m_month->connect_update_signal(
          std::bind_front(&DateComposerModel::on_update, this))),
        m_day_connection(m_day->connect_update_signal(
          std::bind_front(&DateComposerModel::on_update, this))) {
    if(m_source->get_minimum()) {
      m_year->set_minimum(static_cast<int>(m_source->get_minimum()->year()));
    } else {
      m_year->set_minimum(0);
    }
    if(m_source->get_maximum()) {
      m_year->set_maximum(static_cast<int>(m_source->get_maximum()->year()));
    } else {
      m_year->set_maximum(9999);
    }
    if(m_source->get_minimum() && m_source->get_maximum() &&
        m_source->get_minimum()->year() == m_source->get_maximum()->year()) {
      m_month->set_minimum(
        static_cast<int>(m_source->get_minimum()->month()));
      m_month->set_maximum(
        static_cast<int>(m_source->get_maximum()->month()));
    } else {
      m_month->set_minimum(1);
      m_month->set_maximum(12);
    }
    if(m_source->get_minimum() && m_source->get_maximum() &&
        m_source->get_minimum()->year() == m_source->get_maximum()->year() &&
        m_source->get_minimum()->month() ==
          m_source->get_maximum()->month()) {
      m_day->set_minimum(static_cast<int>(m_source->get_minimum()->day()));
      m_day->set_maximum(static_cast<int>(m_source->get_maximum()->day()));
    } else {
      m_day->set_minimum(1);
      m_day->set_maximum(31);
    }
    on_current(m_source->get());
  }

  QValidator::State get_state() const override {
    return m_state;
  }

  const Type& get() const override {
    return m_current.get();
  }

  QValidator::State test(const Type& value) const {
    return QValidator::State::Acceptable;
  }

  QValidator::State set(const Type& value) {
    m_state = QValidator::State::Acceptable;
    m_current.set(value);
    return m_state;
  }

  connection connect_update_signal(
      const UpdateSignal::slot_type& slot) const override {
    return m_current.connect_update_signal(slot);
  }

  void update() {
    if(m_year->get() &&
        m_month->get() && m_day->get()) {
      try {
        auto current = date(static_cast<unsigned short>(*m_year->get()),
          static_cast<unsigned short>(*m_month->get()),
          static_cast<unsigned short>(*m_day->get()));
        m_current.set(current);
        m_state = m_source->set(current);
      } catch(const std::out_of_range&) {
        m_state = QValidator::State::Intermediate;
      }
    } else if(!m_year->get() && !m_month->get() &&
        !m_day->get()) {
      m_current.set(none);
      m_state = m_source->set(none);
    } else {
      m_state = QValidator::State::Intermediate;
    }
  }

  void on_current(const optional<date>& current) {
    m_state = QValidator::State::Acceptable;
    if(current) {
      {
        auto blocker = shared_connection_block(m_year_connection);
        m_year->set(static_cast<int>(current->year()));
      }
      {
        auto blocker = shared_connection_block(m_month_connection);
        m_month->set(static_cast<int>(current->month()));
      }
      {
        auto blocker = shared_connection_block(m_day_connection);
        m_day->set(static_cast<int>(current->day()));
      }
    } else {
      {
        auto blocker = shared_connection_block(m_year_connection);
        m_year->set(none);
      }
      {
        auto blocker = shared_connection_block(m_month_connection);
        m_month->set(none);
      }
      {
        auto blocker = shared_connection_block(m_day_connection);
        m_day->set(none);
      }
    }
    m_current.set(current);
  }

  void on_update(const optional<int>& current) {
    update();
  }
};

DateBox::DateBox(QWidget* parent)
  : DateBox(day_clock::local_day()) {}

DateBox::DateBox(const optional<date>& current, QWidget* parent)
  : DateBox(std::make_shared<LocalOptionalDateModel>(current), parent) {}

DateBox::DateBox(std::shared_ptr<OptionalDateModel> current, QWidget* parent)
    : QWidget(parent),
      m_model(std::make_shared<DateComposerModel>(std::move(current))),
      m_submission(m_model->get()),
      m_is_read_only(false),
      m_is_rejected(false),
      m_focus_observer(*this),
      m_date_picker_panel(nullptr),
      m_date_picker_showing(false) {
  m_model->connect_update_signal(std::bind_front(&DateBox::on_current, this));
  m_date_components = new QWidget();
  m_date_components->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_date_components->setCursor(Qt::IBeamCursor);
  auto layout = make_hbox_layout(m_date_components);
  m_fields.m_year.m_box = make_year_box(m_model->m_year);
  layout->addWidget(m_fields.m_year.m_box);
  m_year_dash = make_dash();
  match(*m_year_dash, Separator());
  layout->addWidget(m_year_dash);
  m_fields.m_month.m_box = make_month_box(m_model->m_month);
  layout->addWidget(m_fields.m_month.m_box);
  auto month_dash = make_dash();
  match(*month_dash, Separator());
  layout->addWidget(month_dash);
  m_fields.m_day.m_box = make_day_box(m_model->m_day);
  layout->addWidget(m_fields.m_day.m_box);
  auto setup_editor = [=] (const IntegerBox& field, auto slot) {
    auto editor = field.findChild<QLineEdit*>();
    editor->installEventFilter(this);
    field.connect_submit_signal(
      std::bind_front(&DateBox::on_field_submit, this, std::ref(field)));
    field.connect_reject_signal(
      std::bind_front(&DateBox::on_field_reject, this));
    connect(editor, &QLineEdit::textEdited, std::bind_front(slot, this));
    return editor;
  };
  m_fields.m_year.m_editor =
    setup_editor(*m_fields.m_year.m_box, &DateBox::on_year_edited);
  m_fields.m_month.m_editor =
    setup_editor(*m_fields.m_month.m_box, &DateBox::on_month_edited);
  m_fields.m_day.m_editor =
    setup_editor(*m_fields.m_day.m_box, &DateBox::on_day_edited);
  auto body = new QWidget();
  body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto body_layout = make_hbox_layout(body);
  body_layout->addWidget(m_date_components);
  m_calendar_button = make_calendar_button();
  m_calendar_button->connect_click_signal(
    std::bind_front(&DateBox::on_button_click, this));
  body_layout->addSpacing(scale_width(2));
  body_layout->addWidget(m_calendar_button);
  body_layout->addSpacing(scale_width(2));
  m_input_box = make_input_box(body);
  enclose(*this, *m_input_box);
  proxy_style(*this, *m_input_box);
  link(*this, *m_calendar_button);
  link(*this, *m_year_dash);
  link(*this, *month_dash);
  update_style(*this, apply_date_box_style);
  m_style_connection = connect_style_signal(*this,
    std::bind_front(&DateBox::on_style, this));
  m_focus_observer.connect_state_signal(
    std::bind_front(&DateBox::on_focus, this));
}

const std::shared_ptr<OptionalDateModel>& DateBox::get_current() const {
  return m_model->m_source;
}

const optional<date>& DateBox::get_submission() const {
  return m_submission;
}

bool DateBox::is_read_only() const {
  return m_is_read_only;
}

void DateBox::set_read_only(bool read_only) {
  if(m_is_read_only == read_only) {
    return;
  }
  m_is_read_only = read_only;
  m_fields.m_year.m_box->set_read_only(m_is_read_only);
  m_fields.m_month.m_box->set_read_only(m_is_read_only);
  m_fields.m_day.m_box->set_read_only(m_is_read_only);
  if(m_is_read_only) {
    m_date_components->setCursor(Qt::ArrowCursor);
    match(*m_input_box, ReadOnly());
    match(*this, ReadOnly());
  } else {
    m_date_components->setCursor(Qt::IBeamCursor);
    unmatch(*m_input_box, ReadOnly());
    unmatch(*this, ReadOnly());
  }
}

connection DateBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

connection DateBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}

bool DateBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    if(m_is_read_only) {
      return QWidget::eventFilter(watched, event);
    }
    auto key_event = *static_cast<QKeyEvent*>(event);
    if(watched == m_date_picker_panel && key_event.key() == Qt::Key_Escape) {
      QCoreApplication::sendEvent(this, event);
    } else if(key_event.key() == Qt::Key_Space) {
      show_date_picker();
      return true;
    } else if(watched == m_fields.m_year.m_editor) {
      if(key_event.key() == Qt::Key_Right &&
          m_fields.m_year.m_editor->cursorPosition() == 4) {
        focus_and_select_all(m_fields.m_month);
        return true;
      }
    } else if(watched == m_fields.m_month.m_editor) {
      if(key_event.key() == Qt::Key_Right &&
          m_fields.m_month.m_editor->cursorPosition() == 2) {
        focus_and_select_all(m_fields.m_day);
        return true;
      } else if(key_event.key() == Qt::Key_Left &&
          m_fields.m_month.m_editor->cursorPosition() == 0 &&
          m_fields.m_year.m_box->isVisible()) {
        focus_and_select_all(m_fields.m_year);
        return true;
      } else if(key_event.key() == Qt::Key_Backspace &&
          m_fields.m_month.m_editor->selectionLength() == 0 &&
          m_fields.m_month.m_editor->cursorPosition() == 0 &&
          m_fields.m_year.m_box->isVisible()) {
        focus_and_select_all(m_fields.m_year);
        QCoreApplication::sendEvent(m_fields.m_year.m_editor, event);
        return true;
      }
    } else if(watched == m_fields.m_day.m_editor) {
      if(key_event.key() == Qt::Key_Left &&
          m_fields.m_day.m_editor->cursorPosition() == 0) {
        focus_and_select_all(m_fields.m_month);
        return true;
      } else if(key_event.key() == Qt::Key_Backspace &&
          m_fields.m_day.m_editor->selectionLength() == 0 &&
          m_fields.m_day.m_editor->cursorPosition() == 0) {
        focus_and_select_all(m_fields.m_month);
        QCoreApplication::sendEvent(m_fields.m_month.m_editor, event);
        return true;
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DateBox::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    if(m_model->get() != m_submission) {
      m_model->m_source->set(m_submission);
    }
  }
  QWidget::keyPressEvent(event);
}

void DateBox::mousePressEvent(QMouseEvent* event) {
  auto focus_field = [] (const Field& field, bool to_home) {
    field.m_box->setFocus();
    if(to_home) {
      field.m_editor->home(false);
    } else {
      field.m_editor->end(false);
    }
  };
  if(event->button() == Qt::MouseButton::LeftButton) {
    auto global_pos = event->globalPos();
    auto dash_width = m_year_dash->width();
    auto half_dash_width = dash_width / 2;
    auto box_height = height();
    if(m_fields.m_year.m_box->isVisible()) {
      auto left_padding = QRect(mapToGlobal(QPoint(0, 0)),
        QSize(m_date_components->mapTo(this, m_fields.m_year.m_box->pos()).x(),
          box_height));
      if(left_padding.contains(global_pos)) {
        focus_field(m_fields.m_year, true);
        return;
      }
      auto right_padding =
        QRect(m_date_components->mapToGlobal(QPoint(m_year_dash->x(), 0)),
          QSize(half_dash_width, box_height));
      if(right_padding.contains(global_pos)) {
        focus_field(m_fields.m_year, false);
        return;
      }
    }
    auto month_left_padding = [&] {
      if(m_fields.m_year.m_box->isVisible()) {
        return QRect(m_date_components->mapToGlobal(
            QPoint(m_year_dash->x() + half_dash_width, 0)),
          QSize(dash_width - half_dash_width, box_height));
      }
      return QRect(mapToGlobal(QPoint(0, 0)),
        QSize(m_date_components->mapTo(this, m_fields.m_month.m_box->pos()).x(),
          box_height));
    }();
    if(month_left_padding.contains(global_pos)) {
      focus_field(m_fields.m_month, true);
      return;
    }
    auto month_right =
      m_fields.m_month.m_box->x() + m_fields.m_month.m_box->width();
    auto month_right_padding = QRect(
      m_date_components->mapToGlobal(QPoint(month_right, 0)),
      QSize(half_dash_width, box_height));
    if(month_right_padding.contains(global_pos)) {
      focus_field(m_fields.m_month, false);
      return;
    }
    auto day_left_padding = QRect(
      m_date_components->mapToGlobal(QPoint(month_right + half_dash_width, 0)),
      QSize(dash_width - half_dash_width, height()));
    if(day_left_padding.contains(global_pos)) {
      focus_field(m_fields.m_day, true);
      return;
    }
    auto day_right = m_fields.m_day.m_box->x() + m_fields.m_day.m_box->width();
    auto day_right_padding = QRect(
      m_date_components->mapToGlobal(QPoint(day_right, 0)),
      QSize(m_date_components->width() - day_right, height()));
    if(day_right_padding.contains(global_pos)) {
      focus_field(m_fields.m_day, false);
    }
  }
}

void DateBox::set_rejected(bool rejected) {
  if(m_is_rejected == rejected) {
    return;
  }
  m_is_rejected = rejected;
  auto apply = [&] (auto& f) {
    f(*this, Rejected());
    f(*m_input_box, Rejected());
  };
  if(rejected) {
    apply(match);
  } else {
    apply(unmatch);
  }
}

void DateBox::focus_and_select_all(const Field& field) {
  field.m_box->setFocus();
  field.m_editor->selectAll();
}

void DateBox::show_date_picker() {
  if(!m_date_picker_panel) {
    auto date_picker = new CalendarDatePicker(m_model->m_source);
    date_picker->connect_submit_signal([=] (auto date) {
      on_submit();
    });
    m_date_picker_panel = new OverlayPanel(*date_picker, *this);
    m_date_picker_panel->setWindowFlags(
      Qt::Popup | (m_date_picker_panel->windowFlags() & ~Qt::Tool));
    m_date_picker_panel->installEventFilter(this);
  }
  m_date_picker_showing = true;
  m_date_picker_panel->show();
  m_date_picker_showing = false;
  m_date_picker_panel->setFocus();
}

void DateBox::on_year_edited(const QString& text) {
  set_rejected(false);
  if(text.size() >= 4) {
    focus_and_select_all(m_fields.m_month);
  }
}

void DateBox::on_month_edited(const QString& text) {
  set_rejected(false);
  if((text.size() == 1 && text[0] >= '2' && text[0] <= '9' &&
      m_fields.m_month.m_editor->cursorPosition() == 1) || text.size() >= 2) {
    focus_and_select_all(m_fields.m_day);
  }
}

void DateBox::on_day_edited(const QString& text) {
  set_rejected(false);
}

void DateBox::on_button_click() {
  if(!m_is_read_only) {
    show_date_picker();
  }
}

void DateBox::on_field_submit(const IntegerBox& field,
    optional<int> submission) {
  if(field.hasFocus()) {
    on_submit();
  }
}

void DateBox::on_field_reject(optional<int> value) {
  set_rejected(true);
}

void DateBox::on_current(const optional<date>& current) {
  if(current) {
    unmatch(*this, InvalidCurrent());
  } else {
    match(*this, InvalidCurrent());
  }
}

void DateBox::on_submit() {
  if(m_model->get_state() != QValidator::State::Acceptable) {
    m_reject_signal(m_model->get());
    m_model->m_source->set(m_submission);
    set_rejected(true);
  } else if(!m_date_picker_showing) {
    if(m_date_picker_panel) {
      m_date_picker_panel->hide();
    }
    m_submission = m_model->get();
    m_submit_signal(m_submission);
  }
}

void DateBox::on_focus(FocusObserver::State state) {
  if(is_read_only()) {
    return;
  }
  if(state == FocusObserver::State::NONE) {
    unmatch(*m_input_box, FocusIn());
    if(m_date_picker_panel && m_date_picker_panel->isVisible()) {
      m_date_picker_panel->hide();
    } else if(m_submission != m_model->m_source->get()) {
      on_submit();
    }
  } else {
    match(*m_input_box, FocusIn());
  }
}

void DateBox::on_style() {
  auto& stylist = find_stylist(*this);
  auto& block = stylist.get_computed_block();
  for(auto& property : block) {
    property.visit(
      [&] (const TextAlign& alignment) {
        stylist.evaluate(alignment, [=] (auto alignment) {
          m_date_components->layout()->setAlignment(alignment);
          m_date_components->layout()->update();
        });
      },
      [&] (const YearField& year_field) {
        stylist.evaluate(year_field, [=] (auto is_year_visible) {
          m_fields.m_year.m_box->setVisible(is_year_visible);
          m_year_dash->setVisible(is_year_visible);
        });
      });
  }
}
