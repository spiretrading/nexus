#include "Spire/Ui/HighlightPicker.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/ColorBox.hpp"
#include "Spire/Ui/ColorCodePanel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/OverlayPanel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  OverlayPanel* get_color_picker(const ColorBox& color_box) {
    auto children = color_box.children();
    for(auto child : children) {
      if(child->isWidgetType()) {
        if(auto widget = static_cast<QWidget*>(child);
          widget->windowFlags() & Qt::Popup) {
          return static_cast<OverlayPanel*>(widget);
        }
      }
    }
    return nullptr;
  }
}

struct HighlightPicker::HighlightPickerModel {
  std::shared_ptr<ValueModel<HighlightSwatch::Highlight>> m_highlight_model;
  std::shared_ptr<ColorModel> m_background_color_model;
  std::shared_ptr<ColorModel> m_text_color_model;
  scoped_connection m_highlight_connection;
  scoped_connection m_background_color_connection;
  scoped_connection m_text_color_connection;

  explicit HighlightPickerModel(std::shared_ptr<ValueModel<HighlightSwatch::Highlight>> highlight_model)
    : m_highlight_model(std::move(highlight_model)),
      m_background_color_model(std::make_shared<LocalColorModel>()),
      m_text_color_model(std::make_shared<LocalColorModel>()),
      m_highlight_connection(m_highlight_model->connect_update_signal(
        std::bind_front(&HighlightPickerModel::on_highlight_color, this))),
      m_background_color_connection(m_background_color_model->connect_update_signal(
        std::bind_front(&HighlightPickerModel::on_background_color, this))),
      m_text_color_connection(m_text_color_model->connect_update_signal(
        std::bind_front(&HighlightPickerModel::on_text_color, this))) {
  }

  void on_highlight_color(const HighlightSwatch::Highlight& highlight) {
    auto background_blocker = shared_connection_block(m_background_color_connection);
    m_background_color_model->set(highlight.m_background_color);
    auto text_blocker = shared_connection_block(m_text_color_connection);
    m_text_color_model->set(highlight.m_text_color);
  }

  void on_background_color(const QColor& background_color) {
    auto blocker = shared_connection_block(m_highlight_connection);
    m_highlight_model->set({background_color, m_highlight_model->get().m_text_color});
  }

  void on_text_color(const QColor& text_color) {
    auto blocker = shared_connection_block(m_highlight_connection);
    m_highlight_model->set({m_highlight_model->get().m_background_color, text_color});
  }
};

HighlightPicker::HighlightPicker(QWidget& parent)
  : HighlightPicker(std::make_shared<LocalValueModel<Highlight>>(), parent) {}

