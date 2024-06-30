#include "Spire/Ui/DateBox.hpp"
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ScalarValueModelDecorator.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OverlayPanel.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).set(DateFormat::YYYYMMDD);
    style.get(Rejected()).
      set(BackgroundColor(chain(timeout(QColor(0xFFF1F1), milliseconds(250)),
        linear(QColor(0xFFF1F1), revert, milliseconds(300))))).
      set(border_color(
        chain(timeout(QColor(0xB71C1C), milliseconds(550)), revert)));
    return style;
  }

  void apply_integer_box_style(StyleSheet& style) {
    style.get(Any()).
      set(TextAlign(Qt::AlignCenter)).
      set(border_size(0)).
      set(padding(0));
    style.get(Any() > is_a<Button>()).set(Visibility::NONE);
  }

  auto make_year_box(const std::shared_ptr<OptionalIntegerModel>& current) {
    auto box = new IntegerBox(current);
    box->set_placeholder("YYYY");
    box->setFixedSize(scale(29, 26));
    update_style(*box, [&] (auto& style) {
      apply_integer_box_style(style);
      style.get(Any()).set(LeadingZeros(4));
    });
    return box;
  }

  auto make_month_box(const std::shared_ptr<OptionalIntegerModel>& current) {
    auto box = new IntegerBox(current);
    box->set_placeholder("MM");
    box->setFixedSize(scale(21, 26));
    update_style(*box, [&] (auto& style) {
      apply_integer_box_style(style);
      style.get(Any()).set(LeadingZeros(2));
    });
    return box;
  }

  auto make_day_box(const std::shared_ptr<OptionalIntegerModel>& current) {
    auto box = new IntegerBox(current);
    box->set_placeholder("DD");
    box->setFixedSize(scale(21, 26));
    update_style(*box, [&] (auto& style) {
      apply_integer_box_style(style);
      style.get(Any()).set(LeadingZeros(2));
    });
    return box;
  }

  auto make_dash() {
    auto label = make_label("-");
    label->setFixedSize(scale(4, 26));
    return label;
  }

  auto make_body(const std::shared_ptr<OptionalIntegerModel>& year,
      const std::shared_ptr<OptionalIntegerModel>& month,
      const std::shared_ptr<OptionalIntegerModel>& day) {
    auto body = new QWidget();
    auto layout = make_hbox_layout(body);
    layout->addSpacerItem(
      new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
    auto year_box = make_year_box(year);
    layout->addWidget(year_box);
    auto year_dash = make_dash();
    layout->addWidget(year_dash);
    auto month_box = make_month_box(month);
    layout->addWidget(month_box);
    layout->addWidget(make_dash());
    auto day_box = make_day_box(day);
    layout->addWidget(day_box);
    layout->addSpacerItem(
      new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
    return std::tuple(year_box, year_dash, month_box, day_box, body);
  }

  auto make_date_picker(
      const std::shared_ptr<OptionalDateModel>& current, QWidget* parent) {
    auto date_picker = new CalendarDatePicker(current);
    auto panel = new OverlayPanel(*date_picker, *parent);
    return panel;
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
      m_year->set_minimum(1400);
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

DateBox::DateBox(const optional<date>& current, QWidget* parent)
  : DateBox(std::make_shared<LocalOptionalDateModel>(current), parent) {}

DateBox::DateBox(std::shared_ptr<OptionalDateModel> current, QWidget* parent)
    : QWidget(parent),
      m_model(std::make_shared<DateComposerModel>(std::move(current))),
      m_submission(m_model->get()),
      m_is_read_only(false),
      m_is_rejected(false),
      m_focus_observer(*this),
      m_format(DateFormat::YYYYMMDD) {
  setCursor(Qt::IBeamCursor);
  m_model->connect_update_signal(std::bind_front(&DateBox::on_current, this));
  std::tie(m_year_box, m_year_dash, m_month_box, m_day_box, m_body) =
    make_body(m_model->m_year, m_model->m_month, m_model->m_day);
  for(auto box : {m_year_box, m_month_box, m_day_box}) {
    box->connect_submit_signal([=] (const auto& submission) {
      if(box->hasFocus()) {
        on_submit();
      }
    });
  }
  auto input_box = make_input_box(m_body);
  proxy_style(*this, *input_box);
  update_style(*input_box, [&] (auto& style) {
    style.get(Any()).set(padding(0));
  });
  enclose(*this, *input_box);
  set_style(*this, DEFAULT_STYLE());
  m_date_picker = make_date_picker(m_model->m_source, this);
  m_style_connection = connect_style_signal(*this, [=] { on_style(); });
  m_focus_observer.connect_state_signal([=] (auto state) { on_focus(state); });
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
  m_year_box->set_read_only(m_is_read_only);
  m_month_box->set_read_only(m_is_read_only);
  m_day_box->set_read_only(m_is_read_only);
  auto& layout = *static_cast<QBoxLayout*>(m_body->layout());
  if(m_is_read_only) {
    layout.removeItem(layout.itemAt(0));
    match(*this, ReadOnly());
  } else {
    layout.insertItem(
      0, new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
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

void DateBox::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    if(m_model->get() != m_submission) {
      m_model->m_source->set(m_submission);
    }
  }
  QWidget::keyPressEvent(event);
}

void DateBox::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::MouseButton::LeftButton) {
    auto right_padding =
      m_body->layout()->itemAt(m_body->layout()->count() - 1)->geometry();
    right_padding =
      QRect(m_body->mapToGlobal(right_padding.topLeft()), right_padding.size());
    if(right_padding.contains(event->globalPos())) {
      auto box = [&] () -> IntegerBox* {
        if(m_day_box->isVisible()) {
          return m_day_box;
        } else if(m_month_box->isVisible()) {
          return m_month_box;
        } else if(m_year_box->isVisible()) {
          return m_year_box;
        }
        return nullptr;
      }();
      if(box) {
        box->setFocus();
      }
    } else if(!m_is_read_only) {
      auto left_padding = m_body->layout()->itemAt(0)->geometry();
      left_padding =
        QRect(m_body->mapToGlobal(left_padding.topLeft()), left_padding.size());
      if(left_padding.contains(event->globalPos())) {
        auto box = [&] () -> IntegerBox* {
          if(m_year_box->isVisible()) {
            return m_year_box;
          } else if(m_month_box->isVisible()) {
            return m_month_box;
          } else if(m_day_box->isVisible()) {
            return m_day_box;
          }
          return nullptr;
        }();
        if(box) {
          box->setFocus();
          auto editor = box->findChild<QLineEdit*>();
          editor->setCursorPosition(0);
        }
      }
    }
  }
}

void DateBox::on_current(const optional<date>& current) {
  if(m_is_rejected) {
    m_is_rejected = false;
    unmatch(*this, Rejected());
  }
}

void DateBox::on_submit() {
  if(m_model->get_state() != QValidator::State::Acceptable) {
    auto current = m_model->get();
    auto submission = m_submission;
    m_reject_signal(current);
    m_model->m_source->set(m_submission);
    if(!m_is_rejected) {
      m_is_rejected = true;
      match(*this, Rejected());
    }
  } else {
    m_submission = m_model->get();
    m_submit_signal(m_submission);
  }
}

void DateBox::on_focus(FocusObserver::State state) {
  if(is_set(state, FocusObserver::State::FOCUS_IN)) {
    m_date_picker->show();
  } else {
    m_date_picker->hide();
    if(m_submission != m_model->m_source->get()) {
      on_submit();
    }
  }
}

void DateBox::on_style() {
  auto& stylist = find_stylist(*this);
  auto& block = stylist.get_computed_block();
  for(auto& property : block) {
    property.visit(
      [&] (const EnumProperty<DateFormat>& format) {
        stylist.evaluate(format, [=] (auto format) {
          if(format == m_format) {
            return;
          }
          m_format = format;
          m_year_box->setVisible(m_format == DateFormat::YYYYMMDD);
          m_year_dash->setVisible(m_format == DateFormat::YYYYMMDD);
        });
      });
  }
}
