#include "Spire/Ui/ColorPicker.hpp"
#include <QEvent>
#include <QPainter>
#include <QPixmap>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Ui/ColorCodePanel.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/Slider2D.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto& CHEQUERED_BOARD_IMAGE() {
    static auto image = QPixmap(":/Icons/chequered-board.png");
    return image;
  }

  const auto& HUE_SPECTRUM_IMAGE() {
    static auto image = QImage(":/Icons/hue-spectrum.png");
    return image;
  }

  const auto& THUMB_ICON() {
    static auto icon = imageFromSvg(":/Icons/color-thumb.svg", scale(14, 14));
    return icon;
  }

  const auto& THUMB_INVERT_ICON() {
    static auto icon =
      imageFromSvg(":/Icons/color-thumb-invert.svg", scale(14, 14));
    return icon;
  }

  auto get_hue(const QColor& color) {
    if(auto hue = color.hsvHueF(); hue >= 0) {
      return hue;
    }
    return 0.0;
  }

  auto get_pure_color(const QColor& color) {
    return QColor::fromHsvF(get_hue(color), 1.0, 1.0);
  }

  auto make_modifiers() {
    return QHash<Qt::KeyboardModifier, Decimal>(
      {{Qt::NoModifier, 1}, {Qt::ShiftModifier, 10}});
  }

  void update_color_spectrum_track(Slider2D& color_spectrum,
      const QColor& pure_color) {
    auto track_size = color_spectrum.size().shrunkBy(
      QMargins{scale_width(1), scale_width(1), scale_width(1), scale_width(1)});
    auto saturation_gradient = QLinearGradient(0, 0, track_size.width(), 0);
    saturation_gradient.setColorAt(0, QColor(0xFFFFFF));
    saturation_gradient.setColorAt(1, pure_color);
    auto brightness_gradient = QLinearGradient(0, 0, 0, track_size.height());
    brightness_gradient.setColorAt(0, Qt::transparent);
    brightness_gradient.setColorAt(1, Qt::black);
    auto track_image = QImage(track_size, QImage::Format_ARGB32_Premultiplied);
    auto painter = QPainter(&track_image);
    auto track_area = QRect(QPoint(0, 0), track_size);
    painter.fillRect(track_area, saturation_gradient);
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.fillRect(track_area, brightness_gradient);
    update_style(color_spectrum, [&] (auto& style) {
      style.get(Any() > Track()).set(IconImage(track_image));
    });
  }

  void update_alpha_slider_track(Slider& alpha_slider, const QColor& color) {
    auto track_size = alpha_slider.size().shrunkBy(
      QMargins{scale_width(1), scale_width(1), scale_width(1), scale_width(1)});
    auto alpha_gradient = QLinearGradient(0, 0, track_size.width(), 0);
    alpha_gradient.setColorAt(0, Qt::transparent);
    alpha_gradient.setColorAt(1, color.rgb());
    auto alpha_image = QImage(track_size, QImage::Format_ARGB32_Premultiplied);
    auto alpha_painter = QPainter(&alpha_image);
    auto track_area = QRect{QPoint(0, 0), track_size};
    auto board_image = CHEQUERED_BOARD_IMAGE();
    alpha_painter.drawTiledPixmap(track_area,
      board_image.scaled(QSize(board_image.width(), track_size.height())));
    alpha_painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    alpha_painter.fillRect(track_area, alpha_gradient);
    update_style(alpha_slider, [&] (auto& style) {
      style.get(Any() > Track()).set(IconImage(alpha_image));
    });
  }

  auto make_color_spectrum(std::shared_ptr<DecimalModel> x_model,
      std::shared_ptr<DecimalModel> y_model) {
    auto color_spectrum = new Slider2D(std::move(x_model), std::move(y_model),
      make_modifiers(), make_modifiers());
    color_spectrum->setFixedHeight(scale_height(164));
    color_spectrum->findChild<QLabel*>()->setScaledContents(false);
    update_style(*color_spectrum, [&] (auto& style) {
      style.get(Any()).set(border(scale_width(1), QColor(0xC8C8C8)));
      style.get(Any() > Thumb() > is_a<Icon>()).
        set(Fill(optional<QColor>())).
        set(IconImage(THUMB_ICON()));
      style.get(FocusVisible() > Thumb() > is_a<Icon>()).
        set(IconImage(THUMB_INVERT_ICON()));
    });
    return color_spectrum;
  }

  auto make_hue_slider(std::shared_ptr<DecimalModel> model) {
    auto hue_slider = new Slider(std::move(model), make_modifiers());
    hue_slider->setFixedHeight(scale_height(16));
    update_style(*hue_slider, [&] (auto& style) {
      style.get(Any()).set(border(scale_width(1), QColor(0xC8C8C8)));
      style.get(Any() > Track()).set(IconImage(HUE_SPECTRUM_IMAGE()));
      style.get(Any() > Thumb() > is_a<Icon>()).
        set(Fill(optional<QColor>())).
        set(IconImage(THUMB_ICON()));
      style.get(FocusVisible() > Thumb() > is_a<Icon>()).
        set(IconImage(THUMB_INVERT_ICON()));
    });
    return hue_slider;
  }

  auto make_alpha_slider(std::shared_ptr<DecimalModel> model) {
    auto alpha_slider = new Slider(std::move(model), make_modifiers());
    alpha_slider->setFixedHeight(scale_height(16));
    alpha_slider->findChild<QLabel*>()->setScaledContents(false);
    match(*alpha_slider, Alpha());
    update_style(*alpha_slider, [&] (auto& style) {
      style.get(Any()).set(border(scale_width(1), QColor(0xC8C8C8)));
      style.get(Any() > Thumb() > is_a<Icon>()).
        set(Fill(boost::optional<QColor>())).
        set(IconImage(THUMB_ICON()));
      style.get(FocusVisible() > Thumb() > is_a<Icon>()).
        set(IconImage(THUMB_INVERT_ICON()));
    });
    return alpha_slider;
  }
}

