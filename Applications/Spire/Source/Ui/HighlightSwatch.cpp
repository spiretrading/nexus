#include "Spire/Ui/HighlightSwatch.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

HighlightSwatch::HighlightSwatch(QWidget* parent)
  : HighlightSwatch(std::make_shared<LocalValueModel<Highlight>>(), parent) {}

HighlightSwatch::HighlightSwatch(std::shared_ptr<ValueModel<Highlight>> current,
    QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)) {
  auto label = make_label("Aa");
  label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  enclose(*this, *label);
  proxy_style(*this, *label);
  auto font = QFont("Roboto");
  font.setWeight(QFont::Medium);
  font.setPixelSize(scale_width(10));
  update_style(*this, [&] (auto& style) {
    style.get(Any()).
      set(border(scale_width(1), QColor(0, 0, 0, 51))).
      set(Font(std::move(font))).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
  });
  on_current(m_current->get());
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&HighlightSwatch::on_current, this));
}

const std::shared_ptr<ValueModel<HighlightSwatch::Highlight>>&
    HighlightSwatch::get_current() const {
  return m_current;
}

void HighlightSwatch::on_current(const Highlight& highlight) {
  update_style(*this, [&] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(highlight.m_background_color))).
      set(TextColor(QColor(highlight.m_text_color)));
  });
}
