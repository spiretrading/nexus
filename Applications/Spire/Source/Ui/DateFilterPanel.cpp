#include "Spire/Ui/DateFilterPanel.hpp"
#include <QEvent>
#include <QStackedWidget>
#include "Spire/Spire/AssociativeValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/AdaptiveBox.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Checkbox.hpp"
#include "Spire/Ui/DateBox.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  constexpr DateFilterPanel::DateUnit DateUnits[] =
    {DateFilterPanel::DateUnit::DAY, DateFilterPanel::DateUnit::WEEK,
    DateFilterPanel::DateUnit::MONTH, DateFilterPanel::DateUnit::YEAR};

  template<class... Ts>
  struct Overloaded : Ts... {
    using Ts::operator()...;
  };

  template<class... Ts>
  Overloaded(Ts...) -> Overloaded<Ts...>;

  const QString& to_text(DateFilterPanel::DateUnit unit) {
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

  struct OffsetUnitButtonGroup : public QWidget {
    using DateUnit = DateFilterPanel::DateUnit;
    struct ButtonGroup {
      Button* m_small;
      Button* m_medium;
    };
    std::shared_ptr<AssociativeValueModel<DateUnit>> m_current;
    std::unordered_map<DateUnit, ButtonGroup> m_buttons;

    explicit OffsetUnitButtonGroup(
        std::shared_ptr<AssociativeValueModel<DateUnit>> current,
        QWidget* parent = nullptr)
        : QWidget(parent),
          m_current(std::move(current)) {
      auto adaptive_box = new AdaptiveBox();
      adaptive_box->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Preferred);
      auto small_layout = make_hbox_layout();
      small_layout->setAlignment(Qt::AlignLeft);
      small_layout->setSpacing(scale_width(4));
      auto medium_layout = make_hbox_layout();
      medium_layout->setAlignment(Qt::AlignLeft);
      medium_layout->setSpacing(scale_width(4));
      for(auto unit : DateUnits) {
        auto label = to_text(unit);
        auto small_button = make_button(label.left(1), unit);
        update_style(*small_button, [] (auto& style) {
          style.get(Any() > Body()).set(horizontal_padding(0));
        });
        small_button->setFixedWidth(scale_width(26));
        small_layout->addWidget(small_button);
        auto medium_button = make_button(label, unit);
        medium_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        medium_layout->addWidget(medium_button);
        m_buttons.emplace(unit, ButtonGroup{small_button, medium_button});
        m_current->get_association(unit)->connect_update_signal(
          std::bind_front(&OffsetUnitButtonGroup::on_current, this, unit));
      }
      adaptive_box->add(*small_layout);
      adaptive_box->add(*medium_layout);
      enclose(*this, *adaptive_box);
      setMinimumWidth(small_layout->sizeHint().width());
      setMaximumWidth(medium_layout->sizeHint().width());
      on_current(m_current->get(), true);
      m_buttons[DateUnit::YEAR].m_small->installEventFilter(this);
      m_buttons[DateUnit::YEAR].m_medium->installEventFilter(this);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::Show) {
        if(watched == m_buttons[DateUnit::YEAR].m_medium) {
          set_tab_order(&ButtonGroup::m_medium);
        } else if(watched == m_buttons[DateUnit::YEAR].m_small) {
          set_tab_order(&ButtonGroup::m_small);
        }
      }
      return QWidget::eventFilter(watched, event);
    }

    Button* make_button(const QString& label, DateFilterPanel::DateUnit unit) {
      auto button = make_label_button(label);
      update_style(*button, [] (auto& style) {
        style.get((Checked() && !Hover() && !Press()) > Body()).
          set(BackgroundColor(QColor(0x7F5EEC))).
          set(TextColor(QColor(0xFFFFFF)));
        style.get((Checked() && Disabled()) > Body()).
          set(BackgroundColor(QColor(0xC8C8C8))).
          set(TextColor(QColor(0xFFFFFF)));
      });
      button->connect_click_signal([=] {
        m_current->set(unit);
      });
      return button;
    }

    void set_tab_order(Button* ButtonGroup::* member) {
      QWidget::setTabOrder(
        m_buttons[DateFilterPanel::DateUnit::DAY].*member,
        m_buttons[DateFilterPanel::DateUnit::WEEK].*member);
      QWidget::setTabOrder(
        m_buttons[DateFilterPanel::DateUnit::WEEK].*member,
        m_buttons[DateFilterPanel::DateUnit::MONTH].*member);
      QWidget::setTabOrder(
        m_buttons[DateFilterPanel::DateUnit::MONTH].*member,
        m_buttons[DateFilterPanel::DateUnit::YEAR].*member);
    }

    void on_current(DateFilterPanel::DateUnit unit, bool value) {
      auto& button_group = m_buttons[unit];
      if(value) {
        match(*button_group.m_small, Checked());
        match(*button_group.m_medium, Checked());
      } else {
        unmatch(*button_group.m_small, Checked());
        unmatch(*button_group.m_medium, Checked());
      }
    }
  };

  std::tuple<QWidget*, IntegerBox*, OffsetUnitButtonGroup*> make_offset_widget(
      std::shared_ptr<LocalOptionalIntegerModel> offset_value_model,
      std::shared_ptr<AssociativeValueModel<DateFilterPanel::DateUnit>>
        date_unit_model) {
    auto widget = new QWidget();
    auto layout = make_hbox_layout(widget);
    layout->setAlignment(Qt::AlignLeft);
    auto offset_value_box = new IntegerBox(std::move(offset_value_model));
    offset_value_box->setFixedWidth(scale_width(60));
    layout->addWidget(offset_value_box);
    layout->addSpacing(scale_width(8));
    auto unit_group = new OffsetUnitButtonGroup(std::move(date_unit_model));
    unit_group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(unit_group, 1);
    return {widget, offset_value_box, unit_group};
  }

  std::tuple<QWidget*, DateBox*, DateBox*> make_range_widget(
      std::shared_ptr<OptionalDateModel> start_model,
      std::shared_ptr<OptionalDateModel> end_model) {
    auto widget = new QWidget();
    auto layout = make_hbox_layout(widget);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    auto start_date_box = new DateBox(std::move(start_model));
    start_date_box->setFixedWidth(scale_width(112));
    auto end_date_box = new DateBox(std::move(end_model));
    end_date_box->setFixedWidth(scale_width(112));
    layout->addWidget(start_date_box);
    layout->addSpacing(scale_width(4));
    layout->addWidget(end_date_box);
    return {widget, start_date_box, end_date_box};
  }
}