struct ColorPicker::ColorPickerModel {
  std::shared_ptr<ColorModel> m_source;
  std::shared_ptr<LocalDecimalModel> m_hue_slider_model;
  std::shared_ptr<LocalDecimalModel> m_alpha_slider_model;
  std::shared_ptr<LocalDecimalModel> m_spectrum_x_model;
  std::shared_ptr<LocalDecimalModel> m_spectrum_y_model;
  scoped_connection m_source_connection;
  scoped_connection m_hue_connection;
  scoped_connection m_alpha_connection;
  scoped_connection m_spectrum_x_connection;
  scoped_connection m_spectrum_y_connection;

  ColorPickerModel(std::shared_ptr<ColorModel> source)
      : m_source(std::move(source)),
        m_hue_slider_model(std::make_shared<LocalDecimalModel>()),
        m_alpha_slider_model(std::make_shared<LocalDecimalModel>()),
        m_spectrum_x_model(std::make_shared<LocalDecimalModel>()),
        m_spectrum_y_model(std::make_shared<LocalDecimalModel>()) {
    m_hue_slider_model->set_minimum(Decimal(0));
    m_hue_slider_model->set_maximum(Decimal(360));
    m_alpha_slider_model->set_minimum(Decimal(0));
    m_alpha_slider_model->set_maximum(Decimal(100));
    m_spectrum_x_model->set_minimum(Decimal(0));
    m_spectrum_x_model->set_maximum(Decimal(100));
    m_spectrum_y_model->set_minimum(Decimal(0));
    m_spectrum_y_model->set_maximum(Decimal(100));
    on_current(m_source->get());
    m_source_connection = m_source->connect_update_signal(
      std::bind_front(&ColorPickerModel::on_current, this));
    m_hue_connection = m_hue_slider_model->connect_update_signal(
      std::bind_front(&ColorPickerModel::on_hue_current, this));
    m_alpha_connection = m_alpha_slider_model->connect_update_signal(
      std::bind_front(&ColorPickerModel::on_alpha_current, this));
    m_spectrum_x_connection = m_spectrum_x_model->connect_update_signal(
      std::bind_front(&ColorPickerModel::on_color_spectrum_x_current, this));
    m_spectrum_y_connection = m_spectrum_y_model->connect_update_signal(
      std::bind_front(&ColorPickerModel::on_color_spectrum_y_current, this));
  }

  void on_hue_current(const Decimal& value) {
    auto& color = m_source->get();
    auto blocker = shared_connection_block(m_source_connection);
    m_source->set(QColor::fromHsvF(static_cast<double>(value) / 360.0,
      color.hsvSaturationF(), color.valueF(), color.alphaF()));
  }

  void on_alpha_current(const Decimal& value) {
    auto color = m_source->get();
    color.setAlphaF(static_cast<qreal>(value / 100.0));
    auto blocker = shared_connection_block(m_source_connection);
    m_source->set(color);
  }

  void on_color_spectrum_x_current(const Decimal& value) {
    auto& color = m_source->get();
    auto blocker = shared_connection_block(m_source_connection);
    m_source->set(QColor::fromHsvF(get_hue(color),
      static_cast<qreal>(value / 100.0), color.valueF(), color.alphaF()));
  }

  void on_color_spectrum_y_current(const Decimal& value) {
    auto& color = m_source->get();
    auto blocker = shared_connection_block(m_source_connection);
    m_source->set(QColor::fromHsvF(get_hue(color), color.hsvSaturationF(),
      static_cast<qreal>(value / 100.0), color.alphaF()));
  }

