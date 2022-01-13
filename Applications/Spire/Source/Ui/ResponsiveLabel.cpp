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
    connect_style_signal(*this, [=] { on_text_box_style(); });
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_text_box);
  m_labels->connect_operation_signal(
    [=] (auto operation) { on_list_operation(operation); });
  update_current_font();
  reset_cached_labels();
  m_text_model->connect_update_signal([=] (const auto& str) {
    qDebug() << str;
  });
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
  qDebug() << "w: " << width();
  qDebug() << "tbw: " << m_text_box->width();
  if(is_outside_width(width())) {
    update_display_text();
  }
  QWidget::resizeEvent(event);
}

bool ResponsiveLabel::is_outside_width(int width) const {
  // TODO: if these members are only used here, then store the current cached label index
  //        and retrieve them when needed.
  return width < m_current_label_length || width >= m_next_label_length;
}

void ResponsiveLabel::reset_cached_labels() {
  m_cached_labels.clear();
  m_cached_labels.reserve(m_labels->get_size());
  for(auto i = 0; i < m_labels->get_size(); ++i) {
    m_cached_labels.push_back({i,
      QFontMetrics(m_text_box_font).horizontalAdvance(m_labels->get(i))});
  }
  sort_cached_labels();
  update_display_text();
}

void ResponsiveLabel::sort_cached_labels() {
  std::sort(m_cached_labels.begin(), m_cached_labels.end(),
    [=] (const auto& first, const auto& second) {
      return first.m_pixel_length <= second.m_pixel_length;
    });
}

void ResponsiveLabel::update_current_bounds(
    const boost::optional<int>& current_index) {
  if(!current_index) {
    m_current_label_length = 0;
    m_next_label_length = 0;
    return;
  }
  m_current_label_length = [&] {
    return m_cached_labels.at(*current_index).m_pixel_length;
  }();
  m_next_label_length = [&] {
    if(*current_index + 1 > static_cast<int>(m_cached_labels.size()) - 1) {
      return QWIDGETSIZE_MAX;
    }
    return m_cached_labels.at(*current_index + 1).m_pixel_length;
  }();
}

void ResponsiveLabel::update_current_font() {
  auto updated_font = QFont();
  auto updated_font_size = m_text_box_font.pixelSize();
  auto& stylist = find_stylist(*this);
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
    reset_cached_labels();
  }
}

void ResponsiveLabel::update_display_text() {
  if(m_cached_labels.empty()) {
    m_text_model->set("");
    update_current_bounds(none);
    return;
  } else if(m_cached_labels.back().m_pixel_length < width()) {
    update_current_bounds(m_cached_labels.size() - 1);
    auto index = m_cached_labels.back().m_index;
    m_text_model->set(m_labels->get(index));
    return;
  }
  auto cached_label = [&] {
    return std::lower_bound(m_cached_labels.begin(), m_cached_labels.end(),
      width() + 1, [=] (const auto& cached_label, auto width) {
        return cached_label.m_pixel_length < width;
      });
  }();
  --cached_label;
  if(cached_label != m_cached_labels.begin()) {
    update_current_bounds(std::distance(m_cached_labels.begin(), cached_label));
    m_text_model->set(m_labels->get(cached_label->m_index));
  } else {
    update_current_bounds(0);
    m_text_model->set(m_labels->get(m_cached_labels.at(0).m_index));
  }
}

void ResponsiveLabel::on_label_added(int index) {
  //m_cached_labels.push_back({index, text_pixel_length});
  //if(index != m_labels->get_size() - 1) {
  //  auto i = std::lower_bound(m_cached_labels.begin(), m_cached_labels.end(),
  //    [&] (auto cached_label) { return cached_label.m_pixel_length  });
  //}
}

void ResponsiveLabel::on_label_removed(int index) {
  //auto i = std::find_if(m_cached_labels.begin(), m_cached_labels.end(),
  //  [&] (const auto& cached_label) {
  //    return index == cached_label.m_index;
  //  });
  //m_cached_labels.erase(i);
}

void ResponsiveLabel::on_label_updated(int index) {
  //auto i = std::find_if(m_cached_labels.begin(), m_cached_labels.end(),
  //  [&] (const auto& cached_label) {
  //    return index == cached_label.m_index;
  //  });
  // TODO: update text length at index
  // TODO: only if required, move updated cached label to new index
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
