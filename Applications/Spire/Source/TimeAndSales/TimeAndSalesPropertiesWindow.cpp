#include "Spire/TimeAndSales/TimeAndSalesPropertiesWindow.hpp"
#include <QFontDatabase>
#include <QStackedLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/FontBox.hpp"
#include "Spire/Ui/HighlightBox.hpp"
#include "Spire/Ui/HighlightPicker.hpp"
#include "Spire/Ui/layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto HEADER_LABEL_STYLE(StyleSheet style, const QFont& font) {
    style.get(Any()).
      set(Font(font)).
      set(PaddingBottom(scale_height(8)));
    return style;
  }

  const QString& get_bbo_indicator_name(int index) {
    static const auto names = std::array<QString, BBO_INDICATOR_COUNT>{
      QObject::tr("None"), QObject::tr("Above Ask"), QObject::tr("At Ask"),
        QObject::tr("Inside"), QObject::tr("At Bid"), QObject::tr("Below Bid")};
    return names[index];
  }

  auto make_bbo_indicator_row(const QString& name,
      std::shared_ptr<HighlightColorModel> color_model) {
    auto layout = make_hbox_layout();
    auto label = make_label(name);
    update_style(*label, [] (auto& style) {
      style.get(Any()).set(PaddingRight(scale_width(8)));
    });
    layout->addWidget(label);
    auto highlight_box = new HighlightBox(std::move(color_model));
    highlight_box->setFixedSize(scale(120, 20));
    layout->addWidget(highlight_box);
    return layout;
  }

  auto get_highlight_box(const QLayout& layout, int index) {
    return layout.itemAt(index)->layout()->itemAt(1)->widget();
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
          m_properties_model->get().is_show_grid())) {
    m_properties_model->connect_update_signal(std::bind_front(
      &PropertiesWindowModel::on_properties, this));
    m_font_model->connect_update_signal(std::bind_front(
      &PropertiesWindowModel::on_font, this));
    m_show_grid_model->connect_update_signal(std::bind_front(
      &PropertiesWindowModel::on_show_grid, this));
    for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
      m_highlight_models[i] = std::make_shared<LocalHighlightColorModel>(
        m_properties_model->get().get_highlight(static_cast<BboIndicator>(i)));
      m_highlight_connections[i] = m_highlight_models[i]->connect_update_signal(
        std::bind_front(&PropertiesWindowModel::on_highlight, this,
          static_cast<BboIndicator>(i)));
    }
  }

  void on_properties(const TimeAndSalesProperties& properties) {
    if(properties.get_font() != m_font_model->get()) {
      auto blocker = shared_connection_block(m_font_connection);
      m_font_model->set(properties.get_font());
    }
    if(properties.is_show_grid() != m_show_grid_model->get()) {
      auto blocker = shared_connection_block(m_show_grid_connection);
      m_show_grid_model->set(properties.is_show_grid());
    }
    for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
      auto indicator = static_cast<BboIndicator>(i);
      if(properties.get_highlight(indicator) != m_highlight_models[i]->get()) {
        auto blocker = shared_connection_block(m_show_grid_connection);
        m_highlight_models[i]->set(properties.get_highlight(indicator));
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
    properties.set_show_grid(checked);
    auto blocker = shared_connection_block(m_properties_connection);
    m_properties_model->set(properties);
  }

  void on_highlight(BboIndicator indicator, const HighlightColor& highlight) {
    auto properties = m_properties_model->get();
    properties.set_highlight(indicator, highlight);
    auto blocker = shared_connection_block(m_properties_connection);
    m_properties_model->set(properties);
  }
};