  void on_current(const QColor& current) {
    auto hue_blocker = shared_connection_block(m_hue_connection);
    m_hue_slider_model->set(360 * get_hue(current));
    auto alpha_blocker = shared_connection_block(m_alpha_connection);
    m_alpha_slider_model->set(100 * current.alphaF());
    auto spectrum_x_blocker = shared_connection_block(m_spectrum_x_connection);
    m_spectrum_x_model->set(100 * current.hsvSaturationF());
    auto spectrum_y_blocker = shared_connection_block(m_spectrum_y_connection);
    m_spectrum_y_model->set(100 * current.valueF());
  }
};

ColorPicker::ColorPicker(QWidget& parent)
  : ColorPicker(std::make_shared<LocalColorModel>(), parent) {}

ColorPicker::ColorPicker(std::shared_ptr<ColorModel> current,
  QWidget& parent)
  : ColorPicker(std::move(current), std::make_shared<ArrayListModel<QColor>>(),
      parent) {}

ColorPicker::ColorPicker(std::shared_ptr<ColorModel> current,
    std::shared_ptr<ListModel<QColor>> palette, QWidget& parent)
    : QWidget(&parent),
      m_model(std::make_shared<ColorPickerModel>(std::move(current))),
      m_palette(std::move(palette)),
      m_is_alpha_visible(true),
      m_panel_horizontal_spacing(0) {
  m_color_spectrum = make_color_spectrum(m_model->m_spectrum_x_model,
    m_model->m_spectrum_y_model);
  m_alpha_slider = make_alpha_slider(m_model->m_alpha_slider_model);
  auto layout = make_vbox_layout(this);
  layout->setSpacing(scale_height(8));
  layout->addWidget(m_color_spectrum);
  layout->addWidget(make_hue_slider(m_model->m_hue_slider_model));
  layout->addWidget(m_alpha_slider);
  layout->addSpacing(scale_height(10));
  layout->addWidget(new ColorCodePanel(get_current()));
  m_panel = new OverlayPanel(*this, parent);
  m_panel->setWindowFlags(Qt::Popup | (m_panel->windowFlags() & ~Qt::Tool));
  m_panel->installEventFilter(this);
  m_panel_style_connection = connect_style_signal(*m_panel,
    std::bind_front(&ColorPicker::on_panel_style, this));
  update_style(*m_panel, [] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(18)));
  });
  on_current(get_current()->get());
  m_current_connection = get_current()->connect_update_signal(
    std::bind_front(&ColorPicker::on_current, this));
  m_style_connection = connect_style_signal(*this,
    std::bind_front(&ColorPicker::on_style, this));
}

const std::shared_ptr<ColorModel>& ColorPicker::get_current() const {
  return m_model->m_source;
}

const std::shared_ptr<ListModel<QColor>>& ColorPicker::get_palette() const {
  return m_palette;
}

bool ColorPicker::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Close) {
    m_panel->hide();
    hide();
  }
  return QWidget::eventFilter(watched, event);
}

bool ColorPicker::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_panel->show();
    auto margins = m_panel->layout()->contentsMargins();
    m_panel->setFixedWidth(m_panel->get_body().width() + margins.left() +
      margins.right() + m_panel_horizontal_spacing);
  } else if(event->type() == QEvent::HideToParent) {
    m_panel->hide();
  }
  return QWidget::event(event);
}

void ColorPicker::resizeEvent(QResizeEvent* event) {
  auto& color = get_current()->get();
  update_color_spectrum_track(*m_color_spectrum, get_pure_color(color));
  update_alpha_slider_track(*m_alpha_slider, color);
  return QWidget::resizeEvent(event);
}

void ColorPicker::on_current(const QColor& current) {
  if(auto pure_color = get_pure_color(current);
      get_pure_color(m_last_color) != pure_color) {
    update_color_spectrum_track(*m_color_spectrum, pure_color);
  }
  if(m_is_alpha_visible && m_last_color.rgb() != current.rgb()) {
    update_alpha_slider_track(*m_alpha_slider, current);
  }
  m_last_color = current;
}

void ColorPicker::on_style() {
  auto& stylist = find_stylist(*this);
  if(auto visibility = Styles::find<Visibility>(stylist.get_computed_block())) {
    stylist.evaluate(*visibility, [=] (auto visibility) {
      if(visibility == Visibility::VISIBLE) {
        m_is_alpha_visible = true;
      } else {
        m_is_alpha_visible = false;
      }
    });
  }
}

void ColorPicker::on_panel_style() {
  m_panel_horizontal_spacing = 0;
  auto& stylist = find_stylist(*m_panel);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const BorderRightSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_panel_horizontal_spacing += size;
        });
      },
      [&] (const BorderLeftSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_panel_horizontal_spacing += size;
        });
      },
      [&] (const PaddingRight& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_panel_horizontal_spacing += size;
        });
      },
      [&] (const PaddingLeft& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_panel_horizontal_spacing += size;
        });
      });
  }
}
