#include "Spire/BookView/BboBox.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto BORDER_TOP_COLOR = QColor(0xC8C8C8);
  const auto DOWNTICK_BORDER_TOP_COLOR = QColor(0xE63F44);
  const auto UPTICK_BORDER_TOP_COLOR = QColor(0x1FD37A);
  const auto MINIMUM_FONT_TATIO = 0.4;
  const auto MAXIMUM_FONT_TATIO = 1.8;
  const auto FONT_ADJUSTMENT = 6.89;
  const auto WIDTH_SCALE_FACTOR = 0.48276;

  template<typename T, typename U>
  auto make_bbo_label(std::shared_ptr<QuoteModel> quote, U field) {
    auto label = make_label(
      make_to_text_model<T>(make_field_value_model(quote, field),
        [] (const auto& value) {
          return to_text(value);
        }));
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    return label;
  }

  auto get_gap_width(int font_size, int parent_width) {
    return std::clamp(
      WIDTH_SCALE_FACTOR * parent_width - FONT_ADJUSTMENT * font_size,
      MINIMUM_FONT_TATIO * font_size, MAXIMUM_FONT_TATIO * font_size);
  }
}

BboBox::BboBox(std::shared_ptr<QuoteModel> quote, QWidget* parent)
    : QWidget(parent),
      m_quote(std::move(quote)),
      m_previous_price(m_quote->get().m_price),
      m_font_size(scale_width(10)),
      m_quote_connection(m_quote->connect_update_signal(
        std::bind_front(&BboBox::on_quote, this))) {
  auto body = new QWidget();
  m_body_layout = make_hbox_layout(body);
  m_money_label = make_bbo_label<Money>(m_quote, &Quote::m_price);
  link(*this, *m_money_label);
  m_body_layout->addStretch(1);
  m_body_layout->addWidget(m_money_label);
  auto gap_width = get_gap_width(m_font_size, width());
  m_gap1 = new QSpacerItem(gap_width, 0, QSizePolicy::Fixed);
  m_body_layout->addItem(m_gap1);
  auto label = make_label(tr("/"));
  label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  link(*this, *label);
  m_body_layout->addWidget(label);
  m_gap2 = new QSpacerItem(gap_width, 0, QSizePolicy::Fixed);
  m_body_layout->addItem(m_gap2);
  auto quantity_label = make_bbo_label<Quantity>(m_quote, &Quote::m_size);
  link(*this, *quantity_label);
  m_body_layout->addWidget(quantity_label);
  m_body_layout->addStretch(1);
  auto box = new Box(body);
  enclose(*this, *box);
  proxy_style(*this, *box);
  auto font = QFont("Roboto");
  font.setWeight(QFont::Medium);
  font.setPixelSize(m_font_size);
  update_style(*this, [&] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(BorderTopSize(scale_height(2))).
      set(BorderTopColor(BORDER_TOP_COLOR)).
      set(padding(scale_width(4)));
    style.get(Downtick()).set(BorderTopColor(DOWNTICK_BORDER_TOP_COLOR));
    style.get(Uptick()).set(BorderTopColor(UPTICK_BORDER_TOP_COLOR));
    style.get(Any() > is_a<TextBox>()).
      set(Font(font));
  });
  on_quote(m_quote->get());
  m_style_connection = connect_style_signal(*m_money_label,
    std::bind_front(&BboBox::on_style, this));
}

const std::shared_ptr<QuoteModel>& BboBox::get_quote() const {
  return m_quote;
}

void BboBox::resizeEvent(QResizeEvent* event) {
  update_gap_width();
  QWidget::resizeEvent(event);
}

void BboBox::update_gap_width() {
  auto gap_width = get_gap_width(m_font_size, width());
  m_gap1->changeSize(gap_width, 0, QSizePolicy::Fixed);
  m_gap2->changeSize(gap_width, 0, QSizePolicy::Fixed);
  m_body_layout->invalidate();
}

void BboBox::on_quote(const Quote& quote) {
  if(m_previous_price < quote.m_price) {
    unmatch(*this, Downtick());
    match(*this, Uptick());
  } else if(m_previous_price > quote.m_price) {
    unmatch(*this, Uptick());
    match(*this, Downtick());
  }
  m_previous_price = quote.m_price;
}

void BboBox::on_style() {
  auto previous_font_size = m_font_size;
  auto& stylist = find_stylist(*m_money_label);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const FontSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_font_size = size;
        });
      });
  }
  if(previous_font_size != m_font_size) {
    update_gap_width();
  }
}
