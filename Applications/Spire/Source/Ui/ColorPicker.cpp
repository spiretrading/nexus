#include "Spire/Ui/ColorPicker.hpp"
#include <QEvent>
#include <QImage>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Ui/ColorCodePanel.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/Slider.hpp"
#include "Spire/Ui/Slider2D.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

//struct ColorPicker::ColorPickerModel {
//  std::shared_ptr<ValueModel<QColor>> m_source;
//  std::shared_ptr<LocalValueModel<QColor>> m_hex_model;
//  std::shared_ptr<LocalOptionalIntegerModel> m_red_model;
//  std::shared_ptr<LocalOptionalIntegerModel> m_green_model;
//  std::shared_ptr<LocalOptionalIntegerModel> m_blue_model;
//  std::shared_ptr<LocalOptionalIntegerModel> m_hue_model;
//  std::shared_ptr<LocalOptionalDecimalModel> m_saturation_model;
//  std::shared_ptr<LocalOptionalDecimalModel> m_brightness_model;
//  std::shared_ptr<LocalOptionalDecimalModel> m_alpha_model;
//  scoped_connection m_connection;
//
//  ColorPickerModel(std::shared_ptr<ValueModel<QColor>> source)
//    : m_source(std::move(source)),
//      m_hex_model(std::make_shared<LocalValueModel<QColor>>()),
//      m_red_model(std::make_shared<LocalOptionalIntegerModel>()),
//      m_green_model(std::make_shared<LocalOptionalIntegerModel>()),
//      m_blue_model(std::make_shared<LocalOptionalIntegerModel>()),
//      m_hue_model(std::make_shared<LocalOptionalIntegerModel>()),
//      m_saturation_model(std::make_shared<LocalOptionalDecimalModel>()),
//      m_brightness_model(std::make_shared<LocalOptionalDecimalModel>()),
//      m_alpha_model(std::make_shared<LocalOptionalDecimalModel>()),
//      m_connection(m_source->connect_update_signal(
//        std::bind_front(&ColorCodeValueModel::on_current, this))) {
//    set_color_value_range(*m_red_model);
//    set_color_value_range(*m_green_model);
//    set_color_value_range(*m_blue_model);
//    set_percent_value_range(*m_saturation_model);
//    set_percent_value_range(*m_brightness_model);
//    set_percent_value_range(*m_alpha_model);
//    m_hue_model->set_minimum(0);
//    m_hue_model->set_maximum(360);
//    on_current(m_source->get());
//    m_hex_model->connect_update_signal(
//      std::bind_front(&ColorCodeValueModel::on_hex_current, this));
//    m_red_model->connect_update_signal(
//      std::bind_front(&ColorCodeValueModel::on_red_current, this));
//    m_green_model->connect_update_signal(
//      std::bind_front(&ColorCodeValueModel::on_green_current, this));
//    m_blue_model->connect_update_signal(
//      std::bind_front(&ColorCodeValueModel::on_blue_current, this));
//    m_hue_model->connect_update_signal(
//      std::bind_front(&ColorCodeValueModel::on_hue_current, this));
//    m_saturation_model->connect_update_signal(
//      std::bind_front(&ColorCodeValueModel::on_saturation_current, this));
//    m_brightness_model->connect_update_signal(
//      std::bind_front(&ColorCodeValueModel::on_brightness_current, this));
//    m_alpha_model->connect_update_signal(
//      std::bind_front(&ColorCodeValueModel::on_alpha_current, this));
//  }
//
//  void on_current(const QColor& current) {
//    m_hex_model->set(current);
//    m_red_model->set(current.red());
//    m_green_model->set(current.green());
//    m_blue_model->set(current.blue());
//    m_hue_model->set(to_hue(current.hslHueF()));
//    m_saturation_model->set(round_value(Decimal(current.hsvSaturationF())));
//    m_brightness_model->set(round_value(Decimal(current.valueF())));
//    m_alpha_model->set(round_value(Decimal(current.alphaF())));
//  }
//
//  void on_hex_current(const QColor& value) {
//    if(value.rgb() != m_source->get().rgb()) {
//      m_source->set(value);
//    }
//  }
//
//  void on_red_current(const optional<int>& value) {
//    if(value && *value != m_source->get().red()) {
//      auto color = m_source->get();
//      color.setRed(*value);
//      m_source->set(color);
//    }
//  }
//
//  void on_green_current(const optional<int>& value) {
//    if(value && *value != m_source->get().green()) {
//      auto color = m_source->get();
//      color.setGreen(*value);
//      m_source->set(color);
//    }
//  }
//
//  void on_blue_current(const optional<int>& value) {
//    if(value && *value != m_source->get().blue()) {
//      auto color = m_source->get();
//      color.setBlue(*value);
//      m_source->set(color);
//    }
//  }
//
//  void on_hue_current(const optional<int>& value) {
//    if(value && *value != to_hue(m_source->get().hslHueF())) {
//      auto color = m_source->get();
//      color.setHsvF(*value / 360.0, color.hsvSaturationF(), color.valueF(),
//        color.alphaF());
//      m_source->set(color);
//    }
//  }
//
//  void on_saturation_current(const optional<Decimal>& value) {
//    if(value &&
//        *value != round_value(Decimal(m_source->get().hsvSaturationF()))) {
//      auto color = m_source->get();
//      color.setHsvF(color.hsvHueF(), static_cast<qreal>(*value), color.valueF(),
//        color.alphaF());
//      m_source->set(color);
//    }
//  }
//
//  void on_brightness_current(const optional<Decimal>& value) {
//    if(value && *value != round_value(Decimal(m_source->get().valueF()))) {
//      auto color = m_source->get();
//      color.setHsvF(color.hsvHueF(), color.hsvSaturationF(),
//        static_cast<qreal>(*value), color.alphaF());
//      m_source->set(color);
//    }
//  }
//
//  void on_alpha_current(const optional<Decimal>& value) {
//    if(value && *value != round_value(Decimal(m_source->get().alphaF()))) {
//      auto color = m_source->get();
//      color.setAlphaF(static_cast<qreal>(*value));
//      m_source->set(color);
//    }
//  }
//};

