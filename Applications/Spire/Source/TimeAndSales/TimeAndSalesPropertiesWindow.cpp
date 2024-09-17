#include "Spire/TimeAndSales/TimeAndSalesPropertiesWindow.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/FontBox.hpp"
#include "Spire/Ui/HighlightBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto apply_header_label_style(StyleSheet& style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(12));
    style.get(Any()).
      set(Font(font)).
      set(PaddingBottom(scale_height(8)));
  }

  auto apply_content_style(StyleSheet& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(18)));
  }

  auto apply_actions_style(StyleSheet& style) {
    style.get(Any()).
      set(BorderTopSize(scale_width(1))).
      set(BorderTopColor(QColor(0xE0E0E0))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  }

  const QString& get_bbo_indicator_name(int index) {
    static const auto names = std::array<QString, BBO_INDICATOR_COUNT>{
      QObject::tr("None"), QObject::tr("Above Ask"), QObject::tr("At Ask"),
        QObject::tr("Inside"), QObject::tr("At Bid"), QObject::tr("Below Bid")};
    return names[index];
  }

  auto make_bbo_indicator_row(const QString& name,
      std::shared_ptr<HighlightColorModel> color_model, QWidget& parent) {
    auto layout = make_hbox_layout();
    auto label = make_label(name);
    update_style(*label, [] (auto& style) {
      style.get(Any()).set(PaddingRight(scale_width(8)));
    });
    layout->addWidget(label);
    auto highlight_box = new HighlightBox(std::move(color_model));
    highlight_box->setFixedSize(scale(120, 20));
    link(parent, *highlight_box);
    layout->addWidget(highlight_box);
    return layout;
  }
}

struct TimeAndSalesPropertiesWindow::PropertiesWindowModel {
  std::shared_ptr<TimeAndSalesPropertiesModel> m_properties_model;
  std::shared_ptr<ValueModel<QFont>> m_font_model;
  std::shared_ptr<BooleanModel> m_show_grid_model;
  std::array<std::shared_ptr<HighlightColorModel>, BBO_INDICATOR_COUNT>
    m_highlight_models;
  scoped_connection m_properties_connection;
  scoped_connection m_font_connection;
  scoped_connection m_show_grid_connection;
  std::array<scoped_connection, BBO_INDICATOR_COUNT> m_highlight_connections;

  explicit PropertiesWindowModel(
      std::shared_ptr<TimeAndSalesPropertiesModel> properties_model)
      : m_properties_model(std::move(properties_model)),
        m_font_model(std::make_shared<LocalValueModel<QFont>>(
          m_properties_model->get().get_font())),
        m_show_grid_model(std::make_shared<LocalBooleanModel>(
          m_properties_model->get().is_grid_enabled())) {
    m_properties_connection = m_properties_model->connect_update_signal(
      std::bind_front(&PropertiesWindowModel::on_properties, this));
    m_font_connection = m_font_model->connect_update_signal(
      std::bind_front(&PropertiesWindowModel::on_font, this));
    m_show_grid_connection = m_show_grid_model->connect_update_signal(
      std::bind_front(&PropertiesWindowModel::on_show_grid, this));
    for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
      auto indicator = static_cast<BboIndicator>(i);
      m_highlight_models[i] = std::make_shared<LocalHighlightColorModel>(
        m_properties_model->get().get_highlight_color(indicator));
      m_highlight_connections[i] = m_highlight_models[i]->connect_update_signal(
        std::bind_front(&PropertiesWindowModel::on_highlight, this, indicator));
    }
  }

  void on_properties(const TimeAndSalesProperties& properties) {
    if(properties.get_font() != m_font_model->get()) {
      auto blocker = shared_connection_block(m_font_connection);
      m_font_model->set(properties.get_font());
    }
    if(properties.is_grid_enabled() != m_show_grid_model->get()) {
      auto blocker = shared_connection_block(m_show_grid_connection);
      m_show_grid_model->set(properties.is_grid_enabled());
    }
    for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
      auto indicator = static_cast<BboIndicator>(i);
      if(properties.get_highlight_color(indicator) !=
          m_highlight_models[i]->get()) {
        auto blocker = shared_connection_block(m_highlight_connections[i]);
        m_highlight_models[i]->set(properties.get_highlight_color(indicator));
      }
    }
  }

  void on_font(const QFont& font) {
    auto properties = m_properties_model->get();
    properties.set_font(font);
    auto blocker = shared_connection_block(m_properties_connection);
    m_properties_model->set(properties);
  }

  void on_show_grid(bool checked) {
    auto properties = m_properties_model->get();
    properties.set_grid_enabled(checked);
    auto blocker = shared_connection_block(m_properties_connection);
    m_properties_model->set(properties);
  }

  void on_highlight(BboIndicator indicator, const HighlightColor& highlight) {
    auto properties = m_properties_model->get();
    properties.set_highlight_color(indicator, highlight);
    auto blocker = shared_connection_block(m_properties_connection);
    m_properties_model->set(properties);
  }
};

