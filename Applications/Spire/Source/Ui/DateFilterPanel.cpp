#include "Spire/Ui/DateFilterPanel.hpp"
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <QEvent>
#include "Spire/Spire/AssociativeValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Checkbox.hpp"
#include "Spire/Ui/DateBox.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto LABEL_CHECK_BUTTON_STYLE(StyleSheet style) {
    style.get((Checked() && !Hover() && !Press()) > Body()).
      set(BackgroundColor(QColor(0x7F5EEC))).
      set(TextColor(QColor(0xFFFFFF)));
    style.get((Checked() && Disabled()) > Body()).
      set(BackgroundColor(QColor(0xC8C8C8))).
      set(TextColor(QColor(0xFFFFFF)));
    return style;
  }

  auto display_text(DateFilterPanel::DateUnit unit) {
    if(unit == DateFilterPanel::DateUnit::DAY) {
      static const auto value = QObject::tr("Day");
      return value;
    } else if(unit == DateFilterPanel::DateUnit::WEEK) {
      static const auto value = QObject::tr("Week");
      return value;
    } else if(unit == DateFilterPanel::DateUnit::MONTH) {
      static const auto value = QObject::tr("Month");
      return value;
    } else {
      static const auto value = QObject::tr("Year");
      return value;
    }
  }

  auto make_date_row_layout(const QString& label, DateBox& date_box) {
    auto layout = make_hbox_layout();
    layout->addWidget(make_label(label));
    layout->addSpacing(scale_width(18));
    layout->addStretch();
    date_box.setFixedSize(scale(178, 26));
    layout->addWidget(&date_box);
    return layout;
  }

  auto make_range_setting_body(DateBox& start_date_box, DateBox& end_date_box) {
    auto body = new QWidget();
    auto layout = make_hbox_layout(body);
    layout->addSpacing(scale_width(24));
    auto date_range_layout = make_vbox_layout();
    date_range_layout->setSpacing(scale_height(10));
    date_range_layout->addLayout(
      make_date_row_layout(QObject::tr("Start Date"), start_date_box));
    date_range_layout->addLayout(
      make_date_row_layout(QObject::tr("End Date"), end_date_box));
    layout->addLayout(date_range_layout);
    return body;
  }

  auto make_offset_body(QWidget& offset_value, QWidget& units) {
    auto body = new QWidget();
    auto layout = make_hbox_layout(body);
    layout->addSpacing(scale_width(24));
    offset_value.setFixedSize(scale_width(60), scale_height(26));
    layout->addWidget(&offset_value);
    layout->addSpacing(scale_width(8));
    layout->addWidget(&units);
    layout->addStretch();
    return body;
  }

  auto make_line_element() {
    auto line_element = new Box(nullptr);
    auto style = StyleSheet();
    style.get(Any()).set(BackgroundColor(QColor(0xE0E0E0)));
    set_style(*line_element, std::move(style));
    line_element->setFixedHeight(scale_height(1));
    return line_element;
  }
}

class OffsetUnitButtonGroup : public QWidget {
  public:
    explicit OffsetUnitButtonGroup(
        std::shared_ptr<AssociativeValueModel<DateFilterPanel::DateUnit>> model,
        QWidget* parent = nullptr)
        : QWidget(parent),
          m_model(std::move(model)) {
      auto layout = make_hbox_layout(this);
      layout->setSpacing(scale_width(4));
      for(auto unit : {DateFilterPanel::DateUnit::DAY,
          DateFilterPanel::DateUnit::WEEK, DateFilterPanel::DateUnit::MONTH,
          DateFilterPanel::DateUnit::YEAR}) {
        auto button = make_label_button(display_text(unit));
        m_buttons[unit] = button;
        set_style(*button, LABEL_CHECK_BUTTON_STYLE(get_style(*button)));
        m_model->get_association(unit)->connect_update_signal(
          std::bind_front(&OffsetUnitButtonGroup::on_update, this, unit));
        button->connect_click_signal([=] { m_model->set(unit); });
        layout->addWidget(button);
      }
      on_update(m_model->get(), true);
    }

    const std::shared_ptr<AssociativeValueModel<DateFilterPanel::DateUnit>>&
        get_current() const {
      return m_model;
    }