ColorPicker::ColorPicker(QWidget& parent)
  : ColorPicker(std::make_shared<LocalValueModel<QColor>>(), parent) {}

ColorPicker::ColorPicker(std::shared_ptr<ValueModel<QColor>> current,
    QWidget& parent)
    : ColorPicker(std::move(current), std::make_shared<ArrayListModel<QColor>>(), parent) {
}

ColorPicker::ColorPicker(std::shared_ptr<ValueModel<QColor>> current,
    std::shared_ptr<ListModel<QColor>> palette, QWidget& parent)
    : QWidget(&parent),
      m_current(std::move(current)),
      m_palette(std::move(palette)) { 
  auto thumb_image =
    imageFromSvg(":/Icons/color-thumb.svg", scale(14, 14));
  auto spectrum_x_model = std::make_shared<LocalDecimalModel>();
  spectrum_x_model->set_minimum(Decimal(0));
  spectrum_x_model->set_maximum(Decimal(100));
  auto spectrum_y_model = std::make_shared<LocalDecimalModel>();
  spectrum_y_model->set_minimum(Decimal(0));
  spectrum_y_model->set_maximum(Decimal(100));
  m_color_spectrum = new Slider2D(std::move(spectrum_x_model), std::move(spectrum_y_model));
  m_color_spectrum->setFixedHeight(scale_height(164));
  update_style(*m_color_spectrum, [&] (auto& style) {
    style.get(Any() > Thumb() > is_a<Icon>()).
      set(Fill(boost::optional<QColor>())).
      set(IconImage(thumb_image));
    style.get(Focus() > Thumb() > is_a<Icon>()).
      set(Fill(QColor(0x808080)));
  });
  auto hue_model = std::make_shared<LocalDecimalModel>();
  hue_model->set_minimum(Decimal(0));
  hue_model->set_maximum(Decimal(360));
  m_hue_slider = new Slider(std::move(hue_model));
  m_hue_slider->setFixedHeight(scale_height(16));
  auto hue_track_image = QImage(":/Icons/hue-spectrum.png");
  update_style(*m_hue_slider, [&] (auto& style) {
    style.get(Any()).set(border(scale_width(1), QColor(0xC8C8C8)));
    style.get(Any() > Track()).set(IconImage(hue_track_image));
    style.get(Any() > Thumb() > is_a<Icon>()).
      set(Fill(boost::optional<QColor>())).
      set(IconImage(thumb_image));
    style.get(Focus() > Thumb() > is_a<Icon>()).
      set(Fill(QColor(0x808080)));
  });
  auto alpha_model = std::make_shared<LocalDecimalModel>();
  alpha_model->set_minimum(Decimal(0));
  alpha_model->set_maximum(Decimal(100));
  m_alpha_slider = new Slider(std::move(alpha_model));
  m_alpha_slider->setFixedHeight(scale_height(16));
  update_style(*m_alpha_slider, [&] (auto& style) {
    style.get(Any()).set(border(scale_width(1), QColor(0xC8C8C8)));
    style.get(Any() > Thumb() > is_a<Icon>()).
      set(Fill(boost::optional<QColor>())).
      set(IconImage(thumb_image));
    style.get(Focus() > Thumb() > is_a<Icon>()).
      set(Fill(QColor(0x808080)));
  });
  m_color_code_panel = new ColorCodePanel(m_current);
  //color_code_panel->get_current()->connect_update_signal(std::bind(&ColorPicker::on_color_code_current, this));
  auto layout = make_vbox_layout(this);
  layout->addWidget(m_color_spectrum);
  layout->addSpacing(scale_height(8));
  layout->addWidget(m_hue_slider);
  layout->addSpacing(scale_height(8));
  layout->addWidget(m_alpha_slider);
  layout->addSpacing(scale_height(18));
  layout->addWidget(m_color_code_panel);
  m_panel = new OverlayPanel(*this, parent);
  m_panel->installEventFilter(this);
  update_style(*m_panel, [&] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(18)));
    //style.get(Any() > Alpha()).set(Visibility::NONE);
  });
  on_current(m_current->get());
  //setFixedWidth(color_code_panel->sizeHint().width());
  //m_panel->setFixedWidth(color_code_panel->sizeHint().width() + 2 * scale_width(9));
  //m_panel->set_closed_on_focus_out(true);
  m_hue_connection = m_hue_slider->get_current()->connect_update_signal(
    std::bind_front(&ColorPicker::on_hue_current, this));
  m_alpha_connection = m_alpha_slider->get_current()->connect_update_signal(
    std::bind_front(&ColorPicker::on_alpha_current, this));
  m_spectrum_x_connection = m_color_spectrum->get_x_current()->connect_update_signal(
    std::bind_front(&ColorPicker::on_color_spectrum_x_current, this));
  m_spectrum_y_connection = m_color_spectrum->get_y_current()->connect_update_signal(
    std::bind_front(&ColorPicker::on_color_spectrum_y_current, this));
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&ColorPicker::on_current, this));
}

