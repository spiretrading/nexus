#include "Spire/BookView/TechnicalsPanel.hpp"
#include <QEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Ui/AdaptiveBox.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto& LABEL_FONT() {
    static auto font = [] {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Medium);
      font.setPixelSize(scale_width(10));
      return font;
    }();
    return font;
  }

  auto get_value_field_minimum_width() {
    return QFontMetrics(LABEL_FONT()).averageCharWidth() * 8;
  }

  auto to_default_quantity(Quantity bid_quantity, Quantity ask_quantity) {
    return to_text(bid_quantity) + "x" + to_text(ask_quantity);
  }

  template<typename T, typename U>
  auto make_technicals_value_field(
      std::shared_ptr<SecurityTechnicalsModel> technicals, U field) {
    auto label = make_label(
      make_to_text_model<T>(make_field_value_model(technicals, field),
        [] (const auto& value) {
          return to_text(value);
        }));
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->setMinimumWidth(get_value_field_minimum_width());
    return label;
  }

  auto make_indicator_label(const QString& name) {
    auto label = make_label(name);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    update_style(*label, [] (auto& style) {
      style.get(Any()).set(PaddingRight(scale_width(4)));
    });
    return label;
  }

  auto make_large_layout(const std::vector<TextBox*>& indicators,
      const std::vector<TextBox*>& fields) {
    auto layout = make_hbox_layout();
    for(auto i = 0; i < std::ssize(indicators); ++i) {
      layout->addWidget(indicators[i]);
      layout->addWidget(fields[i]);
      if(i != std::ssize(indicators) - 1) {
        layout->addSpacing(scale_width(4));
      }
    }
    return layout;
  }

  auto make_small_layout(const std::vector<TextBox*>& indicators,
      const std::vector<TextBox*>& fields) {
    auto layout = make_grid_layout();
    layout->setVerticalSpacing(scale_height(2));
    for(auto i = 0, initial_column = 0, column = 0; i < std::ssize(indicators);
        ++i) {
      auto row = i % 2;
      layout->addWidget(indicators[i], row, column++);
      layout->addWidget(fields[i], row, column++);
      if(i < 4) {
        layout->addItem(new QSpacerItem(scale_width(4), 0, QSizePolicy::Fixed),
          row, column++);
      }
      if(row == 0) {
        column = initial_column;
      } else {
        initial_column = column;
      }
    }
    return layout;
  }
}

TechnicalsPanel::TechnicalsPanel(
    std::shared_ptr<SecurityTechnicalsModel> technicals,
    std::shared_ptr<QuantityModel> default_bid_quantity,
    std::shared_ptr<QuantityModel> default_ask_quantity, QWidget* parent)
    : QWidget(parent),
      m_technicals(std::move(technicals)),
      m_bid_quantity(std::move(default_bid_quantity)),
      m_ask_quantity(std::move(default_ask_quantity)),
      m_bid_quantity_connection(m_bid_quantity->connect_update_signal(
        std::bind_front(&TechnicalsPanel::on_bid_quantity_update, this))),
      m_ask_quantity_connection(m_ask_quantity->connect_update_signal(
        std::bind_front(&TechnicalsPanel::on_ask_quantity_update, this))) {
  m_default_field = make_label("");
  m_default_field->setMinimumWidth(get_value_field_minimum_width());
  on_ask_quantity_update(m_ask_quantity->get());
  auto name_indicators = std::vector<TextBox*>{
    make_indicator_label(tr("High")), make_indicator_label(tr("Low")),
    make_indicator_label(tr("Open")), make_indicator_label(tr("Close")),
    make_indicator_label(tr("Vol")), make_indicator_label(tr("Def"))};
  auto short_name_indicators = std::vector<TextBox*>{
    make_indicator_label(tr("H")), make_indicator_label(tr("L")),
    make_indicator_label(tr("O")), make_indicator_label(tr("C")),
    make_indicator_label(tr("V")), make_indicator_label(tr("D"))};
  auto fields = std::vector<TextBox*>{
    make_technicals_value_field<Money>(m_technicals,
      &SecurityTechnicals::m_high),
    make_technicals_value_field<Money>(m_technicals,
      &SecurityTechnicals::m_low),
    make_technicals_value_field<Money>(m_technicals,
      &SecurityTechnicals::m_open),
    make_technicals_value_field<Money>(m_technicals,
      &SecurityTechnicals::m_close),
    make_technicals_value_field<Quantity>(m_technicals,
      &SecurityTechnicals::m_volume),
    m_default_field};
  for(auto i = 0; i < std::ssize(fields); ++i) {
    link(*this, *name_indicators[i]);
    link(*this, *short_name_indicators[i]);
    link(*this, *fields[i]);
  }
  auto adaptive_box = new AdaptiveBox();
  adaptive_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  adaptive_box->add(*make_large_layout(name_indicators, fields));
  adaptive_box->add(*make_large_layout(short_name_indicators, fields));
  adaptive_box->add(*make_small_layout(name_indicators, fields));
  m_extra_small_layout = make_small_layout(short_name_indicators, fields);
  adaptive_box->add(*m_extra_small_layout);
  auto body = new QWidget();
  enclose(*body, *adaptive_box);
  auto box = new Box(body);
  box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  enclose(*this, *box);
  proxy_style(*this, *box);
  m_style_connection = connect_style_signal(*this,
    std::bind_front(&TechnicalsPanel::on_style, this));
  update_style(*this, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(4)));
    style.get(Any() > is_a<TextBox>()).
      set(Font(LABEL_FONT()));
  });
}

const std::shared_ptr<SecurityTechnicalsModel>&
    TechnicalsPanel::get_technicals() const {
  return m_technicals;
}

const std::shared_ptr<QuantityModel>&
    TechnicalsPanel::get_default_bid_quantity() const {
  return m_bid_quantity;
}

const std::shared_ptr<QuantityModel>&
    TechnicalsPanel::get_default_ask_quantity() const {
  return m_ask_quantity;
}

QSize TechnicalsPanel::minimumSizeHint() const {
  if(m_minimum_size_hint) {
    return *m_minimum_size_hint;
  }
  m_minimum_size_hint.emplace(m_extra_small_layout->totalMinimumSize().grownBy(
    get_content_margins(m_geometry)));
  return *m_minimum_size_hint;
}

void TechnicalsPanel::on_bid_quantity_update(Quantity quantity) {
  m_default_field->get_current()->set(
    to_default_quantity(quantity, m_ask_quantity->get()));
}

void TechnicalsPanel::on_ask_quantity_update(Quantity quantity) {
  m_default_field->get_current()->set(
    to_default_quantity(m_bid_quantity->get(), quantity));
}

void TechnicalsPanel::on_style() {
  auto& stylist = find_stylist(*this);
  for(auto& property : stylist.get_computed_block()) {
    apply(property, m_geometry, stylist);
  }
  m_minimum_size_hint = none;
  updateGeometry();
}