class DateFilterPanel::DateRangeModeButtonGroup {
  public:
    explicit DateRangeModeButtonGroup(
        std::shared_ptr<AssociativeValueModel<Mode>> current)
        : m_current(std::move(current)) {
      make_date_type_button(Mode::OFFSET);
      make_date_type_button(Mode::RANGE);
      auto mode = m_current->get();
      on_current(mode, m_current->get_association(mode)->get());
    }

    const std::shared_ptr<AssociativeValueModel<Mode>>& get_current() const {
      return m_current;
    }

    CheckBox* get_button(Mode mode) const {
      return m_buttons.at(mode);
    }

  private:
    std::shared_ptr<AssociativeValueModel<Mode>> m_current;
    std::unordered_map<Mode, CheckBox*> m_buttons;

    void make_date_type_button(Mode mode) {
      auto button = make_radio_button();
      button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
      button->set_label(to_text(mode));
      button->get_current()->connect_update_signal([=] (auto value) {
        if(m_current->get() == mode && !value) {
          button->get_current()->set(true);
        } else if(value) {
          m_current->set(mode);
        }
      });
      m_current->get_association(mode)->connect_update_signal(
        std::bind_front(&DateRangeModeButtonGroup::on_current, this, mode));
      m_buttons.emplace(mode, button);
    }

    const QString& to_text(Mode type) const {
      if(type == Mode::OFFSET) {
        static const auto value = QObject::tr("Offset");
        return value;
      } else {
        static const auto value = QObject::tr("Range");
        return value;
      }
    }

