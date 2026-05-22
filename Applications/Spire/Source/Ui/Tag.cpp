#include "Spire/Ui/Tag.hpp"
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto& get_delete_icon() {
    static auto icon = image_from_svg(":/Icons/delete.svg", scale(16, 16));
    return icon;
  }

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(12));
    style.get(Any()).
      set(BackgroundColor(QColor(0xEBEBEB))).
      set(border_radius(scale_width(3))).
      set(Font(font)).
      set(TextColor(QColor(Qt::black))).
      set(horizontal_padding(scale_width(5))).
      set(vertical_padding(scale_height(2)));
    return style;
  }
}

Tag::Tag(QString label, QWidget* parent)
  : Tag(std::make_shared<LocalTextModel>(std::move(label)), parent) {}

Tag::Tag(std::shared_ptr<TextModel> label, QWidget* parent)
    : QWidget(parent),
      m_label(std::move(label)),
      m_is_read_only(false),
      m_is_delete_hovered(false),
      m_is_delete_pressed(false),
      m_text_color(Qt::black),
      m_delete_fill(0xA0A0A0),
      m_delete_icon(get_delete_icon()) {
  setMouseTracking(true);
  m_label_connection =
    m_label->connect_update_signal([this] (const auto&) { update(); });
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&Tag::on_style, this));
  set_style(*this, DEFAULT_STYLE());
}

const std::shared_ptr<TextModel>& Tag::get_label() const {
  return m_label;
}

bool Tag::is_read_only() const {
  return m_is_read_only;
}

void Tag::set_read_only(bool is_read_only) {
  m_is_read_only = is_read_only;
  if(m_is_read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
  updateGeometry();
  update();
}

connection Tag::connect_delete_signal(
    const DeleteSignal::slot_type& slot) const {
  return m_delete_signal.connect(slot);
}

QSize Tag::sizeHint() const {
  auto metrics = QFontMetrics(m_font);
  auto content_width = metrics.horizontalAdvance(m_label->get());
  if(is_delete_visible()) {
    content_width += m_delete_icon.width();
  }
  auto content_height = metrics.height();
  return QSize(content_width, content_height) +
    m_geometry.get_geometry().size() - m_geometry.get_content_area().size();
}

void Tag::mouseMoveEvent(QMouseEvent* event) {
  if(!is_delete_visible()) {
    return;
  }
  auto was_hovered = m_is_delete_hovered;
  m_is_delete_hovered = get_delete_rect().contains(event->pos());
  if(was_hovered != m_is_delete_hovered) {
    update();
  }
}

void Tag::mousePressEvent(QMouseEvent* event) {
  if(is_delete_visible() && get_delete_rect().contains(event->pos())) {
    m_is_delete_pressed = true;
    update();
  }
}

void Tag::mouseReleaseEvent(QMouseEvent* event) {
  if(m_is_delete_pressed) {
    m_is_delete_pressed = false;
    if(get_delete_rect().contains(event->pos())) {
      m_delete_signal();
    }
    update();
  }
}

void Tag::leaveEvent(QEvent* event) {
  if(m_is_delete_hovered) {
    m_is_delete_hovered = false;
    update();
  }
}

void Tag::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  m_painter.paint(painter);
  auto& content_area = m_geometry.get_content_area();
  auto metrics = QFontMetrics(m_font);
  auto available_width = content_area.width();
  if(is_delete_visible()) {
    available_width -= m_delete_icon.width();
  }
  auto elided =
    metrics.elidedText(m_label->get(), Qt::ElideRight, available_width);
  painter.setPen(m_text_color);
  painter.setFont(m_font);
  painter.drawText(content_area.left(), content_area.top() + metrics.ascent(),
    elided);
  if(is_delete_visible()) {
    auto delete_rect = get_delete_rect();
    auto fill = m_delete_fill;
    if(m_is_delete_pressed || m_is_delete_hovered) {
      fill = QColor(0x4B23A0);
    }
    auto icon = QPixmap::fromImage(m_delete_icon);
    auto icon_painter = QPainter(&icon);
    icon_painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    icon_painter.fillRect(icon.rect(), fill);
    icon_painter.end();
    painter.drawPixmap(delete_rect.topLeft(), icon);
  }
}

void Tag::resizeEvent(QResizeEvent* event) {
  m_geometry.set_size(size());
  QWidget::resizeEvent(event);
}

QRect Tag::get_delete_rect() const {
  auto& content_area = m_geometry.get_content_area();
  auto icon_size = m_delete_icon.size();
  auto x = content_area.right() + 1 - icon_size.width();
  auto y =
    content_area.top() + (content_area.height() - icon_size.height()) / 2;
  return QRect(QPoint(x, y), icon_size);
}

bool Tag::is_delete_visible() const {
  return !m_is_read_only;
}

void Tag::on_style() {
  auto& stylist = find_stylist(*this);
  auto font_size = std::make_shared<std::optional<int>>();
  for(auto& property : stylist.get_computed_block()) {
    apply(property, m_geometry, stylist);
    apply(property, m_painter, stylist);
    property.visit(
      [&] (const TextColor& color) {
        stylist.evaluate(color, [this] (auto color) {
          m_text_color = color;
          update();
        });
      },
      [&] (const Font& font) {
        stylist.evaluate(font, [this] (const auto& font) {
          m_font = font;
          updateGeometry();
          update();
        });
      },
      [&] (const FontSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          *font_size = size;
        });
      });
  }
  if(*font_size) {
    m_font.setPixelSize(**font_size);
    updateGeometry();
    update();
  }
}
