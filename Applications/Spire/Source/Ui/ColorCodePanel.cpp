#include "Spire/Ui/ColorCodePanel.hpp"
#include <QKeyEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/HexColorBox.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/PercentBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct CustomIntegerBox : IntegerBox {
    using EditingFinishSignal = Signal<void (const optional<int>& value)>;
    mutable EditingFinishSignal m_editing_finish_signal;

    explicit CustomIntegerBox(std::shared_ptr<OptionalIntegerModel> current,
      QWidget* parent = nullptr)
      : IntegerBox(std::move(current), parent) {}

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::KeyPress) {
        auto& key_event = *static_cast<QKeyEvent*>(event);
        if(key_event.key() == Qt::Key_Enter ||
            key_event.key() == Qt::Key_Return) {
          m_editing_finish_signal(get_current()->get());
        }
      } else if(event->type() == QEvent::FocusOut) {
        m_editing_finish_signal(get_current()->get());
      }
      return IntegerBox::eventFilter(watched, event);
    }

    void showEvent(QShowEvent* event) override {
      find_focus_proxy(*this)->installEventFilter(this);
      IntegerBox::showEvent(event);
    }
  };

  struct CustomPercentBox : PercentBox {
    using EditingFinishSignal = Signal<void (const optional<Decimal>& value)>;
    mutable EditingFinishSignal m_editing_finish_signal;

    explicit CustomPercentBox(std::shared_ptr<OptionalDecimalModel> current,
      QWidget* parent = nullptr)
      : PercentBox(std::move(current), parent) {}

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::KeyPress) {
        auto& key_event = *static_cast<QKeyEvent*>(event);
        if(key_event.key() == Qt::Key_Enter ||
            key_event.key() == Qt::Key_Return) {
          m_editing_finish_signal(get_current()->get());
        }
      } else if(event->type() == QEvent::FocusOut) {
        m_editing_finish_signal(get_current()->get());
      }
      return PercentBox::eventFilter(watched, event);
    }

    void showEvent(QShowEvent* event) override {
      find_focus_proxy(*this)->installEventFilter(this);
      PercentBox::showEvent(event);
    }
  };

  auto get_character_width() {
    static auto width = [] {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Normal);
      font.setPixelSize(scale_width(12));
      return QFontMetrics(font).averageCharWidth();
    }();
    return width;
  }

  auto get_component_minimum_width() {
    static auto width = 3 * get_character_width() + scale_width(18);
    return width;
  }

  auto get_component_maximum_width() {
    static auto width = 6 * get_character_width() + scale_width(18);
    return width;
  }

  void set_width_range(QWidget& widget) {
    widget.setMinimumWidth(get_component_minimum_width());
    widget.setMaximumWidth(get_component_maximum_width());
  }

  void set_color_value_range(LocalOptionalIntegerModel& model) {
    model.set_minimum(0);
    model.set_maximum(255);
  }

  void set_percent_value_range(LocalOptionalDecimalModel& model) {
    model.set_minimum(Decimal(0));
    model.set_maximum(Decimal(1));
  }

  Decimal round_value(const Decimal& value) {
    return round(100.0 * value) / 100.0;
  }

  auto get_hue(const QColor& color) {
    if(auto hue = color.hsvHueF(); hue >= 0.0) {
      return hue;
    }
    return 0.0;
  }

  int to_hue(const QColor& color) {
    return std::round(360 * get_hue(color));
  }

  Decimal to_saturation(const QColor& color) {
    return round_value(Decimal(color.hsvSaturationF()));
  }

  Decimal to_brightness(const QColor& color) {
    return round_value(Decimal(color.valueF()));
  }

  Decimal to_alpha(const QColor& color) {
    return round_value(Decimal(color.alphaF()));
  }

  void update_component_style(QWidget& component) {
    update_style(component, [] (auto& style) {
      style.get(Any()).set(TextAlign(Qt::AlignCenter));
      style.get(Any() > is_a<Button>()).set(Visibility::NONE);
    });
  }

  void initialize_color_component(QWidget& component) {
    set_width_range(component);
    update_component_style(component);
  }

  auto make_color_format_box(QWidget* parent) {
    auto list_model = std::make_shared<ArrayListModel<QString>>();
    list_model->push("HEX");
    list_model->push("RGB");
    list_model->push("HSB");
    auto color_format_box = new DropDownBox(std::move(list_model), parent);
    color_format_box->setMinimumWidth(
      6 * get_character_width() + scale_width(28));
    color_format_box->get_current()->set(0);
    return color_format_box;
  }

  auto make_rgb_color_box(std::shared_ptr<ValueModel<QColor>> color_model,
      std::shared_ptr<OptionalIntegerModel> red_model,
      std::shared_ptr<OptionalIntegerModel> green_model,
      std::shared_ptr<OptionalIntegerModel> blue_model) {
    auto red_box = new CustomIntegerBox(std::move(red_model));
    red_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    red_box->m_editing_finish_signal.connect([=] (const auto& value) {
      if(!value) {
        red_box->get_current()->set(color_model->get().red());
      }
    });
    initialize_color_component(*red_box);
    auto green_box = new CustomIntegerBox(std::move(green_model));
    green_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    green_box->m_editing_finish_signal.connect([=] (const auto& value) {
      if(!value) {
        green_box->get_current()->set(color_model->get().green());
      }
    });
    initialize_color_component(*green_box);
    auto blue_box = new CustomIntegerBox(std::move(blue_model));
    blue_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    blue_box->m_editing_finish_signal.connect([=] (const auto& value) {
      if(!value) {
        blue_box->get_current()->set(color_model->get().blue());
      }
    });
    initialize_color_component(*blue_box);
    auto rgb_color_box = new QWidget();
    rgb_color_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto layout = make_hbox_layout(rgb_color_box);
    layout->addWidget(red_box);
    layout->addSpacing(scale_width(4));
    layout->addWidget(green_box);
    layout->addSpacing(scale_width(4));
    layout->addWidget(blue_box);
    return rgb_color_box;
  }

  auto make_hsb_color_box(std::shared_ptr<ValueModel<QColor>> color_model,
      std::shared_ptr<OptionalIntegerModel> hue_model,
      std::shared_ptr<OptionalDecimalModel> saturation_model,
      std::shared_ptr<OptionalDecimalModel> brightness_model) {
    auto hue_box = new CustomIntegerBox(std::move(hue_model));
    hue_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hue_box->m_editing_finish_signal.connect([=] (const auto& value) {
      if(!value) {
        hue_box->get_current()->set(to_hue(color_model->get()));
      }
    });
    initialize_color_component(*hue_box);
    auto saturation_box = new CustomPercentBox(std::move(saturation_model));
    saturation_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    saturation_box->m_editing_finish_signal.connect([=] (const auto& value) {
      if(!value) {
        saturation_box->get_current()->set(to_saturation(color_model->get()));
      }
    });
    initialize_color_component(*saturation_box);
    auto brightness_box = new CustomPercentBox(std::move(brightness_model));
    brightness_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    brightness_box->m_editing_finish_signal.connect([=] (const auto& value) {
      if(!value) {
        brightness_box->get_current()->set(to_brightness(color_model->get()));
      }
    });
    initialize_color_component(*brightness_box);
    auto hsb_color_box = new QWidget();
    hsb_color_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto layout = make_hbox_layout(hsb_color_box);
    layout->addWidget(hue_box);
    layout->addSpacing(scale_width(4));
    layout->addWidget(saturation_box);
    layout->addSpacing(scale_width(4));
    layout->addWidget(brightness_box);
    return hsb_color_box;
  }

  auto make_alpha_box(std::shared_ptr<ValueModel<QColor>> color_model,
      std::shared_ptr<OptionalDecimalModel> alpha_model, QWidget* parent) {
    auto alpha_box = new CustomPercentBox(alpha_model, parent);
    alpha_box->m_editing_finish_signal.connect([=] (const auto& value) {
      if(!value) {
        alpha_box->get_current()->set(to_alpha(color_model->get()));
      }
    });
    update_component_style(*alpha_box);
    match(*alpha_box, Alpha());
    return alpha_box;
  }
}

