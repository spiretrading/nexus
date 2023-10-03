#include "Spire/Ui/ColorPicker.hpp"
#include <QEvent>
#include <QPainter>
#include "Spire/Spire/Decimal.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
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
  auto get_board_image() {
    static auto image = QImage(":/Icons/chequered-board.png");
    return image;
  }

  auto get_pure_color(const QColor& color) {
    return QColor::fromHsvF(color.hsvHueF(), 1.0, 1.0);
  }

  void update_color_spectrum_track(QWidget& color_spectrum,
      const QColor& pure_color) {
    auto saturation_gradient = QLinearGradient(0, 0, color_spectrum.width(), 0);
    saturation_gradient.setColorAt(0, QColor(0xFFFFFF));
    saturation_gradient.setColorAt(1, pure_color);
    auto brightness_gradient =
      QLinearGradient(0, 0, 0, color_spectrum.height());
    brightness_gradient.setColorAt(0, Qt::transparent);
    brightness_gradient.setColorAt(1, Qt::black);
    auto track_image =
      QImage(color_spectrum.size(), QImage::Format_ARGB32_Premultiplied);
    auto painter = QPainter(&track_image);
    painter.fillRect(QRect({0, 0}, color_spectrum.size()), saturation_gradient);
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.fillRect(QRect({0, 0}, color_spectrum.size()), brightness_gradient);
    update_style(color_spectrum, [&] (auto& style) {
      style.get(Any() > Track()).set(IconImage(track_image));
    });
  }

  void update_alpha_slider_track(QWidget& alpha_slider, const QColor& color) {
    auto alpha_gradient = QLinearGradient(0, 0, alpha_slider.width(), 0);
    alpha_gradient.setColorAt(0, Qt::transparent);
    alpha_gradient.setColorAt(1, color.rgb());
    auto alpha_image =
      QImage(alpha_slider.size(), QImage::Format_ARGB32_Premultiplied);
    auto alpha_painter = QPainter(&alpha_image);
    alpha_painter.fillRect(
      QRect({0, 0}, alpha_slider.size()), get_board_image());
    alpha_painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    alpha_painter.fillRect(QRect({0, 0}, alpha_slider.size()), alpha_gradient);
    update_style(alpha_slider, [&] (auto& style) {
      style.get(Any() > Track()).set(IconImage(alpha_image));
    });
  }

  auto make_color_spectrum(std::shared_ptr<DecimalModel> x_model,
      std::shared_ptr<DecimalModel> y_model, const QImage& thumb_image) {
    auto color_spectrum = new Slider2D(std::move(x_model), std::move(y_model));
    color_spectrum->setFixedHeight(scale_height(164));
    update_style(*color_spectrum, [&] (auto& style) {
      style.get(Any() > Thumb() > is_a<Icon>()).
        set(Fill(optional<QColor>())).
        set(IconImage(thumb_image));
      style.get(Focus() > Thumb() > is_a<Icon>()).
        set(Fill(QColor(0x808080)));
    });
    return color_spectrum;
  }

  auto make_hue_slider(std::shared_ptr<DecimalModel> model,
      const QImage& thumb_image) {
    auto hue_slider = new Slider(std::move(model));
    hue_slider->setFixedHeight(scale_height(16));
    auto hue_track_image = QImage(":/Icons/hue-spectrum.png");
    update_style(*hue_slider, [&] (auto& style) {
      style.get(Any()).set(border(scale_width(1), QColor(0xC8C8C8)));
      style.get(Any() > Track()).set(IconImage(hue_track_image));
      style.get(Any() > Thumb() > is_a<Icon>()).
        set(Fill(optional<QColor>())).
        set(IconImage(thumb_image));
      style.get(Focus() > Thumb() > is_a<Icon>()).
        set(Fill(QColor(0x808080)));
    });
    return hue_slider;
  }

  auto make_alpha_slider(std::shared_ptr<DecimalModel> model,
      const QImage& thumb_image) {
    auto alpha_slider = new Slider(std::move(model));
    alpha_slider->setFixedHeight(scale_height(16));
    match(*alpha_slider, Alpha());
    update_style(*alpha_slider, [&] (auto& style) {
      style.get(Any()).set(border(scale_width(1), QColor(0xC8C8C8)));
      style.get(Any() > Thumb() > is_a<Icon>()).
        set(Fill(boost::optional<QColor>())).
        set(IconImage(thumb_image));
      style.get(Focus() > Thumb() > is_a<Icon>()).
        set(Fill(QColor(0x808080)));
    });
    return alpha_slider;
  }
}

struct ColorPicker::ColorPickerModel {
  std::shared_ptr<ValueModel<QColor>> m_source;
  std::shared_ptr<LocalDecimalModel> m_hue_slider_model;
  std::shared_ptr<LocalDecimalModel> m_alpha_slider_model;
  std::shared_ptr<LocalDecimalModel> m_spectrum_x_model;
  std::shared_ptr<LocalDecimalModel> m_spectrum_y_model;
  scoped_connection m_hue_connection;
  scoped_connection m_alpha_connection;
  scoped_connection m_spectrum_x_connection;
  scoped_connection m_spectrum_y_connection;
  scoped_connection m_current_connection;