  private:
    std::shared_ptr<AssociativeValueModel<DateFilterPanel::DateUnit>> m_model;
    std::unordered_map<DateFilterPanel::DateUnit, Button*> m_buttons;

    void on_update(DateFilterPanel::DateUnit unit, bool value) {
      auto button = m_buttons[unit];
      if(value) {
        match(*button, Checked());
      } else {
        unmatch(*button, Checked());
      }
    }
};

class DateFilterPanel::DateRangeTypeButtonGroup {
  public:
    DateRangeTypeButtonGroup(
        std::shared_ptr<AssociativeValueModel<DateRangeType>> model,
        const DateRange& date_range)
        : m_model(std::move(model)) {
      make_date_type_button(DateRangeType::OFFSET);
      make_date_type_button(DateRangeType::RANGE);
      set(date_range);
    }

    const std::shared_ptr<AssociativeValueModel<DateRangeType>>&
        get_current() const {
      return m_model;
    }

    CheckBox* get_button(DateRangeType type) {
      return m_buttons[type];
    }

    CheckBox* get_current_button() {
      return get_button(m_model->get());
    }

    void set(const DateRange& date_range) {
      if(date_range.m_offset && !date_range.m_start && !date_range.m_end) {
        m_model->set(DateRangeType::OFFSET);
      } else {
        m_model->set(DateRangeType::RANGE);
      }
    }

  private:
    std::shared_ptr<AssociativeValueModel<DateRangeType>> m_model;
    std::unordered_map<DateFilterPanel::DateRangeType, CheckBox*> m_buttons;

    void on_update(DateRangeType type, bool value) {
      m_buttons[type]->get_current()->set(value);
    }

    void make_date_type_button(DateRangeType type) {
      auto button = make_radio_button();
      button->set_label(display_text(type));
      button->get_current()->connect_update_signal([=] (auto value) {
        if(m_model->get() == type && !value) {
          button->get_current()->set(true);
        } else if(value) {
          m_model->set(type);
        }
      });
      m_model->get_association(type)->connect_update_signal(
        std::bind_front(&DateRangeTypeButtonGroup::on_update, this, type));
      m_buttons[type] = button;
    }

    QString display_text(DateRangeType type) {
      if(type == DateRangeType::OFFSET) {
        static const auto value = QObject::tr("Offset");
        return value;
      } else {
        static const auto value = QObject::tr("Range");
        return value;
      }
    }
};