HighlightPicker::HighlightPicker(std::shared_ptr<ValueModel<Highlight>> current,
    QWidget& parent)
    : QWidget(&parent),
      m_model(std::make_shared<HighlightPickerModel>(std::move(current))) {
  auto list_model = std::make_shared<ArrayListModel<Highlight>>();
  list_model->push({"#FFF1F1", "#B71C1C"});
  list_model->push({"#FFECFF", "#76008A"});
  list_model->push({"#F4F4FF", "#4C00DA"});
  list_model->push({"#DDF9FF", "#003698"});
  list_model->push({"#EBFFF0", "#007735"});
  list_model->push({"#FFFFC4", "#834A2D"});
  list_model->push({"#FFE6C9", "#A00000"});
  list_model->push({"#FFFFFF", "#000000"});

  list_model->push({"#FFABAA", "#FFFFFF"});
  list_model->push({"#F1A6F1", "#770088"});
  list_model->push({"#C1BAFF", "#4F00D7"});
  list_model->push({"#92CFE9", "#0000C6"});
  list_model->push({"#96D8A8", "#004E00"});
  list_model->push({"#FEEE7F", "#630000"});
  list_model->push({"#FFBF76", "#A00000"});
  list_model->push({"#C6C6C6", "#000000"});

  list_model->push({"#C33A39", "#FFFFFF"});
  list_model->push({"#A937B2", "#FFFFFF"});
  list_model->push({"#6F53C5", "#FFFFFF"});
  list_model->push({"#246FBC", "#FFFFFF"});
  list_model->push({"#218542", "#FFFFFF"});
  list_model->push({"#8A6729", "#FFFFFF"});
  list_model->push({"#AE4D1F", "#FFFFFF"});
  list_model->push({"#6E6E6E", "#FFFFFF"});

  list_model->push({"#780000", "#FFFFFF"});
  list_model->push({"#5F006F", "#FFFFFF"});
  list_model->push({"#361976", "#FFFFFF"});
  list_model->push({"#21148C", "#FFFFFF"});
  list_model->push({"#004516", "#FFFFFF"});
  list_model->push({"#4D2A00", "#FFFFFF"});
  list_model->push({"#680000", "#FFFFFF"});
  list_model->push({"#000000", "#FFFFFF"});
  m_palette = new ListView(std::move(list_model), [] (const std::shared_ptr<AnyListModel>& list, int index) {
    auto current = std::make_shared<LocalValueModel<HighlightSwatch::Highlight>>(std::any_cast<Highlight>(list->get(index)));
    auto swatch = new HighlightSwatch(current);
    swatch->setFixedSize(scale(22, 18));
    return swatch;
  });
  update_style(*m_palette, [] (auto& style) {
    style.get(Any()).
      set(Overflow::WRAP).
      set(Qt::Horizontal);
    style.get(Any() > is_a<ListItem>()).
      set(border(scale_width(1), QColor(Qt::transparent))).
      set(BackgroundColor(Qt::transparent)).
      set(horizontal_padding(scale_width(1))).
      set(vertical_padding(scale_height(1)));
    style.get(Any() > (is_a<ListItem>() && (Hover() || Current()))).
      set(border_color(QColor(0x4B23A0)));
  });
  //enclose(*this, *m_palette);
  //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto layout = make_vbox_layout(this);
  layout->addWidget(m_palette);
  layout->addSpacing(scale_height(18));
  auto color_layout = make_hbox_layout();
  auto background_color_box = new ColorBox(m_model->m_background_color_model);
  background_color_box->setFixedSize(scale(100, 26));
  get_color_picker(*background_color_box)->get_body().setFixedWidth(scale_width(220));
  update_style(*background_color_box, [&] (auto& style) {
    style.get(Any() > Alpha()).set(Visibility::NONE);
  });
  color_layout->addWidget(background_color_box);
  color_layout->addSpacing(scale_width(8));
  auto text_color_box = new ColorBox(m_model->m_text_color_model);
  color_layout->addWidget(text_color_box);
  text_color_box->setFixedSize(scale(100, 26));
  get_color_picker(*text_color_box)->get_body().setFixedWidth(scale_width(220));
  update_style(*text_color_box, [&] (auto& style) {
    style.get(Any() > Alpha()).set(Visibility::NONE);
  });
  layout->addLayout(color_layout);
  m_panel = new OverlayPanel(*this, parent);
  update_style(*m_panel, [] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  on_current(get_current()->get());
  //m_current_connection = get_current()->connect_update_signal(
  //  std::bind_front(&HighlightPicker::on_current, this));
  m_palette->get_current()->connect_update_signal(
    std::bind_front(&HighlightPicker::on_palette_current, this));
}

const std::shared_ptr<ValueModel<HighlightPicker::Highlight>>&
    HighlightPicker::get_current() const {
  return m_model->m_highlight_model;
}

bool HighlightPicker::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_palette->setFixedWidth(m_palette->get_list_item(0)->sizeHint().width() * 8);
    m_panel->show();
  } else if(event->type() == QEvent::HideToParent) {
    m_panel->hide();
  }
  return QWidget::event(event);
}

void HighlightPicker::on_current(const Highlight& highlight) {
}

void HighlightPicker::on_palette_current(optional<int> current) {
  if(!current) {
    return;
  }
  get_current()->set(std::any_cast<Highlight>(m_palette->get_list()->get(*current)));
}
