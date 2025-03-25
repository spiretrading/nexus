#include "Spire/Ui/HighlightPicker.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QCoreApplication>
#include <QMouseEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/ColorBox.hpp"
#include "Spire/Ui/ColorCodePanel.hpp"
#include "Spire/Ui/ColorConversion.hpp"
#include "Spire/Ui/ColorPicker.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/OverlayPanel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto PALETTE_STYLE(StyleSheet style) {
    style.get(Any()).
      set(EdgeNavigation::CONTAIN).
      set(Overflow::WRAP).
      set(Qt::Horizontal);
    style.get(Any() > is_a<ListItem>()).
      set(border(scale_width(1), QColor(Qt::transparent))).
      set(BackgroundColor(Qt::transparent)).
      set(horizontal_padding(scale_width(1))).
      set(vertical_padding(scale_height(1)));
    style.get(Any() > Current()).
      set(BackgroundColor(QColor(0xD0D0D0)));
    style.get(FocusIn() > Current()).
      set(border_color(QColor(0x4B23A0)));
    style.get(Any() > (is_a<ListItem>() && Hover())).
      set(BackgroundColor(QColor(0xE2E0FF)));
    return style;
  }

  std::shared_ptr<ArrayListModel<HighlightPicker::Highlight>>
      DEFUALT_HIGHLIGHT_LIST() {
    static auto highlight_list =
      std::weak_ptr<ArrayListModel<HighlightPicker::Highlight>>();
    if(auto highlights = highlight_list.lock()) {
      return highlights;
    }
    auto highlights =
      std::make_shared<ArrayListModel<HighlightPicker::Highlight>>();
    highlight_list = highlights;
    highlights->push({"#FFF1F1", "#B71C1C"});
    highlights->push({"#FFECFF", "#76008A"});
    highlights->push({"#F4F4FF", "#4C00DA"});
    highlights->push({"#DDF9FF", "#003698"});
    highlights->push({"#EBFFF0", "#007735"});
    highlights->push({"#FFFFC4", "#834A2D"});
    highlights->push({"#FFE6C9", "#A00000"});
    highlights->push({"#FFFFFF", "#000000"});
    highlights->push({"#FFABAA", "#A00000"});
    highlights->push({"#F1A6F1", "#770088"});
    highlights->push({"#C1BAFF", "#4F00D7"});
    highlights->push({"#92CFE9", "#0000C6"});
    highlights->push({"#96D8A8", "#004E00"});
    highlights->push({"#FEEE7F", "#630000"});
    highlights->push({"#FFBF76", "#A00000"});
    highlights->push({"#C6C6C6", "#000000"});
    highlights->push({"#C33A39", "#FFFFFF"});
    highlights->push({"#A937B2", "#FFFFFF"});
    highlights->push({"#6F53C5", "#FFFFFF"});
    highlights->push({"#246FBC", "#FFFFFF"});
    highlights->push({"#218542", "#FFFFFF"});
    highlights->push({"#8A6729", "#FFFFFF"});
    highlights->push({"#AE4D1F", "#FFFFFF"});
    highlights->push({"#6E6E6E", "#FFFFFF"});
    highlights->push({"#780000", "#FFFFFF"});
    highlights->push({"#5F006F", "#FFFFFF"});
    highlights->push({"#361976", "#FFFFFF"});
    highlights->push({"#21148C", "#FFFFFF"});
    highlights->push({"#004516", "#FFFFFF"});
    highlights->push({"#4D2A00", "#FFFFFF"});
    highlights->push({"#680000", "#FFFFFF"});
    highlights->push({"#000000", "#FFFFFF"});
    return highlights;
  }

  auto make_highlight_swatch(const std::shared_ptr<AnyListModel>& list,
      int index) {
    auto swatch = new HighlightSwatch(
      std::make_shared<LocalValueModel<HighlightPicker::Highlight>>(
        std::any_cast<HighlightPicker::Highlight>(list->get(index))));
    swatch->setFixedSize(scale(22, 18));
    return swatch;
  }

  auto make_color_box(std::shared_ptr<ColorModel> model) {
    auto color_box = new ColorBox(std::move(model));
    color_box->setFixedHeight(scale_height(26));
    return color_box;
  }
}