struct DateFilterPanel::DateRangeComposerModel :
    ValueModel<DateFilterPanel::DateRange> {
  std::shared_ptr<DateRangeModel> m_source;
  std::shared_ptr<LocalOptionalDateModel> m_start;
  std::shared_ptr<LocalOptionalDateModel> m_end;
  std::shared_ptr<LocalOptionalIntegerModel> m_offset_value;
  std::shared_ptr<AssociativeValueModel<DateUnit>> m_date_unit;
  scoped_connection m_source_connection;
  scoped_connection m_start_connection;
  scoped_connection m_end_connection;
  scoped_connection m_offset_value_connection;
  scoped_connection m_date_unit_connection;

  explicit DateRangeComposerModel(
    std::shared_ptr<DateFilterPanel::DateRangeModel> source)
    : m_source(std::move(source)),
      m_start(std::make_shared<LocalOptionalDateModel>()),
      m_end(std::make_shared<LocalOptionalDateModel>()),
      m_offset_value(std::make_shared<LocalOptionalIntegerModel>()),
      m_date_unit(std::make_shared<AssociativeValueModel<DateUnit>>()),
BEAM_SUPPRESS_THIS_INITIALIZER()
      m_source_connection(m_source->connect_update_signal(
        std::bind_front(&DateRangeComposerModel::on_current, this))),
      m_start_connection(m_start->connect_update_signal(
        std::bind_front(&DateRangeComposerModel::on_start_update, this))),
      m_end_connection(m_end->connect_update_signal(
        std::bind_front(&DateRangeComposerModel::on_end_update, this))),
      m_offset_value_connection(m_offset_value->connect_update_signal(
        std::bind_front(&DateRangeComposerModel::on_offset_value_update, this))),
      m_date_unit_connection(m_date_unit->connect_update_signal(
        std::bind_front(&DateRangeComposerModel::on_date_unit_update, this))) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
    m_offset_value->set_minimum(1);
    on_current(m_source->get());
  } 

  QValidator::State get_state() const override {
    return m_source->get_state();
  }

  const Type& get() const override {
    return m_source->get();
  }

  QValidator::State set(const Type& value) override {
    return m_source->set(value);
  }

  connection connect_update_signal(
      const UpdateSignal::slot_type& slot) const override {
    return m_source->connect_update_signal(slot);
  }

  void on_current(const DateRange& current) {
    auto start_blocker = shared_connection_block(m_start_connection);
    m_start->set(current.m_start);
    auto end_blocker = shared_connection_block(m_end_connection);
    m_end->set(current.m_end);
    auto offset = [&] {
      if(current.m_offset) {
        return *current.m_offset;
      } else {
        return DateOffset{m_date_unit->get(), *m_offset_value->get_minimum()};
      }
    }();
    auto value_blocker = shared_connection_block(m_offset_value_connection);
    m_offset_value->set(offset.m_value);
    auto unit_blocker = shared_connection_block(m_date_unit_connection);
    m_date_unit->set(offset.m_unit);
  }

  void on_start_update(const optional<gregorian::date>& current) {
    auto blocker = shared_connection_block(m_source_connection);
    auto date_range = get();
    date_range.m_start = current;
    set(date_range);
  }

  void on_end_update(const optional<gregorian::date>& current) {
    auto blocker = shared_connection_block(m_source_connection);
    auto date_range = get();
    date_range.m_end = current;
    set(date_range);
  }

  void on_offset_value_update(const optional<int>& current) {
    auto blocker = shared_connection_block(m_source_connection);
    auto date_range = get();
    if(date_range.m_offset) {
      date_range.m_offset->m_value = *current;
    } else {
      date_range.m_offset = DateOffset{m_date_unit->get(), *current};
    }
    set(date_range);
  }

  void on_date_unit_update(DateUnit current) {
    auto blocker = shared_connection_block(m_source_connection);
    auto date_range = get();
    if(date_range.m_offset) {
      date_range.m_offset->m_unit = current;
    } else {
      date_range.m_offset = DateOffset{current, *m_offset_value->get()};
    }
    set(date_range);
  }
};

DateFilterPanel::DateFilterPanel(DateRange default_range, QWidget& parent)
  : DateFilterPanel(std::make_shared<LocalValueModel<DateRange>>(),
      std::move(default_range), parent) {}

DateFilterPanel::DateFilterPanel(std::shared_ptr<DateRangeModel> model,
    DateRange default_range, QWidget& parent)
    : QWidget(&parent),
      m_model(std::make_unique<DateRangeComposerModel>(std::move(model))),
      m_default_date_range(std::move(default_range)),
      m_range_type_button_group(std::make_unique<DateRangeTypeButtonGroup>(
        std::make_shared<AssociativeValueModel<DateRangeType>>(),
          m_model->get())) {
  m_filter_panel = new FilterPanel(QObject::tr("Filter by Date"), this, parent);
  m_filter_panel->connect_reset_signal([=] { on_reset(); });
  m_range_type_button_group->get_current()->connect_update_signal(
    std::bind_front(&DateFilterPanel::on_date_range_type_current, this));
  auto offset_button =
    m_range_type_button_group->get_button(DateRangeType::OFFSET);
  offset_button->setFixedHeight(scale_height(16));
  auto layout = make_vbox_layout(this);
  layout->addWidget(offset_button, 0, Qt::AlignLeft);
  layout->addSpacing(scale_height(18));
  auto offset_value_box = new IntegerBox(m_model->m_offset_value);
  offset_value_box->connect_submit_signal(
    std::bind_front(&DateFilterPanel::on_offset_value_submit, this));
  auto unit_group = new OffsetUnitButtonGroup(m_model->m_date_unit);
  m_date_unit_connection = unit_group->get_current()->connect_update_signal(
    std::bind_front(&DateFilterPanel::on_date_unit_current, this));
  m_offset_body = make_offset_body(*offset_value_box, *unit_group);
  layout->addWidget(m_offset_body);
  layout->addSpacing(scale_height(18));
  layout->addWidget(make_line_element());
  layout->addSpacing(scale_height(18));
  auto range_button =
    m_range_type_button_group->get_button(DateRangeType::RANGE);
  range_button->setFixedHeight(scale_height(16));
  layout->addWidget(range_button, 0, Qt::AlignLeft);
  layout->addSpacing(scale_height(14));
  auto start_date_box = new DateBox(m_model->m_start);
  start_date_box->connect_submit_signal(
    std::bind_front(&DateFilterPanel::on_start_date_submit, this));
  auto end_date_box = new DateBox(m_model->m_end);
  end_date_box->connect_submit_signal(
    std::bind_front(&DateFilterPanel::on_end_date_submit, this));
  m_range_body = make_range_setting_body(*start_date_box, *end_date_box);
  layout->addWidget(m_range_body);
  on_date_range_type_current(m_range_type_button_group->get_current()->get());
  window()->setWindowFlags(Qt::Tool | (window()->windowFlags() & ~Qt::Popup));
  window()->installEventFilter(this);
}