    void on_current(Mode mode, bool value) {
      m_buttons[mode]->get_current()->set(value);
    }
};

struct DateFilterPanel::DateRangeComposerModel :
    ValueModel<DateFilterPanel::DateRange> {
  std::shared_ptr<DateRangeModel> m_source;
  std::shared_ptr<LocalOptionalDateModel> m_start;
  std::shared_ptr<LocalOptionalDateModel> m_end;
  std::shared_ptr<LocalOptionalIntegerModel> m_offset_value;
  std::shared_ptr<AssociativeValueModel<DateUnit>> m_date_unit;
  std::shared_ptr<AssociativeValueModel<Mode>> m_mode;
  scoped_connection m_source_connection;
  scoped_connection m_start_connection;
  scoped_connection m_end_connection;
  scoped_connection m_offset_value_connection;
  scoped_connection m_date_unit_connection;
  scoped_connection m_mode_connection;

  explicit DateRangeComposerModel(
    std::shared_ptr<DateFilterPanel::DateRangeModel> source)
    : m_source(std::move(source)),
      m_start(std::make_shared<LocalOptionalDateModel>()),
      m_end(std::make_shared<LocalOptionalDateModel>()),
      m_offset_value(std::make_shared<LocalOptionalIntegerModel>()),
      m_date_unit(std::make_shared<AssociativeValueModel<DateUnit>>()),
      m_mode(std::make_shared<AssociativeValueModel<Mode>>()),
BEAM_SUPPRESS_THIS_INITIALIZER()
      m_source_connection(m_source->connect_update_signal(
        std::bind_front(&DateRangeComposerModel::on_current, this))),
      m_start_connection(m_start->connect_update_signal(
        std::bind_front(&DateRangeComposerModel::on_start_update, this))),
      m_end_connection(m_end->connect_update_signal(
        std::bind_front(&DateRangeComposerModel::on_end_update, this))),
      m_offset_value_connection(m_offset_value->connect_update_signal(
        std::bind_front(&DateRangeComposerModel::on_offset_value_update,
          this))),
      m_date_unit_connection(m_date_unit->connect_update_signal(
        std::bind_front(&DateRangeComposerModel::on_date_unit_update, this))),
      m_mode_connection(m_mode->connect_update_signal(
        std::bind_front(&DateRangeComposerModel::on_mode_update, this))) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
    m_offset_value->set_minimum(1);
    for(auto unit : DateUnits) {
      m_date_unit->get_association(unit);
    }
    m_mode->get_association(Mode::OFFSET);
    m_mode->get_association(Mode::RANGE);
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

  void update_absolute_date_range(const optional<date>& start_date,
      const optional<date>& end_date) {
    auto blocker = shared_connection_block(m_source_connection);
    set(AbsoluteDateRange{start_date.value_or(date()),
      end_date.value_or(date())});
  }

  void update_relative_date_range(DateUnit unit, const optional<int>& value) {
    auto blocker = shared_connection_block(m_source_connection);
    set(RelativeDateRange{unit,
      value.value_or(*m_offset_value->get_minimum())});
  }

  void on_current(const DateRange& current) {
    std::visit(Overloaded {
      [=] (const AbsoluteDateRange& date_range) {
        auto mode_blocker = shared_connection_block(m_mode_connection);
        m_mode->set(Mode::RANGE);
        auto start_blocker = shared_connection_block(m_start_connection);
        if(date_range.m_start.is_not_a_date()) {
          m_start->set(none);
        } else {
          m_start->set(date_range.m_start);
        }
        auto end_blocker = shared_connection_block(m_end_connection);
        if(date_range.m_end.is_not_a_date()) {
          m_end->set(none);
        } else {
          m_end->set(date_range.m_end);
        }
        auto value_blocker = shared_connection_block(m_offset_value_connection);
        m_offset_value->set(*m_offset_value->get_minimum());
        auto unit_blocker = shared_connection_block(m_date_unit_connection);
        m_date_unit->set(DateUnit::DAY);
      },
      [=] (const RelativeDateRange& date_range) {
        auto mode_blocker = shared_connection_block(m_mode_connection);
        m_mode->set(Mode::OFFSET);
        auto start_blocker = shared_connection_block(m_start_connection);
        m_start->set(none);
        auto end_blocker = shared_connection_block(m_end_connection);
        m_end->set(none);
        auto value_blocker = shared_connection_block(m_offset_value_connection);
        m_offset_value->set(date_range.m_value);
        auto unit_blocker = shared_connection_block(m_date_unit_connection);
        m_date_unit->set(date_range.m_unit);
      }}, current);
  }

  void on_start_update(const optional<date>& current) {
    update_absolute_date_range(current, m_end->get());
  }

  void on_end_update(const optional<gregorian::date>& current) {
    update_absolute_date_range(m_start->get(), current);
  }

  void on_offset_value_update(const optional<int>& current) {
    update_relative_date_range(m_date_unit->get(), current);
  }

  void on_date_unit_update(DateUnit current) {
    update_relative_date_range(current, m_offset_value->get());
  }

  void on_mode_update(Mode current) {
    if(current == Mode::OFFSET) {
      update_relative_date_range(m_date_unit->get(), m_offset_value->get());
    } else {
      update_absolute_date_range(m_start->get(), m_end->get());
    }
  }
};