struct HighlightPicker::HighlightPickerModel {
  std::shared_ptr<ValueModel<HighlightSwatch::Highlight>> m_highlight_model;
  std::shared_ptr<ColorModel> m_background_color_model;
  std::shared_ptr<ColorModel> m_text_color_model;
  scoped_connection m_highlight_connection;
  scoped_connection m_background_color_connection;
  scoped_connection m_text_color_connection;

  explicit HighlightPickerModel(
    std::shared_ptr<ValueModel<HighlightSwatch::Highlight>> highlight_model)
    : m_highlight_model(std::move(highlight_model)),
      m_background_color_model(std::make_shared<LocalColorModel>(
        m_highlight_model->get().m_background_color)),
      m_text_color_model(std::make_shared<LocalColorModel>(
        m_highlight_model->get().m_text_color)),
      m_highlight_connection(m_highlight_model->connect_update_signal(
        std::bind_front(&HighlightPickerModel::on_highlight_color, this))),
      m_background_color_connection(
        m_background_color_model->connect_update_signal(
          std::bind_front(&HighlightPickerModel::on_background_color, this))),
      m_text_color_connection(m_text_color_model->connect_update_signal(
        std::bind_front(&HighlightPickerModel::on_text_color, this))) {}

  void on_highlight_color(const HighlightPicker::Highlight& highlight) {
    auto background_blocker =
      shared_connection_block(m_background_color_connection);
    m_background_color_model->set(highlight.m_background_color);
    auto text_blocker = shared_connection_block(m_text_color_connection);
    m_text_color_model->set(highlight.m_text_color);
  }

  void on_background_color(const QColor& background_color) {
    auto background_lch_color = to_oklch(background_color);
    auto lightness = [&] {
      if(background_lch_color.m_l > 0.5) {
        return 0.3;
      }
      return 0.5;
    }();
    auto chroma = [&] {
      if(background_lch_color.m_c > 0.19) {
        return 0.1;
      }
      return 0.37;
    }();
    auto text_color_candidate =
      OklchColor(lightness, chroma, background_lch_color.m_h);
    auto text_color = [&] {
      auto candidate_contrast = apca(to_rgb(text_color_candidate),
        background_color);
      if(std::abs(candidate_contrast) > 75 && background_lch_color.m_c > 0.01) {
        return to_rgb(text_color_candidate);
      }
      return apca_text_color(background_color);
    }();
    auto highlight_blocker = shared_connection_block(m_highlight_connection);
    m_highlight_model->set({background_color, text_color});
    auto text_blocker = shared_connection_block(m_text_color_connection);
    m_text_color_model->set(text_color);
  }

  void on_text_color(const QColor& text_color) {
    auto blocker = shared_connection_block(m_highlight_connection);
    m_highlight_model->set(
      {m_highlight_model->get().m_background_color, text_color});
  }
};

HighlightPicker::HighlightPicker(QWidget& parent)
  : HighlightPicker(std::make_shared<LocalValueModel<Highlight>>(), parent) {}