const std::shared_ptr<ValueModel<QColor>>& ColorPicker::get_current() const {
  return m_current;
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
    auto m = m_color_code_panel->sizeHint().grownBy(margins);
    m_panel->setFixedWidth(m.width() + 2 * scale_width(9));
  } else if(event->type() == QEvent::HideToParent) {
    m_panel->hide();
  }
  return QWidget::event(event);
}

//void ColorPicker::on_color_code_current(const QColor& current) {
//}

void ColorPicker::on_hue_current(const Decimal& value) {
  auto color = m_current->get();
  m_current->set(QColor::fromHsvF(static_cast<double>(value) / 360.0, color.hsvSaturationF(), color.valueF(), color.alphaF()));
}

void ColorPicker::on_alpha_current(const Decimal& value) {
  auto color = m_current->get();
  color.setAlphaF(static_cast<qreal>(value / 100.0));
  auto blocker = shared_connection_block(m_current_connection);
  m_current->set(color);
}

void ColorPicker::on_color_spectrum_x_current(const Decimal& value) {
  auto color = m_current->get();
  m_current->set(QColor::fromHsvF(color.hsvHueF(), static_cast<qreal>(value / 100.0), color.valueF(), color.alphaF()));
}

void ColorPicker::on_color_spectrum_y_current(const Decimal& value) {
  auto color = m_current->get();
  m_current->set(QColor::fromHsvF(color.hsvHueF(), color.hsvSaturationF(), static_cast<qreal>(value / 100.0), color.alphaF()));
}

