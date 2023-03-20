#include "Spire/Ui/SearchBox.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border(scale_width(1), QColor(0xC8C8C8))).
      set(vertical_padding(scale_height(4))).
      set(horizontal_padding(scale_width(5)));
    style.get(Hover() || Focus()).
      set(border_color(QColor(0x4B23A0)));
    style.get(Disabled()).
      set(BackgroundColor(QColor(0xF5F5F5))).
      set(border_color(QColor(0xC8C8C8)));
    style.get(Any() > (is_a<Icon>() && !(+Any() << is_a<Button>()))).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(Fill(QColor(0xA0A0A0)));
    style.get(Disabled() > (is_a<Icon>() && !(+Any() << is_a<Button>()))).
      set(Fill(QColor(0xC8C8C8)));
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(12));
    style.get(Any() > is_a<TextBox>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_size(0)).
      set(text_style(font, QColor(Qt::black))).
      set(TextAlign(Qt::Alignment(Qt::AlignLeft) | Qt::AlignVCenter)).
      set(vertical_padding(0)).
      set(horizontal_padding(scale_width(5)));
    style.get(Disabled() > is_a<TextBox>()).
      set(TextColor(QColor(0xC8C8C8)));
    return style;
  }
}

SearchBox::SearchBox(QWidget* parent)
  : SearchBox(std::make_shared<LocalTextModel>(), parent) {}

SearchBox::SearchBox(std::shared_ptr<TextModel> model, QWidget* parent)
    : QWidget(parent) {
  auto container = new QWidget(this);
  auto search_icon = new Icon(
    imageFromSvg(":/Icons/magnifying-glass.svg", scale(16, 16)), this);
  search_icon->setFixedSize(scale(16, 16));
  auto container_layout = make_hbox_layout(container);
  container_layout->addWidget(search_icon);
  m_text_box = new TextBox(std::move(model), this);
  container->setFocusProxy(m_text_box);
  m_text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  container_layout->addWidget(m_text_box);
  m_delete_button = make_delete_icon_button(this);
  m_delete_button->setFixedSize(scale(16, 16));
  m_delete_button->setFocusPolicy(Qt::NoFocus);
  m_delete_button->hide();
  container_layout->addWidget(m_delete_button);
  auto box = new Box(container);
  enclose(*this, *box);
  setFocusProxy(box);
  proxy_style(*this, *box);
  set_style(*this, DEFAULT_STYLE());
  m_current_connection = m_text_box->get_current()->connect_update_signal(
    [=] (const auto& current) { on_current(current); });
  m_delete_button->connect_click_signal([=] { on_delete_button(); });
}

const std::shared_ptr<TextModel>& SearchBox::get_current() const {
  return m_text_box->get_current();
}

void SearchBox::set_placeholder(const QString& text) {
  m_text_box->set_placeholder(text);
}

connection SearchBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_text_box->connect_submit_signal(slot);
}

void SearchBox::on_current(const QString& current) {
  m_delete_button->setVisible(!current.isEmpty());
}

void SearchBox::on_delete_button() {
  m_text_box->get_current()->set({});
}