TimeAndSalesPropertiesWindow::TimeAndSalesPropertiesWindow(
    std::shared_ptr<TimeAndSalesPropertiesModel> current, QWidget* parent)
    : Window(parent),
      m_model(std::make_unique<PropertiesWindowModel>(std::move(current))),
      m_initial_properties(m_model->m_properties_model->get()) {
  set_svg_icon(":/Icons/time-sales.svg");
  setWindowTitle(tr("Time and Sales Properties"));
  setWindowFlags(windowFlags() & ~Qt::WindowMinimizeButtonHint);
  auto text_header = make_label(tr("Text"));
  text_header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  update_style(*text_header, apply_header_label_style);
  auto font_box = new FontBox(m_model->m_font_model);
  font_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  font_box->get_current()->connect_update_signal(
    std::bind_front(&TimeAndSalesPropertiesWindow::on_font, this));
  auto grid_check_box = new CheckBox(m_model->m_show_grid_model);
  grid_check_box->set_label(tr("Show Grid"));
  grid_check_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  auto bbo_indicator_header = make_label(tr("BBO Indicators"));
  bbo_indicator_header->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Fixed);
  update_style(*bbo_indicator_header, apply_header_label_style);
  auto indicators_layout = make_vbox_layout();
  indicators_layout->setSpacing(scale_height(8));
  for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
    indicators_layout->addLayout(
      make_bbo_indicator_row(get_bbo_indicator_name(i),
        m_model->m_highlight_models[i], *this));
  }
  auto content_body = new QWidget();
  auto content_body_layout = make_vbox_layout(content_body);
  content_body_layout->addWidget(text_header);
  content_body_layout->addWidget(font_box);
  content_body_layout->addSpacing(scale_height(24));
  content_body_layout->addWidget(grid_check_box, 0, Qt::AlignLeft);
  content_body_layout->addSpacing(scale_height(24));
  content_body_layout->addWidget(bbo_indicator_header);
  content_body_layout->addLayout(indicators_layout);
  auto content_box = new Box(content_body);
  content_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*content_box, apply_content_style);
  auto cancel_button = make_label_button("Cancel");
  cancel_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  cancel_button->connect_click_signal(
    std::bind_front(&TimeAndSalesPropertiesWindow::on_cancel, this));
  auto done_button = make_label_button("Done");
  done_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  done_button->connect_click_signal(
    std::bind_front(&TimeAndSalesPropertiesWindow::on_done, this));
  auto actions_body = new QWidget();
  auto actions_body_layout = make_hbox_layout(actions_body);
  actions_body_layout->setSpacing(scale_width(8));
  actions_body_layout->addWidget(cancel_button);
  actions_body_layout->addWidget(done_button);
  auto actions_box = new Box(actions_body);
  actions_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*actions_box, apply_actions_style);
  auto body = new QWidget();
  body->setFixedWidth(scale_width(256));
  auto body_layout = make_vbox_layout(body);
  body_layout->addWidget(content_box);
  body_layout->addWidget(actions_box);
  set_body(body);
  on_font(m_model->m_font_model->get());
}

const std::shared_ptr<TimeAndSalesPropertiesModel>&
    TimeAndSalesPropertiesWindow::get_current() const {
  return m_model->m_properties_model;
}

void TimeAndSalesPropertiesWindow::closeEvent(QCloseEvent*) {
  m_initial_properties = m_model->m_properties_model->get();
}

void TimeAndSalesPropertiesWindow::on_font(const QFont& font) {
  update_style(*this, [&] (auto& style) {
    style.get(Any() > is_a<HighlightBox>() > is_a<TextBox>()).
      set(Font(font));
  });
}

void Spire::TimeAndSalesPropertiesWindow::on_cancel() {
  m_model->m_properties_model->set(m_initial_properties);
  close();
}

void Spire::TimeAndSalesPropertiesWindow::on_done() {
  close();
}