void ColorPicker::on_current(const QColor& current) {
  if(auto pure_color = QColor::fromHsvF(current.hsvHueF(), 1.0, 1.0); m_pure_color != pure_color) {
    m_pure_color = QColor::fromHsvF(current.hsvHueF(), 1.0, 1.0);
    auto brightness_gradient = QLinearGradient(0, 0, 0, m_color_spectrum->height());
    brightness_gradient.setColorAt(0, Qt::transparent);
    brightness_gradient.setColorAt(1, Qt::black);
    auto saturation_gradient = QLinearGradient(0, 0, m_color_spectrum->height(), 0);
    saturation_gradient.setColorAt(0, QColor(0xFFFFFF));
    saturation_gradient.setColorAt(1, m_pure_color);
    auto track_image = QImage(m_color_spectrum->size(), QImage::Format_ARGB32_Premultiplied);
    auto painter = QPainter(&track_image);
    painter.fillRect(QRect({0, 0}, m_color_spectrum->size()), saturation_gradient);
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.fillRect(QRect({0, 0}, m_color_spectrum->size()), brightness_gradient);
    update_style(*m_color_spectrum, [&] (auto& style) {
      style.get(Any() > Track()).set(IconImage(track_image));
    });
  }
  auto checker_board_image = QImage(":/Icons/chequered-board.png");
  auto alpha_gradient = QLinearGradient(0, 0, m_alpha_slider->width(), 0);
  alpha_gradient.setColorAt(0, Qt::transparent);
  alpha_gradient.setColorAt(1, current);
  auto alpha_image = QImage(m_alpha_slider->size(), QImage::Format_ARGB32_Premultiplied);
  auto alpha_painter = QPainter(&alpha_image);
  alpha_painter.fillRect(QRect({0, 0}, m_alpha_slider->size()), checker_board_image);
  alpha_painter.setCompositionMode(QPainter::CompositionMode_Multiply);
  alpha_painter.fillRect(QRect({0, 0}, m_alpha_slider->size()), alpha_gradient);
  update_style(*m_alpha_slider, [&] (auto& style) {
    style.get(Any() > Track()).set(IconImage(alpha_image));
  });
  auto hue_blocker = shared_connection_block(m_hue_connection);
  m_hue_slider->get_current()->set(current.hsvHueF() * 360);
  auto alpha_blocker = shared_connection_block(m_alpha_connection);
  m_alpha_slider->get_current()->set(current.alphaF() * 100);
  auto spectrum_x_blocker = shared_connection_block(m_spectrum_x_connection);
  m_color_spectrum->get_x_current()->set(current.hsvSaturationF() * 100);
  auto spectrum_y_blocker = shared_connection_block(m_spectrum_y_connection);
  m_color_spectrum->get_y_current()->set(current.valueF() * 100);
}