TimeAndSalesPropertiesWindow::TimeAndSalesPropertiesWindow(
    std::shared_ptr<TimeAndSalesPropertiesModel> properties, QWidget* parent)
    : Window(parent),
      m_model(std::make_unique<PropertiesWindowModel>(std::move(properties))) {
  set_svg_icon(":/Icons/time-sales.svg");
  setWindowTitle(tr("Time and Sales Properties"));
  setWindowFlags(windowFlags() & ~Qt::WindowMinimizeButtonHint);
  auto header_font = QFont("Roboto");
  header_font.setWeight(QFont::Medium);
  header_font.setPixelSize(scale_width(12));
  m_body = new QWidget();
  m_body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto body_layout = make_vbox_layout(m_body);
  auto text_header = make_label(tr("Text"));
  text_header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  update_style(*text_header, [&] (auto& style) {
    style = HEADER_LABEL_STYLE(style, header_font);
  });
  body_layout->addWidget(text_header);
  m_font_box = new FontBox(m_model->m_font_model);
  m_font_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  m_font_box->get_current()->connect_update_signal(
    std::bind_front(&TimeAndSalesPropertiesWindow::on_font, this));
  body_layout->addWidget(m_font_box);
  body_layout->addSpacing(scale_height(24));
  m_grid_check_box = new CheckBox(m_model->m_show_grid_model);
  m_grid_check_box->set_label(tr("Show Grid"));
  m_grid_check_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  body_layout->addWidget(m_grid_check_box, 0, Qt::AlignLeft);
  body_layout->addSpacing(scale_height(24));
  auto bbo_indicator_header = make_label(tr("BBO Indicators"));
  bbo_indicator_header->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Minimum);
  update_style(*bbo_indicator_header, [&] (auto& style) {
    style = HEADER_LABEL_STYLE(style, header_font);
  });
  body_layout->addWidget(bbo_indicator_header);
  m_indicators_layout = make_vbox_layout();
  m_indicators_layout->setSpacing(scale_height(8));
  for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
    m_indicators_layout->addLayout(
      make_bbo_indicator_row(get_bbo_indicator_name(i),
        m_model->m_highlight_models[i]));
  }
  body_layout->addLayout(m_indicators_layout);
  auto box = new Box(m_body);
  box->setFixedWidth(scale_width(256));
  update_style(*box, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(18)));
  });
  set_body(box);
  on_font(m_model->m_font_model->get());
}

const std::shared_ptr<TimeAndSalesPropertiesModel>&
    TimeAndSalesPropertiesWindow::get_properties() const {
  return m_model->m_properties_model;
}

void TimeAndSalesPropertiesWindow::showEvent(QShowEvent* event) {
  auto font_size_box = [&] () -> QWidget* {
    auto stacked_layout = static_cast<QStackedLayout*>(
      m_font_box->layout()->itemAt(0)->widget()->layout());
    auto children = stacked_layout->currentWidget()->children();
    for(auto i = children.rbegin(); i != children.rend(); ++i) {
      if((*i)->isWidgetType()) {
        return static_cast<QWidget*>(*i);
      }
    }
    return nullptr;
  }();
  if(font_size_box) {
    auto check_box_proxy = find_focus_proxy(*m_grid_check_box);
    QWidget::setTabOrder(find_focus_proxy(*font_size_box), check_box_proxy);
    auto previous = check_box_proxy;
    for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
      auto highlight_box =
        m_indicators_layout->itemAt(i)->layout()->itemAt(1)->widget();
      QWidget::setTabOrder(previous, highlight_box);
      previous = highlight_box;
    }
  }
  Window::showEvent(event);
}

void TimeAndSalesPropertiesWindow::on_font(const QFont& font) {
  auto property = [&] () -> Property {
    auto font_database = QFontDatabase();
    if(m_font.family() != font.family() ||
        font_database.styleString(m_font) != font_database.styleString(font)) {
      return Font(font);
    } else if(m_font.pixelSize() != font.pixelSize()) {
      return FontSize(font.pixelSize());
    }
    return Font(font);
  }();
  for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
    auto highlight_box = get_highlight_box(*m_indicators_layout, i);
    auto highlight_label = static_cast<Box*>(
      highlight_box->layout()->itemAt(0)->widget())->get_body();
    update_style(*highlight_label, [&] (auto& style) {
      style.get(Any()).set(property);
    });
  }
  m_font = font;
}