DateFilterPanel::DateFilterPanel(std::shared_ptr<DateRangeModel> current,
    QWidget* parent)
    : QWidget(parent),
      m_model(std::make_unique<DateRangeComposerModel>(std::move(current))),
      m_default_date_range(m_model->m_source->get()),
      m_range_mode_button_group(std::make_unique<DateRangeModeButtonGroup>(
        m_model->m_mode)) {
  auto body = new QWidget();
  auto layout = make_hbox_layout(body);
  layout->setSpacing(scale_width(18));
  layout->addWidget(m_range_mode_button_group->get_button(Mode::OFFSET));
  layout->addWidget(m_range_mode_button_group->get_button(Mode::RANGE));
  auto [offset_widget, offset_value_box, unit_group] =
    make_offset_widget(m_model->m_offset_value, m_model->m_date_unit);
  unit_group->m_buttons[DateUnit::DAY].m_medium->installEventFilter(this);
  unit_group->m_buttons[DateUnit::DAY].m_small->installEventFilter(this);
  m_offset_value_box = offset_value_box;
  auto [range_widget, start_date_box, end_date_box] = make_range_widget(
    m_model->m_start, m_model->m_end);
  m_start_date_box = start_date_box;
  auto parameter_widget = new QStackedWidget();
  parameter_widget->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  parameter_widget->addWidget(offset_widget);
  parameter_widget->addWidget(range_widget);
  layout->addWidget(parameter_widget, 1);
  auto filter_panel = new FilterPanel(*body);
  filter_panel->connect_reset_signal(
    std::bind_front(&DateFilterPanel::on_reset, this));
  enclose(*this, *filter_panel);
  proxy_style(*this, *filter_panel);
  auto switch_parameter_widget = [=] (Mode mode) {
    parameter_widget->setCurrentIndex(static_cast<int>(mode));
  };
  switch_parameter_widget(m_range_mode_button_group->get_current()->get());
  m_range_mode_button_group->get_current()->connect_update_signal(
    switch_parameter_widget);
}

const std::shared_ptr<DateFilterPanel::DateRangeModel>&
    DateFilterPanel::get_current() const {
  return m_model->m_source;
}

bool DateFilterPanel::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Show) {
    QWidget::setTabOrder(
      find_focus_proxy(*m_offset_value_box), static_cast<QWidget*>(watched));
  }
  return QWidget::eventFilter(watched, event);
}

void DateFilterPanel::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  if(m_range_mode_button_group->get_current()->get() == Mode::OFFSET) {
    m_offset_value_box->setFocus();
  } else {
    m_start_date_box->setFocus();
  }
}

void DateFilterPanel::on_reset() {
  m_model->set(m_default_date_range);
}
