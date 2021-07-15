#include "Spire/Ui/SearchBox.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
      set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(vertical_padding(scale_height(4))).
      set(horizontal_padding(scale_width(5)));
    style.get(Hover() || Focus()).
      set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    style.get(Disabled()).
      set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
      set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    style.get(Any() >> (is_a<Icon>() && !(+Any() << is_a<Button>()))).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(Fill(QColor::fromRgb(0xA0, 0xA0, 0xA0)));
    style.get(Disabled() >> (is_a<Icon>() && !(+Any() << is_a<Button>()))).
      set(Fill(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(12));
    style.get(Any() >> is_a<TextBox>()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_size(0)).
      set(text_style(font, QColor::fromRgb(0, 0, 0))).
      set(TextAlign(Qt::Alignment(Qt::AlignLeft) | Qt::AlignVCenter)).
      set(vertical_padding(0)).
      set(horizontal_padding(scale_width(5)));
    return style;
  }
}

SearchBox::SearchBox(QWidget* parent)
    : QWidget(parent) {
  auto container = new QWidget(this);
  auto container_layout = new QHBoxLayout(container);
  container_layout->setContentsMargins({});
  container_layout->setSpacing(0);
  auto search_icon = new Icon(
    imageFromSvg(":/Icons/magnifying-glass.svg", scale(16, 16)), this);
  search_icon->setFixedSize(scale(16, 16));
  container_layout->addWidget(search_icon);
  m_text_box = new TextBox(this);
  container->setFocusProxy(m_text_box);
  m_text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  container_layout->addWidget(m_text_box);
  m_delete_button = make_delete_icon_button(this);
  m_delete_button->setFixedSize(scale(16, 16));
  m_delete_button->setFocusPolicy(Qt::NoFocus);
  m_delete_button->hide();
  container_layout->addWidget(m_delete_button);
  auto box = new Box(container, this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(box);
  proxy_style(*this, *box);
  set_style(*this, DEFAULT_STYLE());
  m_current_connection = m_text_box->get_model()->connect_current_signal(
    [=] (const auto& current) {
      m_delete_button->setVisible(!current.isEmpty());
    });
  m_delete_button->connect_clicked_signal([=] {
    m_text_box->get_model()->set_current({});
  });
}

const std::shared_ptr<TextModel>& SearchBox::get_model() const {
  return m_text_box->get_model();
}
