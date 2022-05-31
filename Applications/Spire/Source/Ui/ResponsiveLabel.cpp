#include "Spire/Ui/ResponsiveLabel.hpp"
#include <tuple>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

ResponsiveLabel::ResponsiveLabel(
    std::shared_ptr<ListModel<QString>> labels, QWidget* parent)
    : m_labels(std::move(labels)),
      m_text_model(std::make_shared<LocalTextModel>()) {
  m_text_box = make_label(m_text_model, this);
  proxy_style(*this, *m_text_box);
  m_style_connection =
    connect_style_signal(*m_text_box, [=] { on_text_box_style(); });
  enclose(*this, *m_text_box);
  m_list_operation_connection = m_labels->connect_operation_signal(
    std::bind_front(&ResponsiveLabel::on_list_operation, this));
  on_text_box_style();
  reset_mapped_labels();
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
  return m_size_hint;
}

void ResponsiveLabel::resizeEvent(QResizeEvent* event) {
  if(is_outside_current_bounds(width())) {
    update_display_text();
  }
  QWidget::resizeEvent(event);
}

int ResponsiveLabel::get_current_label_pixel_width() const {
  if(!m_current_mapped_index) {
    return 0;
  }
  return m_mapped_labels.at(*m_current_mapped_index).m_pixel_width;
}

int ResponsiveLabel::get_next_label_pixel_width() const {
  if(!m_current_mapped_index) {
    return 0;
  } else if(*m_current_mapped_index + 1 >
      static_cast<int>(m_mapped_labels.size()) - 1) {
    return QWIDGETSIZE_MAX;
  }
  return m_mapped_labels.at(*m_current_mapped_index + 1).m_pixel_width;
}

int ResponsiveLabel::get_pixel_width(const QString& text) const {
  return QFontMetrics(m_text_box_font).horizontalAdvance(text);
}

bool ResponsiveLabel::is_outside_current_bounds(int width) const {
  return width < get_current_label_pixel_width() ||
    width >= get_next_label_pixel_width();
}

void ResponsiveLabel::reset_mapped_labels() {
  m_current_mapped_index = none;
  m_mapped_labels.clear();
  m_mapped_labels.reserve(m_labels->get_size());
  for(auto i = 0; i < m_labels->get_size(); ++i) {
    m_mapped_labels.push_back({i, get_pixel_width(m_labels->get(i))});
  }
  sort_mapped_labels();
}

void ResponsiveLabel::set_current(const optional<int>& mapped_label_index) {
  m_current_mapped_index = mapped_label_index;
  if(!m_current_mapped_index) {
    m_text_model->set("");
    return;
  }
  m_text_model->set(
    m_labels->get(m_mapped_labels.at(*m_current_mapped_index).m_list_index));
}

void ResponsiveLabel::sort_mapped_labels() {
  std::sort(m_mapped_labels.begin(), m_mapped_labels.end(),
    [=] (const auto& first, const auto& second) {
      return std::tie(first.m_pixel_width, -first.m_list_index) <
        std::tie(second.m_pixel_width, -second.m_list_index);
    });
}

void ResponsiveLabel::update_display_text() {
  if(m_mapped_labels.empty()) {
    set_current(none);
    return;
  } else if(m_mapped_labels.back().m_pixel_width <= width()) {
    set_current(static_cast<int>(m_mapped_labels.size()) - 1);
    return;
  }
  auto mapped_label = [&] {
    return std::lower_bound(m_mapped_labels.begin(), m_mapped_labels.end(),
      width() + 1, [=] (const auto& mapped_label, auto width) {
        return mapped_label.m_pixel_width < width;
      });
  }();
  if(mapped_label == m_mapped_labels.begin()) {
    set_current(0);
    return;
  }
  set_current(std::distance(m_mapped_labels.begin(), std::prev(mapped_label)));
}

void ResponsiveLabel::update_size_hint() {
  auto previous_size_hint = m_size_hint;
  m_size_hint = QSize(0, 0);
  auto& stylist = find_stylist(*m_text_box);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const BorderLeftSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_size_hint.rwidth() += size;
        });
      },
      [&] (const BorderTopSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_size_hint.rheight() += size;
        });
      },
      [&] (const BorderRightSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_size_hint.rwidth() += size;
        });
      },
      [&] (const BorderBottomSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_size_hint.rheight() += size;
        });
      },
      [&] (const PaddingLeft& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_size_hint.rwidth() += size;
        });
      },
      [&] (const PaddingTop& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_size_hint.rheight() += size;
        });
      },
      [&] (const PaddingRight& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_size_hint.rwidth() += size;
        });
      },
      [&] (const PaddingBottom& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_size_hint.rheight() += size;
        });
      });
  }
  if(!m_mapped_labels.empty()) {
    m_size_hint.rwidth() += m_mapped_labels.back().m_pixel_width;
    m_size_hint.rheight() += QFontMetrics(m_text_box_font).height();
  }
  if(m_size_hint != previous_size_hint) {
    updateGeometry();
  }
}

void ResponsiveLabel::on_label_added(int index) {
  m_mapped_labels.push_back({index, get_pixel_width(m_labels->get(index))});
  if(m_mapped_labels.size() > 1) {
    if(m_mapped_labels.back().m_pixel_width <=
        m_mapped_labels.at(m_mapped_labels.size() - 2).m_pixel_width) {
      sort_mapped_labels();
    }
    if(m_mapped_labels.back().m_pixel_width > m_size_hint.width()) {
      update_size_hint();
    }
  }
  update_display_text();
}

void ResponsiveLabel::on_label_removed(int index) {
  auto size_hint_updated = index == m_mapped_labels.back().m_list_index;
  for(auto i = m_mapped_labels.begin(); i != m_mapped_labels.end(); ++i) {
    if(i->m_list_index == index) {
      i = m_mapped_labels.erase(i);
      if(i == m_mapped_labels.end()) {
        break;
      }
    }
    if(i->m_list_index > index) {
      --(i->m_list_index);
    }
  }
  if(size_hint_updated) {
    update_size_hint();
  }
  update_display_text();
}

void ResponsiveLabel::on_label_updated(int index) {
  auto mapped_label =
    std::find_if(m_mapped_labels.begin(), m_mapped_labels.end(),
      [&] (const auto& mapped_label) {
        return index == mapped_label.m_list_index;
      });
  mapped_label->m_pixel_width = get_pixel_width(m_labels->get(index));
  if(m_mapped_labels.size() > 1) {
    auto lower = [&] {
      if(mapped_label == m_mapped_labels.begin()) {
        return 0;
      }
      return std::prev(mapped_label)->m_pixel_width;
    }();
    auto upper = [&] {
      if(std::next(mapped_label) == m_mapped_labels.end()) {
        return mapped_label->m_pixel_width;
      }
      return std::next(mapped_label)->m_pixel_width;
    }();
    if(mapped_label->m_pixel_width <= lower ||
        upper <= mapped_label->m_pixel_width) {
      sort_mapped_labels();
      update_size_hint();
    }
  }
  update_display_text();
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
  auto previous_font = m_text_box_font;
  auto previous_font_size = m_text_box_font.pixelSize();
  auto& stylist = find_stylist(*m_text_box);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const Font& font) {
        stylist.evaluate(font, [=] (const auto& font) {
          m_text_box_font = font;
        });
      },
      [&] (const FontSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_text_box_font.setPixelSize(size);
        });
      });
  }
  if(previous_font != m_text_box_font ||
      previous_font_size != m_text_box_font.pixelSize()) {
    reset_mapped_labels();
    update_display_text();
  }
  update_size_hint();
}
