#include "Spire/Ui/DateBox.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto field_style(StyleSheet style, int leading_zeros) {
    style.get(Any()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_size(0)).
      set(horizontal_padding(scale_width(0))).
      set(LeadingZeros(leading_zeros)).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    style.get(Any() > is_a<Button>()).set(Visibility::NONE);
    return style;
  }

  auto make_integer_field(int min, int max, const QString& placeholder,
      int leading_zeros, const QSize& size) {
    auto model = std::make_shared<LocalOptionalIntegerModel>();
    model->set_minimum(min);
    model->set_maximum(max);
    auto field = new IntegerBox(model, {});
    field->set_placeholder(placeholder);
    set_style(*field, field_style(get_style(*field), leading_zeros));
    field->setFixedSize(size);
    return field;
  }

  auto make_dash() {
    auto dash = make_label(QObject::tr("-"));
    dash->setFixedSize(scale(10, 26));
    auto style = get_style(*dash);
    style.get(Disabled() && ReadOnly()).
      set(TextAlign(Qt::AlignCenter));
    set_style(*dash, std::move(style));
    return dash;
  }
}

DateBox::DateBox(const optional<date>& current, QWidget* parent)
  : DateBox(std::make_shared<LocalOptionalDateModel>(current), parent) {}

DateBox::DateBox(std::shared_ptr<OptionalDateModel> model, QWidget* parent)
    : QWidget(parent),
      m_focus_observer(*this),
      m_model(std::move(model)),
      m_submission(m_model->get_current()),
      m_is_modified(false),
      m_is_rejected(false) {
  m_current_connection = m_model->connect_current_signal(
    [=] (const auto& current) {
      on_current(current);
    });
  m_focus_observer.connect_state_signal([=] (auto state) {
    on_focus(state);
  });
  auto body = new QWidget(this);
  auto body_layout = new QHBoxLayout(body);
  body_layout->setContentsMargins({});
  body_layout->setSpacing(0);
  body_layout->addStretch(1);
  auto input_box = make_input_box(body, this);
  proxy_style(*this, *input_box);
  auto style = get_style(*input_box);
  style.get(Any()).
    set(vertical_padding(0));
  style.get(Rejected()).
    set(BackgroundColor(chain(timeout(QColor(0xFFF1F1), milliseconds(250)),
      linear(QColor(0xFFF1F1), revert, milliseconds(300))))).
    set(border_color(
      chain(timeout(QColor(0xB71C1C), milliseconds(550)), revert)));
  set_style(*this, std::move(style));
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(input_box);
  m_year_field = make_integer_field(0, 9999, tr("YYYY"), 4, scale(38, 26));
  m_year_connection = m_year_field->get_model()->connect_current_signal(
    [=] (const auto& current) { on_field_current(); });
  m_year_field->connect_reject_signal([=] (const auto& value) {
    on_reject();
  });
  m_year_field->findChild<QLineEdit*>()->installEventFilter(this);
  body_layout->addWidget(m_year_field);
  m_year_dash = make_dash();
  body_layout->addWidget(m_year_dash);
  m_month_field = make_integer_field(1, 12, tr("MM"), 2, scale(28, 26));
  m_month_connection = m_month_field->get_model()->connect_current_signal(
    [=] (const auto& current) { on_field_current(); });
  m_month_field->connect_reject_signal([=] (const auto& value) {
    on_reject();
  });
  m_month_field->findChild<QLineEdit*>()->installEventFilter(this);
  body_layout->addWidget(m_month_field);
  body_layout->addWidget(make_dash());
  m_day_field = make_integer_field(1, 31, tr("DD"), 2, scale(28, 26));
  m_day_connection = m_day_field->get_model()->connect_current_signal(
    [=] (const auto& current) { on_field_current(); });
  m_day_field->connect_reject_signal([=] (const auto& value) {
    on_reject();
  });
  m_day_field->findChild<QLineEdit*>()->installEventFilter(this);
  body_layout->addWidget(m_day_field);
  body_layout->addStretch(1);
  auto calendar = new CalendarDatePicker(m_model, this);
  m_panel = new OverlayPanel(*calendar, *this);
  m_panel->set_is_draggable(false);
  m_panel->set_closed_on_focus_out(false);
  calendar->adjustSize();
  setFixedWidth(calendar->width());
  for(auto& property : get_evaluated_block(*m_panel->findChild<Box*>())) {
    property.visit(
      [&] (std::in_place_type_t<BorderLeftSize>, int size) {
        setFixedWidth(width() + size);
      },
      [&] (std::in_place_type_t<BorderRightSize>, int size) {
        setFixedWidth(width() + size);
      });
  }
  populate_input_fields();
}

const std::shared_ptr<OptionalDateModel>& DateBox::get_model() const {
  return m_model;
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
    auto e = static_cast<QKeyEvent*>(event);
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
      on_submit();
    } else if(e->key() == Qt::Key_Escape) {
      m_model->set_current(m_submission);
      m_is_modified = false;
    }
  }
  return QWidget::eventFilter(watched, event);
}

optional<date> DateBox::get_current() const {
  auto year = m_year_field->get_model()->get_current();
  auto month = m_month_field->get_model()->get_current();
  auto day = m_day_field->get_model()->get_current();
  if(year && month && day) {
    try {
      return date(*year, *month, *day);
    } catch (const std::exception&) {
      return {};
    }
  }
  return {};
}

void DateBox::populate_input_fields() {
  auto year_blocker = shared_connection_block(m_year_connection);
  auto month_blocker = shared_connection_block(m_month_connection);
  auto day_blocker = shared_connection_block(m_day_connection);
  if(auto current = m_model->get_current()) {
    m_year_field->get_model()->set_current(optional<int>(current->year()));
    m_month_field->get_model()->set_current(optional<int>(current->month()));
    m_day_field->get_model()->set_current(optional<int>(current->day()));
  } else {
    m_year_field->get_model()->set_current({});
    m_month_field->get_model()->set_current({});
    m_day_field->get_model()->set_current({});
  }
}

void DateBox::on_current(const optional<date>& current) {
  m_is_modified = true;
  populate_input_fields();
  if(m_is_rejected) {
    m_is_rejected = false;
    unmatch(*this, Rejected());
  }
}

void DateBox::on_field_current() {
  m_is_modified = true;
  auto current = get_current();
  if(current && current != m_model->get_current()) {
    auto blocker = shared_connection_block(m_current_connection);
    m_model->set_current(current);
  }
}

void DateBox::on_focus(FocusObserver::State state) {
  if(is_set(FocusObserver::State::FOCUS_IN, state)) {
    m_panel->show();
  } else {
    if(m_is_modified) {
      on_submit();
    }
    m_panel->hide();
  }
}

void DateBox::on_reject() {
  m_is_rejected = true;
}

void DateBox::on_submit() {

  if(auto current = get_current();
      m_model->get_minimum() <= current && current <= m_model->get_maximum()) {
    m_submission = m_model->get_current();
    m_submit_signal(m_submission);
    m_is_modified = false;
  } else {
    auto submission = m_model->get_current();
    m_model->set_current(m_submission);
    m_is_modified = false;
    m_reject_signal(submission);
    if(!m_is_rejected) {
      m_is_rejected = true;
      match(*this, Rejected());
    }
  }
}
