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
  const auto& DELETE_ICON() {
    static const auto ICON =
      image_from_svg(":/Icons/delete.svg", scale(16, 16));
    return ICON;
  }

  const auto& DELETE_PIXMAP() {
    static const auto PIXMAP = [] {
      auto pixmap = QPixmap::fromImage(DELETE_ICON());
      auto painter = QPainter(&pixmap);
      painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
      painter.fillRect(pixmap.rect(), QColor(0xA0A0A0));
      return pixmap;
    }();
    return PIXMAP;
  }

  const auto& DELETE_HOVER_PIXMAP() {
    static const auto PIXMAP = [] {
      auto pixmap = QPixmap::fromImage(DELETE_ICON());
      auto painter = QPainter(&pixmap);
      painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
      painter.fillRect(pixmap.rect(), QColor(0x4B23A0));
      return pixmap;
    }();
    return PIXMAP;
  }

  const auto& DEFAULT_STYLE() {
    static const auto STYLE = [] {
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
    }();
    return STYLE;
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
      m_font_metrics(m_font),
      m_elided_width(-1) {
  setMouseTracking(true);
  m_label_connection =
    m_label->connect_update_signal(std::bind_front(&Tag::on_label, this));
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
  if(m_is_read_only == is_read_only) {
    return;
  }
  m_is_read_only = is_read_only;
  if(m_is_read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
  invalidate_layout();
}

connection Tag::connect_delete_signal(
    const DeleteSignal::slot_type& slot) const {
  return m_delete_signal.connect(slot);
}

QSize Tag::sizeHint() const {
  auto content_width = m_font_metrics.horizontalAdvance(m_label->get());
  if(is_delete_visible()) {
    content_width += DELETE_ICON().width();
  }
  auto content_height = m_font_metrics.height();
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
  painter.setPen(m_text_color);
  painter.setFont(m_font);
  painter.drawText(content_area.left(),
    content_area.top() + m_font_metrics.ascent(), m_elided_label);
  if(is_delete_visible()) {
    if(m_is_delete_pressed || m_is_delete_hovered) {
      painter.drawPixmap(get_delete_rect().topLeft(), DELETE_HOVER_PIXMAP());
    } else {
      painter.drawPixmap(get_delete_rect().topLeft(), DELETE_PIXMAP());
    }
  }
}

void Tag::resizeEvent(QResizeEvent* event) {
  m_geometry.set_size(size());
  update_elided_label();
  QWidget::resizeEvent(event);
}

QRect Tag::get_delete_rect() const {
  auto& content_area = m_geometry.get_content_area();
  auto icon_size = DELETE_ICON().size();
  auto x = content_area.right() + 1 - icon_size.width();
  auto y =
    content_area.top() + (content_area.height() - icon_size.height()) / 2;
  return QRect(QPoint(x, y), icon_size);
}

bool Tag::is_delete_visible() const {
  return !m_is_read_only;
}

void Tag::invalidate_layout() {
  update_elided_label();
  updateGeometry();
  update();
}

void Tag::update_elided_label() {
  auto& content_area = m_geometry.get_content_area();
  auto available_width = content_area.width();
  if(is_delete_visible()) {
    available_width -= DELETE_ICON().width();
  }
  if(available_width == m_elided_width) {
    return;
  }
  m_elided_width = available_width;
  m_elided_label =
    m_font_metrics.elidedText(m_label->get(), Qt::ElideRight, m_elided_width);
}

void Tag::on_label(const QString& label) {
  m_elided_width = -1;
  invalidate_layout();
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
          m_font_metrics = QFontMetrics(m_font);
          m_elided_width = -1;
          invalidate_layout();
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
    m_font_metrics = QFontMetrics(m_font);
    m_elided_width = -1;
    invalidate_layout();
  }
}