HighlightPicker::HighlightPicker(std::shared_ptr<ValueModel<Highlight>> current,
    QWidget& parent)
    : QWidget(&parent),
      m_model(std::make_shared<HighlightPickerModel>(std::move(current))),
      m_background_color_picker(nullptr),
      m_text_color_picker(nullptr) {
  m_palette = new ListView(DEFUALT_HIGHLIGHT_LIST(), make_highlight_swatch);
  update_style(*m_palette, [] (auto& style) {
    style = PALETTE_STYLE(style);
  });
  auto vbox_layout = make_vbox_layout();
  vbox_layout->addWidget(m_palette);
  vbox_layout->addSpacing(scale_height(18));
  auto color_layout = make_hbox_layout();
  m_background_color_box = make_color_box(m_model->m_background_color_model);
  m_background_color_box->installEventFilter(this);
  color_layout->addWidget(m_background_color_box);
  color_layout->addSpacing(scale_width(8));
  m_text_color_box = make_color_box(m_model->m_text_color_model);
  m_text_color_box->installEventFilter(this);
  color_layout->addWidget(m_text_color_box);
  vbox_layout->addLayout(color_layout);
  auto layout = make_hbox_layout(this);
  layout->addLayout(vbox_layout);
  layout->addStretch();
  m_panel = new OverlayPanel(*this, parent);
  m_panel->setWindowFlags(Qt::Popup | (m_panel->windowFlags() & ~Qt::Tool));
  m_panel->installEventFilter(this);
  update_style(*m_panel, [] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  m_palette->get_current()->connect_update_signal(
    std::bind_front(&HighlightPicker::on_palette_current, this));
  m_palette->setFixedWidth(
    m_palette->get_list_item(0)->sizeHint().width() * 8);
}

const std::shared_ptr<ValueModel<HighlightPicker::Highlight>>&
    HighlightPicker::get_current() const {
  return m_model->m_highlight_model;
}

bool HighlightPicker::eventFilter(QObject* watched, QEvent* event) {
  if(m_panel == watched) {
    if(event->type() == QEvent::Close) {
      m_panel->hide();
      hide();
    }
  } else if(event->type() == QEvent::MouseButtonPress) {
    auto& mouse_event = *static_cast<QMouseEvent*>(event);
    if(m_background_color_picker == watched) {
      if(on_mouse_press(*m_background_color_box, *m_text_color_box,
          *m_background_color_picker, mouse_event)) {
        return true;
      }
    } else if(m_text_color_picker == watched) {
      if(on_mouse_press(*m_text_color_box, *m_background_color_box,
          *m_text_color_picker, mouse_event)) {
        return true;
      }
    }
  } else if(event->type() == QEvent::ChildAdded &&
      (watched == m_background_color_box || watched == m_text_color_box)) {
    auto& child_event = *static_cast<QChildEvent*>(event);
    if(child_event.child()->isWidgetType()) {
      if(auto& widget = *static_cast<QWidget*>(child_event.child());
          widget.windowFlags() & Qt::Popup) {
        widget.installEventFilter(this);
        watched->removeEventFilter(this);
        if(watched == m_background_color_box) {
          m_background_color_picker = &widget;
        } else {
          m_text_color_picker = &widget;
        }
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

bool HighlightPicker::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_panel->show();
  } else if(event->type() == QEvent::HideToParent) {
    m_panel->hide();
  }
  return QWidget::event(event);
}

bool HighlightPicker::on_mouse_press(ColorBox& source, ColorBox& destination,
    QWidget& source_picker, const QMouseEvent& mouse_event) {
  auto post_mouse_event = [&] (QWidget& widget, const QPoint& position) {
    auto event = new QMouseEvent(mouse_event.type(), position,
      mouse_event.windowPos(), mouse_event.screenPos(), mouse_event.button(),
      mouse_event.buttons(), mouse_event.modifiers(), mouse_event.source());
    QCoreApplication::postEvent(&widget, event);
  };
  auto update_source_border_color = [&] {
    update_style(*source.layout()->itemAt(0)->widget(), [] (auto& style) {
      style.get(Any()).set(border_color(QColor(0xC8C8C8)));
    });
  };
  if(!source_picker.rect().contains(mouse_event.pos())) {
    auto position_in_destination =
      destination.mapFromGlobal(mouse_event.globalPos());
    if(destination.rect().contains(position_in_destination)) {
      destination.setFocus(Qt::MouseFocusReason);
      post_mouse_event(destination, position_in_destination);
      update_source_border_color();
      return true;
    } else if(m_palette->rect().contains(
        m_palette->mapFromGlobal(mouse_event.globalPos()))) {
      m_palette->setFocus(Qt::MouseFocusReason);
      for(auto i = 0; i < m_palette->get_list()->get_size(); ++i) {
        auto& item = m_palette->get_list_item(i)->get_body();
        auto position_in_item = item.mapFromGlobal(mouse_event.globalPos());
        if(item.rect().contains(position_in_item)) {
          post_mouse_event(item, position_in_item);
          break;
        }
      }
      update_source_border_color();
      return true;
    } else if(m_panel->rect().contains(
        m_panel->mapFromGlobal(mouse_event.globalPos()))) {
      update_source_border_color();
    } else {
      hide();
    }
  }
  return false;
}

void HighlightPicker::on_palette_current(optional<int> current) {
  if(current) {
    get_current()->set(
      std::any_cast<Highlight>(m_palette->get_list()->get(*current)));
  }
}