struct ColorCodePanel::ColorCodeValueModel {
  std::shared_ptr<ValueModel<QColor>> m_source;
  std::shared_ptr<LocalValueModel<QColor>> m_hex_model;
  std::shared_ptr<LocalOptionalIntegerModel> m_red_model;
  std::shared_ptr<LocalOptionalIntegerModel> m_green_model;
  std::shared_ptr<LocalOptionalIntegerModel> m_blue_model;
  std::shared_ptr<LocalOptionalIntegerModel> m_hue_model;
  std::shared_ptr<LocalOptionalDecimalModel> m_saturation_model;
  std::shared_ptr<LocalOptionalDecimalModel> m_brightness_model;
  std::shared_ptr<LocalOptionalDecimalModel> m_alpha_model;
  scoped_connection m_source_connection;
  scoped_connection m_hex_connection;
  scoped_connection m_red_connection;
  scoped_connection m_green_connection;
  scoped_connection m_blue_connection;
  scoped_connection m_hue_connection;
  scoped_connection m_saturation_connection;
  scoped_connection m_brightness_connection;
  scoped_connection m_alpha_connection;

  explicit ColorCodeValueModel(std::shared_ptr<ValueModel<QColor>> source)
      : m_source(std::move(source)),
      m_hex_model(std::make_shared<LocalValueModel<QColor>>()),
      m_red_model(std::make_shared<LocalOptionalIntegerModel>()),
      m_green_model(std::make_shared<LocalOptionalIntegerModel>()),
      m_blue_model(std::make_shared<LocalOptionalIntegerModel>()),
      m_hue_model(std::make_shared<LocalOptionalIntegerModel>()),
      m_saturation_model(std::make_shared<LocalOptionalDecimalModel>()),
      m_brightness_model(std::make_shared<LocalOptionalDecimalModel>()),
      m_alpha_model(std::make_shared<LocalOptionalDecimalModel>()) {
    set_color_value_range(*m_red_model);
    set_color_value_range(*m_green_model);
    set_color_value_range(*m_blue_model);
    set_percent_value_range(*m_saturation_model);
    set_percent_value_range(*m_brightness_model);
    set_percent_value_range(*m_alpha_model);
    m_hue_model->set_minimum(0);
    m_hue_model->set_maximum(360);
    on_current(m_source->get());
    m_source_connection = m_source->connect_update_signal(
      std::bind_front(&ColorCodeValueModel::on_current, this));
    m_hex_connection = m_hex_model->connect_update_signal(
      std::bind_front(&ColorCodeValueModel::on_hex_current, this));
    m_red_connection = m_red_model->connect_update_signal(
      std::bind_front(&ColorCodeValueModel::on_red_current, this));
    m_green_connection = m_green_model->connect_update_signal(
      std::bind_front(&ColorCodeValueModel::on_green_current, this));
    m_blue_connection = m_blue_model->connect_update_signal(
      std::bind_front(&ColorCodeValueModel::on_blue_current, this));
    m_hue_connection = m_hue_model->connect_update_signal(
      std::bind_front(&ColorCodeValueModel::on_hue_current, this));
    m_saturation_connection = m_saturation_model->connect_update_signal(
      std::bind_front(&ColorCodeValueModel::on_saturation_current, this));
    m_brightness_connection = m_brightness_model->connect_update_signal(
      std::bind_front(&ColorCodeValueModel::on_brightness_current, this));
    m_alpha_connection = m_alpha_model->connect_update_signal(
      std::bind_front(&ColorCodeValueModel::on_alpha_current, this));
  }

