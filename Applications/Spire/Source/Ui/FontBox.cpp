#include "Spire/Ui/FontBox.hpp"
#include <QFontDatabase>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"
#include "Spire/Ui/AdaptiveBox.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto get_character_width() {
    static auto width = optional<int>();
    if(width) {
      return *width;
    }
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(12));
    width = QFontMetrics(font).averageCharWidth();
    return *width;
  }

  struct CustomFontFamilyBox : QWidget {
    explicit CustomFontFamilyBox(FontFamilyBox& family_box,
        QWidget* parent = nullptr)
        : QWidget(parent) {
      //setFocusPolicy(Qt::StrongFocus);
      //setFocusPolicy(Qt::TabFocus);
      setFocusProxy(&family_box);
      enclose(*this, family_box);
    }

    QSize sizeHint() const override {
      auto size = QWidget::sizeHint();
      return {20 * get_character_width() + scale_width(22), size.height()};
    }
  };

  struct CustomFontStyleBox : QWidget {
    explicit CustomFontStyleBox(FontStyleBox& style_box,
        QWidget* parent = nullptr)
        : QWidget(parent) {
      //setFocusPolicy(Qt::StrongFocus);
      //setFocusPolicy(Qt::TabFocus);
      setFocusProxy(&style_box);
      enclose(*this, style_box);
    }

    QSize sizeHint() const override {
      auto size = QWidget::sizeHint();
      return {16 * get_character_width() + scale_width(22), size.height()};
    }
  };
}

FontBox::FontBox(QWidget* parent)
  : FontBox(std::make_shared<LocalValueModel<QFont>>(QFont())) {}

FontBox::FontBox(std::shared_ptr<ValueModel<QFont>> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)) {
  auto adaptive_box = new AdaptiveBox();
  adaptive_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_font_family_box = make_font_family_box(m_current->get().family());
  m_font_style_box = make_font_style_box(m_font_family_box->get_current());
  auto size_model = std::make_shared<LocalScalarValueModel<optional<Decimal>>>();
  size_model->set_minimum(Decimal(1));
  size_model->set_increment(pow(Decimal(10), -2));
  size_model->set(Decimal(m_current->get().pixelSize()));
  m_font_size_box = new DecimalBox(std::move(size_model));
  m_font_size_box->setFixedWidth(6 * get_character_width() + scale_width(22));
  auto custom_family_box = new CustomFontFamilyBox(*m_font_family_box);
  custom_family_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto custom_style_box = new CustomFontFamilyBox(*m_font_style_box);
  custom_style_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto medium_layout = make_hbox_layout();
  medium_layout->addWidget(custom_family_box);
  medium_layout->addSpacing(scale_width(8));
  medium_layout->addWidget(custom_style_box);
  medium_layout->addSpacing(scale_width(8));
  medium_layout->addWidget(m_font_size_box);
  auto small_layout = make_vbox_layout();
  small_layout->addWidget(custom_family_box);
  small_layout->addSpacing(scale_height(8));
  auto horizontal_layout = make_hbox_layout();
  horizontal_layout->addWidget(custom_style_box);
  horizontal_layout->addSpacing(scale_width(8));
  horizontal_layout->addWidget(m_font_size_box);
  small_layout->addLayout(horizontal_layout);
  adaptive_box->add(*medium_layout);
  adaptive_box->add(*small_layout);
  enclose(*this, *adaptive_box);
  m_font_connection = m_current->connect_update_signal(
    std::bind_front(&FontBox::on_current, this));
  m_style_connection = m_font_style_box->get_current()->connect_update_signal(
    std::bind_front(&FontBox::on_style_current, this));
  m_size_connection = m_font_size_box->get_current()->connect_update_signal(
    std::bind_front(&FontBox::on_size_current, this));
  //auto next = nextInFocusChain();
  //auto next = [&] () -> QWidget* {
  //  auto widget = nextInFocusChain();
  //  while(widget != this) {
  //    if(!isAncestorOf(widget)) {
  //      return widget;
  //    }
  //    widget = widget->nextInFocusChain();
  //  }
  //  return this;
  //}();
  //auto previous = previousInFocusChain();
  //QWidget::setTabOrder(previous, find_focus_proxy(*m_font_family_box));
  //QWidget::setTabOrder(find_focus_proxy(*m_font_family_box), find_focus_proxy(*m_font_style_box));
  //QWidget::setTabOrder(find_focus_proxy(*m_font_style_box), find_focus_proxy(*m_font_size_box));
  //QWidget::setTabOrder(find_focus_proxy(*m_font_size_box), this);

  //auto children = std::queue<QWidget*>();
}

const std::shared_ptr<ValueModel<QFont>>& FontBox::get_current() const {
  return m_current;
}

void FontBox::on_current(const QFont& font) {
  if(font.family() != m_font_family_box->get_current()->get()) {
    auto style_blocker = shared_connection_block(m_style_connection);
    m_font_family_box->get_current()->set(font.family());
  }
  auto style = QFontDatabase().styleString(font);
  if(style != m_font_style_box->get_current()->get()) {
    auto style_blocker = shared_connection_block(m_style_connection);
    m_font_style_box->get_current()->set(style);
  }
  auto size = Decimal(unscale_width(font.pixelSize()));
  if(size != m_font_size_box->get_current()->get()) {
    auto size_blocker = shared_connection_block(m_size_connection);
    m_font_size_box->get_current()->set(size);
  }
}

void FontBox::on_style_current(const QString& style) {
  if(style.isEmpty()) {
    return;
  }
  if(m_font_family_box->get_current()->get() == m_current->get().family() &&
      style == m_current->get().styleName()) {
    return;
  }
  auto font =
    QFontDatabase().font(m_font_family_box->get_current()->get(), style, -1);
  font.setPixelSize(m_current->get().pixelSize());
  auto font_blocker = shared_connection_block(m_font_connection);
  m_current->set(font);
}

void FontBox::on_size_current(const optional<Decimal>& value) {
  if(!value) {
    return;
  }
  auto font = m_current->get();
  auto size = scale_width(static_cast<int>(*value));
  if(size == font.pixelSize()) {
    return;
  }
  font.setPixelSize(size);
  auto font_blocker = shared_connection_block(m_font_connection);
  m_current->set(font);
}
