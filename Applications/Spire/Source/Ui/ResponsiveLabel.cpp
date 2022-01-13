#include "Spire/Ui/ResponsiveLabel.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/LocalValueModel.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

ResponsiveLabel::ResponsiveLabel(
    std::shared_ptr<ListModel<QString>> labels, QWidget* parent)
    : m_labels(std::move(labels)),
      m_text_model(std::make_shared<LocalTextModel>()),
      m_current_label_length(0),
      m_next_label_length(0) {
  m_text_box = make_label(m_text_model, this);
  proxy_style(*this, *m_text_box);
  m_style_connection =
    connect_style_signal(*m_text_box, [=] { on_text_box_style(); });
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_text_box);
  m_list_operation_connection = m_labels->connect_operation_signal(
    [=] (auto operation) { on_list_operation(operation); });
  update_current_font();
  reset_cached_labels();
  update_display_text();
}

const std::shared_ptr<ListModel<QString>>&
    ResponsiveLabel::get_labels() const {
  return m_labels;
}

const std::shared_ptr<TextModel>& ResponsiveLabel::get_current() const {
  return m_text_box->get_current();
}

const std::shared_ptr<HighlightModel>& ResponsiveLabel::get_highlight() const {
  return m_text_box->get_highlight();
}

QSize ResponsiveLabel::sizeHint() const {
  return m_text_box->sizeHint();
}

void ResponsiveLabel::resizeEvent(QResizeEvent* event) {
  if(is_outside_current_bounds(width())) {
    update_display_text();
  }
  QWidget::resizeEvent(event);
}

int ResponsiveLabel::get_pixel_width(const QString& text) const {
  return QFontMetrics(m_text_box_font).horizontalAdvance(text);
}

bool ResponsiveLabel::is_outside_current_bounds(int width) const {
  return width < m_current_label_length || width >= m_next_label_length;
}

void ResponsiveLabel::reset_cached_labels() {
  m_current_cached_label_index = none;
  m_cached_labels.clear();
  m_cached_labels.reserve(m_labels->get_size());
  for(auto i = 0; i < m_labels->get_size(); ++i) {
    m_cached_labels.push_back({i, get_pixel_width(m_labels->get(i))});
  }
  sort_cached_labels();
}

void ResponsiveLabel::set_current(const optional<int> cached_label_index) {
  m_current_cached_label_index = cached_label_index;
  if(!cached_label_index) {
    update_current_bounds(m_current_cached_label_index);
    m_text_model->set("");
    return;
  }
  update_current_bounds(m_current_cached_label_index);
  m_text_model->set(
    m_labels->get(m_cached_labels.at(*m_current_cached_label_index).m_index));
  adjustSize();
  if(get_pixel_width(m_text_model->get()) > width()) {
    update_display_text();
  }
}

void ResponsiveLabel::sort_cached_labels() {
  std::sort(m_cached_labels.begin(), m_cached_labels.end(),
    [=] (const auto& first, const auto& second) {
      return first.m_pixel_width < second.m_pixel_width;
    });
}

void ResponsiveLabel::update_current_bounds(
    const optional<int>& cached_label_index) {
  if(!m_current_cached_label_index) {
    m_current_label_length = 0;
    m_next_label_length = 0;
    return;
  }
  m_current_label_length = [&] {
    return m_cached_labels.at(*m_current_cached_label_index).m_pixel_width;
  }();
  m_next_label_length = [&] {
    if(*m_current_cached_label_index + 1 >
        static_cast<int>(m_cached_labels.size()) - 1) {
      return QWIDGETSIZE_MAX;
    }
    return m_cached_labels.at(*m_current_cached_label_index + 1).m_pixel_width;
  }();
}

void ResponsiveLabel::update_current_font() {
  auto updated_font = QFont();
  auto updated_font_size = m_text_box_font.pixelSize();
  auto& stylist = find_stylist(*m_text_box);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const Font& font) {
        stylist.evaluate(font, [&] (const auto& font) {
          updated_font = font;
        });
      },
      [&] (const FontSize& size) {
        stylist.evaluate(size, [&] (auto size) {
          updated_font_size = size;
        });
      });
  }
  if(updated_font != m_text_box_font ||
      updated_font_size != m_text_box_font.pixelSize()) {
    m_text_box_font = updated_font;
    m_text_box_font.setPixelSize(updated_font_size);
    auto current = m_current_cached_label_index;
    reset_cached_labels();
    set_current(current);
  }
}

void ResponsiveLabel::update_display_text() {
  if(m_cached_labels.empty() ||
      m_cached_labels.front().m_pixel_width > width()) {
    set_current(none);
    return;
  } else if(m_cached_labels.back().m_pixel_width < width()) {
    set_current(m_cached_labels.size() - 1);
    return;
  }
  auto cached_label = [&] {
    return std::lower_bound(m_cached_labels.begin(), m_cached_labels.end(),
      width() + 1, [=] (const auto& cached_label, auto width) {
        return cached_label.m_pixel_width < width;
      });
  }();
  set_current(std::distance(m_cached_labels.begin(), std::prev(cached_label)));
}

void ResponsiveLabel::on_label_added(int index) {
  m_cached_labels.push_back({index, get_pixel_width(m_labels->get(index))});
  if(index != m_labels->get_size() - 1) {
    sort_cached_labels();
  }
  set_current(m_cached_labels.size() - 1);
}

void ResponsiveLabel::on_label_removed(int index) {
  auto label = std::find_if(m_cached_labels.begin(), m_cached_labels.end(),
    [&] (const auto& cached_label) {
      return index == cached_label.m_index;
    });
  m_cached_labels.erase(label);
  // TODO: don't reset, decrement all indices that are greater than label's index
  reset_cached_labels();
  auto current_index = [&] () -> optional<int> {
    if(m_cached_labels.empty()) {
      return none;
    }
    return m_cached_labels.size() - 1;
  }();
  set_current(current_index);
}

void ResponsiveLabel::on_label_updated(int index) {
  auto cached_label =
    std::find_if(m_cached_labels.begin(), m_cached_labels.end(),
      [&] (const auto& cached_label) {
        return index == cached_label.m_index;
      });
  cached_label->m_pixel_width = get_pixel_width(m_labels->get(index));
  // TODO: optimizations
  sort_cached_labels();
  set_current(m_cached_labels.size() - 1);
}

void ResponsiveLabel::on_list_operation(
    const AnyListModel::Operation& operation) {
  visit(operation,
    [&] (const AnyListModel::AddOperation& operation) {
      on_label_added(operation.m_index);
    },
    [&] (const AnyListModel::RemoveOperation& operation) {
      on_label_removed(operation.m_index);
    },
    [&] (const AnyListModel::UpdateOperation& operation) {
      on_label_updated(operation.m_index);
    });
}

void ResponsiveLabel::on_text_box_style() {
  update_current_font();
}