  void on_current(const QColor& current) {
    update_hex(current);
    update_rgb(current);
    update_hsv(current);
    auto blocker = shared_connection_block(m_alpha_connection);
    m_alpha_model->set(to_alpha(current));
  }

  void on_hex_current(const QColor& value) {
    auto color = QColor(value.red(), value.green(), value.blue(),
      m_source->get().alpha());
    update_rgb(color);
    update_hsv(color);
    update_source(color);
  }

  void on_red_current(const optional<int>& value) {
    if(!value) {
      return;
    }
    auto color = m_source->get();
    color.setRed(*value);
    update_hex(color);
    update_hsv(color);
    update_source(color);
  }

  void on_green_current(const optional<int>& value) {
    if(!value) {
      return;
    }
    auto color = m_source->get();
    color.setGreen(*value);
    update_hex(color);
    update_hsv(color);
    update_source(color);
  }

  void on_blue_current(const optional<int>& value) {
    if(!value) {
      return;
    }
    auto color = m_source->get();
    color.setBlue(*value);
    update_hex(color);
    update_hsv(color);
    update_source(color);
  }

  void on_hue_current(const optional<int>& value) {
    if(!value) {
      return;
    }
    auto color = QColor::fromHsvF(*value / 360.0,
      m_source->get().hsvSaturationF(), m_source->get().valueF(),
      m_source->get().alphaF());
    update_hex(color);
    update_rgb(color);
    update_source(color);
  }

  void on_saturation_current(const optional<Decimal>& value) {
    if(!value) {
      return;
    }
    auto color = QColor::fromHsvF(get_hue(m_source->get()),
      static_cast<qreal>(*value), m_source->get().valueF(),
      m_source->get().alphaF());
    update_hex(color);
    update_rgb(color);
    update_source(color);
  }

  void on_brightness_current(const optional<Decimal>& value) {
    if(!value) {
      return;
    }
    auto color = QColor::fromHsvF(get_hue(m_source->get()),
      m_source->get().hsvSaturationF(), static_cast<qreal>(*value),
      m_source->get().alphaF());
    update_hex(color);
    update_rgb(color);
    update_source(color);
  }

  void on_alpha_current(const optional<Decimal>& value) {
    if(!value) {
      return;
    }
    auto color = m_source->get();
    color.setAlphaF(static_cast<qreal>(*value));
    update_source(color);
  }

  void update_source(const QColor& color) {
    auto blocker = shared_connection_block(m_source_connection);
    m_source->set(color);
  }

  void update_hex(const QColor& color) {
    auto blocker = shared_connection_block(m_hex_connection);
    m_hex_model->set(color);
  }