  ColorPickerModel(std::shared_ptr<ValueModel<QColor>> source)
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
    m_hue_connection = m_hue_slider_model->connect_update_signal(
      std::bind_front(&ColorPickerModel::on_hue_current, this));
    m_alpha_connection = m_alpha_slider_model->connect_update_signal(
      std::bind_front(&ColorPickerModel::on_alpha_current, this));
    m_spectrum_x_connection = m_spectrum_x_model->connect_update_signal(
      std::bind_front(&ColorPickerModel::on_color_spectrum_x_current, this));
    m_spectrum_y_connection = m_spectrum_y_model->connect_update_signal(
      std::bind_front(&ColorPickerModel::on_color_spectrum_y_current, this));
    m_current_connection = m_source->connect_update_signal(
      std::bind_front(&ColorPickerModel::on_current, this));
  }

  void on_hue_current(const Decimal& value) {
    auto& color = m_source->get();
    m_source->set(QColor::fromHsvF(static_cast<double>(value) / 360.0,
      color.hsvSaturationF(), color.valueF(), color.alphaF()));
  }

  void on_alpha_current(const Decimal& value) {
    auto color = m_source->get();
    color.setAlphaF(static_cast<qreal>(value / 100.0));
    auto blocker = shared_connection_block(m_current_connection);
    m_source->set(color);
  }

  void on_color_spectrum_x_current(const Decimal& value) {
    auto& color = m_source->get();
    auto blocker = shared_connection_block(m_current_connection);
    m_source->set(QColor::fromHsvF(color.hsvHueF(),
      static_cast<qreal>(value / 100.0), color.valueF(), color.alphaF()));
  }

  void on_color_spectrum_y_current(const Decimal& value) {
    auto& color = m_source->get();
    auto blocker = shared_connection_block(m_current_connection);
    m_source->set(QColor::fromHsvF(color.hsvHueF(), color.hsvSaturationF(),
      static_cast<qreal>(value / 100.0), color.alphaF()));
  }

  void on_current(const QColor& current) {
    auto hue_blocker = shared_connection_block(m_hue_connection);
    m_hue_slider_model->set(360 * current.hsvHueF());
    auto alpha_blocker = shared_connection_block(m_alpha_connection);
    m_alpha_slider_model->set(100 * current.alphaF());
    auto spectrum_x_blocker = shared_connection_block(m_spectrum_x_connection);
    m_spectrum_x_model->set(100 * current.hsvSaturationF());
    auto spectrum_y_blocker = shared_connection_block(m_spectrum_y_connection);
    m_spectrum_y_model->set(100 * current.valueF());
  }
};

ColorPicker::ColorPicker(QWidget& parent)
  : ColorPicker(std::make_shared<LocalValueModel<QColor>>(), parent) {}

ColorPicker::ColorPicker(std::shared_ptr<ValueModel<QColor>> current,
  QWidget& parent)
  : ColorPicker(std::move(current), std::make_shared<ArrayListModel<QColor>>(),
      parent) {}

ColorPicker::ColorPicker(std::shared_ptr<ValueModel<QColor>> current,
    std::shared_ptr<ListModel<QColor>> palette, QWidget& parent)
    : QWidget(&parent),
      m_model(std::make_shared<ColorPickerModel>(std::move(current))),
      m_palette(std::move(palette)),
      m_panel_horizontal_spacing(0) {
  auto thumb_image =
    imageFromSvg(":/Icons/color-thumb.svg", scale(14, 14));
  m_color_spectrum = make_color_spectrum(m_model->m_spectrum_x_model,
    m_model->m_spectrum_y_model, thumb_image);
  update_color_spectrum_track(*m_color_spectrum,
    get_pure_color(get_current()->get()));
  auto hue_slider = make_hue_slider(m_model->m_hue_slider_model, thumb_image);
  m_alpha_slider =
    make_alpha_slider(m_model->m_alpha_slider_model, thumb_image);
  update_alpha_slider_track(*m_alpha_slider, m_model->m_source->get());
  m_color_code_panel = new ColorCodePanel(get_current());
  auto layout = make_vbox_layout(this);
  layout->setSpacing(scale_height(8));
  layout->addWidget(m_color_spectrum);
  layout->addWidget(hue_slider);
  layout->addWidget(m_alpha_slider);
  layout->addSpacing(scale_height(10));
  layout->addWidget(m_color_code_panel);
  m_panel = new OverlayPanel(*this, parent);
  m_panel->setWindowFlags(Qt::Popup | (m_panel->windowFlags() & ~Qt::Tool));
  m_panel->installEventFilter(this);
  m_panel_style_connection = connect_style_signal(*m_panel,
    std::bind_front(&ColorPicker::on_panel_style, this));
  update_style(*m_panel, [&] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(18)));
  });
  on_current(get_current()->get());
  m_current_connection = get_current()->connect_update_signal(
    std::bind_front(&ColorPicker::on_current, this));
}

const std::shared_ptr<ValueModel<QColor>>& ColorPicker::get_current() const {
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
    m_panel->setFixedWidth(12 * scale_width(22) + m_panel_horizontal_spacing);
  } else if(event->type() == QEvent::HideToParent) {
    m_panel->hide();
  }
  return QWidget::event(event);
}

void ColorPicker::on_current(const QColor& current) {
  if(auto pure_color = get_pure_color(current);
      get_pure_color(m_last_color) != pure_color) {
    update_color_spectrum_track(*m_color_spectrum, pure_color);
  }
  if(m_alpha_slider->isVisible() && m_last_color.rgb() != current.rgb()) {
    update_alpha_slider_track(*m_alpha_slider, current);
  }
  m_last_color = current;
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