const std::shared_ptr<DateFilterPanel::DateRangeModel>&
    DateFilterPanel::get_model() const {
  return m_model->m_source;
}

const DateFilterPanel::DateRange& DateFilterPanel::get_default_range() const {
  return m_default_date_range;
}

void DateFilterPanel::set_default_range(const DateRange& default_range) {
  m_default_date_range = default_range;
}

connection DateFilterPanel::connect_submit_signal(
    const typename SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool DateFilterPanel::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Close) {
    m_filter_panel->hide();
    hide();
  }
  return QWidget::eventFilter(watched, event);
}

bool DateFilterPanel::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_filter_panel->show();
    window()->activateWindow();
    m_range_type_button_group->get_current_button()->setFocus();
  } else if(event->type() == QEvent::HideToParent) {
    m_filter_panel->hide();
  }
  return QWidget::event(event);
}

void DateFilterPanel::on_date_range_type_current(DateRangeType type) {
  if(type == DateRangeType::OFFSET) {
    m_offset_body->setEnabled(true);
    m_range_body->setEnabled(false);
  } else {
    m_range_body->setEnabled(true);
    m_offset_body->setEnabled(false);
  }
}

void DateFilterPanel::on_date_unit_current(DateUnit unit) {
  auto date_range = m_model->get();
  date_range.m_start = none;
  date_range.m_end = none;
  if(date_range.m_offset) {
    date_range.m_offset->m_unit = unit;
  } else {
    date_range.m_offset = DateOffset{unit, *m_model->m_offset_value->get()};
  }
  m_submit_signal(date_range);
}

void DateFilterPanel::on_end_date_submit(
    const boost::optional<boost::gregorian::date>& submission) {
  auto date_range = m_model->get();
  if(date_range.m_start && submission && *date_range.m_start > *submission) {
    date_range.m_start = submission;
    m_model->set(date_range);
  }
  date_range.m_offset = none;
  m_submit_signal(date_range);
}

void DateFilterPanel::on_start_date_submit(
    const boost::optional<boost::gregorian::date>& submission) {
  auto date_range = m_model->get();
  if(date_range.m_end && submission && *date_range.m_end < *submission) {
    date_range.m_end = submission;
    m_model->set(date_range);
  }
  date_range.m_offset = none;
  m_submit_signal(date_range);
}

void DateFilterPanel::on_offset_value_submit(
    const boost::optional<int>& submission) {
  auto date_range = m_model->get();
  date_range.m_start = none;
  date_range.m_end = none;
  if(date_range.m_offset) {
    date_range.m_offset->m_value = *submission;
  } else {
    date_range.m_offset = DateOffset{m_model->m_date_unit->get(), *submission};
  }
  m_submit_signal(date_range);
}

void DateFilterPanel::on_reset() {
  auto unit_blocker = shared_connection_block(m_date_unit_connection);
  m_model->set(m_default_date_range);
  m_range_type_button_group->set(m_default_date_range);
  m_range_type_button_group->get_current_button()->setFocus();
  m_submit_signal(m_default_date_range);
}