  void update_rgb(const QColor& color) {
    auto red_blocker = shared_connection_block(m_red_connection);
    m_red_model->set(color.red());
    auto green_blocker = shared_connection_block(m_green_connection);
    m_green_model->set(color.green());
    auto blue_blocker = shared_connection_block(m_blue_connection);
    m_blue_model->set(color.blue());
  }

  void update_hsv(const QColor& color) {
    auto hue_blocker = shared_connection_block(m_hue_connection);
    m_hue_model->set(to_hue(color));
    auto saturation_blocker = shared_connection_block(m_saturation_connection);
    m_saturation_model->set(to_saturation(color));
    auto brightness_blocker = shared_connection_block(m_brightness_connection);
    m_brightness_model->set(to_brightness(color));
  }
};

ColorCodePanel::ColorCodePanel(QWidget* parent)
  : ColorCodePanel(std::make_shared<LocalValueModel<QColor>>(), parent) {}

ColorCodePanel::ColorCodePanel(std::shared_ptr<ValueModel<QColor>> current,
    QWidget* parent)
    : QWidget(parent),
      m_current(std::make_shared<ColorCodeValueModel>(std::move(current))) {
  m_color_format_box = make_color_format_box(this);
  m_color_format_box->get_current()->connect_update_signal(
    std::bind_front(&ColorCodePanel::on_mode_current, this));
  m_color_input = new QStackedWidget(this);
  m_color_input->setMinimumWidth(
    3 * get_component_minimum_width() + scale_width(8));
  m_color_input->setMaximumWidth(
    3 * get_component_maximum_width() + scale_width(8));
  auto hex_color_box = new HexColorBox(m_current->m_hex_model);
  hex_color_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_color_input->addWidget(hex_color_box);
  m_color_input->addWidget(make_rgb_color_box(m_current->m_source,
    m_current->m_red_model, m_current->m_green_model, m_current->m_blue_model));
  m_color_input->addWidget(make_hsb_color_box(m_current->m_source,
    m_current->m_hue_model, m_current->m_saturation_model,
    m_current->m_brightness_model));
  m_alpha_box =
    make_alpha_box(m_current->m_source, m_current->m_alpha_model, this);
  m_alpha_box->installEventFilter(this);
}

const std::shared_ptr<ValueModel<QColor>>& ColorCodePanel::get_current() const {
  return m_current->m_source;
}

ColorCodePanel::Mode ColorCodePanel::get_mode() const {
  return static_cast<Mode>(*m_color_format_box->get_current()->get());
}

void ColorCodePanel::set_mode(Mode mode) {
  m_color_format_box->get_current()->set(static_cast<int>(mode));
}

QSize ColorCodePanel::sizeHint() const {
  if(m_size_hint) {
    return *m_size_hint;
  }
  auto color_format_width = std::max(m_color_format_box->sizeHint().width(),
    m_color_format_box->minimumWidth());
  auto color_input_width = std::clamp(m_color_input->sizeHint().width(),
    m_color_input->minimumWidth(), m_color_input->maximumWidth());
  auto alpha_width = [&] {
    if(m_alpha_box->isVisible()) {
      return (color_input_width - scale_width(8)) / 3;
    }
    return 0;
  }();
  m_size_hint.emplace(color_format_width + scale_width(8) + color_input_width +
    scale_width(4) + alpha_width, m_color_format_box->sizeHint().height());
  return *m_size_hint;
}

bool ColorCodePanel::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Show || event->type() == QEvent::Hide) {
    m_size_hint = none;
    update_layout();
    updateGeometry();
  }
  return QWidget::eventFilter(watched, event);
}

void ColorCodePanel::resizeEvent(QResizeEvent* event) {
  update_layout();
}

void ColorCodePanel::update_layout() {
  m_color_format_box->adjustSize();
  auto y = (height() - m_color_format_box->height()) / 2.0;
  auto height = m_color_format_box->height();
  m_color_format_box->move(0, y);
  auto color_input_width = [&] {
    if(m_alpha_box->isVisible()) {
      return (3 * (width() - m_color_format_box->width() - scale_width(12)) +
        scale_width(8)) / 4;
    }
    return width() - m_color_format_box->width() - scale_width(8);
  }();
  m_color_input->setGeometry(m_color_format_box->width() + scale_width(8), y,
    color_input_width, height);
  if(m_alpha_box->isVisible()) {
    m_alpha_box->setGeometry(m_color_input->geometry().right() + scale_width(4),
      y, (m_color_input->width() - scale_width(8)) / 3, height);
  }
}

void ColorCodePanel::on_mode_current(const optional<int>& current) {
  if(current) {
    m_color_input->setCurrentIndex(*current);
  } else {
    m_color_format_box->get_current()->set(0);
  }
}
