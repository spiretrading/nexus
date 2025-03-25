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
  auto DROP_DOWN_BOX_HORIZONTAL_PADDING() {
    static auto padding = scale_width(22);
    return padding;
  }

  auto INTEGER_BOX_HORIZONTAL_PADDING() {
    static auto padding = scale_width(32);
    return padding;
  }

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
      enclose(*this, family_box);
    }

    QSize sizeHint() const override {
      return {14 * get_character_width() + DROP_DOWN_BOX_HORIZONTAL_PADDING(),
        QWidget::sizeHint().height()};
    }
  };

  struct CustomFontStyleBox : QWidget {
    explicit CustomFontStyleBox(FontStyleBox& style_box,
        QWidget* parent = nullptr)
        : QWidget(parent) {
      enclose(*this, style_box);
    }

    QSize sizeHint() const override {
      return {12 * get_character_width() + DROP_DOWN_BOX_HORIZONTAL_PADDING(),
        QWidget::sizeHint().height()};
    }
  };
}

FontBox::FontBox(QWidget* parent)
  : FontBox(std::make_shared<LocalValueModel<QFont>>(QFont())) {}

FontBox::FontBox(std::shared_ptr<ValueModel<QFont>> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)) {
  m_font_family_box = make_font_family_box(m_current->get().family());
  m_font_style_box = make_font_style_box(m_font_family_box->get_current(),
    std::make_shared<LocalValueModel<QString>>(
      QFontDatabase().styleString(m_current->get())));
  auto size_model = std::make_shared<LocalScalarValueModel<optional<int>>>();
  size_model->set_minimum(1);
  auto font_size = [&] {
    if(m_current->get().pixelSize() < 1) {
      return 1;
    }
    return unscale_width(m_current->get().pixelSize());
  }();
  size_model->set(font_size);
  m_font_size_box = new IntegerBox(std::move(size_model));
  m_font_size_box->setFixedWidth(
    6 * get_character_width() + INTEGER_BOX_HORIZONTAL_PADDING());
  auto custom_family_box = new CustomFontFamilyBox(*m_font_family_box);
  custom_family_box->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  auto custom_style_box = new CustomFontStyleBox(*m_font_style_box);
  custom_style_box->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
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
  auto adaptive_box = new AdaptiveBox();
  adaptive_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  adaptive_box->add(*medium_layout);
  adaptive_box->add(*small_layout);
  enclose(*this, *adaptive_box);
  m_font_connection = m_current->connect_update_signal(
    std::bind_front(&FontBox::on_current, this));
  m_style_connection = m_font_style_box->get_current()->connect_update_signal(
    std::bind_front(&FontBox::on_style_current, this));
  m_size_connection = m_font_size_box->get_current()->connect_update_signal(
    std::bind_front(&FontBox::on_size_current, this));
}

const std::shared_ptr<ValueModel<QFont>>& FontBox::get_current() const {
  return m_current;
}

void FontBox::on_current(const QFont& font) {
  if(font.family() != m_font_family_box->get_current()->get()) {
    auto blocker = shared_connection_block(m_style_connection);
    m_font_family_box->get_current()->set(font.family());
  }
  auto style = QFontDatabase().styleString(font);
  if(style != m_font_style_box->get_current()->get()) {
    auto blocker = shared_connection_block(m_style_connection);
    m_font_style_box->get_current()->set(style);
  }
  auto size = unscale_width(font.pixelSize());
  if(size != m_font_size_box->get_current()->get()) {
    auto blocker = shared_connection_block(m_size_connection);
    m_font_size_box->get_current()->set(size);
  }
}

void FontBox::on_style_current(const QString& style) {
  if(style.isEmpty()) {
    return;
  }
  auto font_database = QFontDatabase();
  auto& current_font = m_current->get();
  if(m_font_family_box->get_current()->get() == current_font.family() &&
      style == font_database.styleString(current_font)) {
    return;
  }
  auto font =
    font_database.font(m_font_family_box->get_current()->get(), style, -1);
  if(font_database.isScalable(m_font_family_box->get_current()->get(), style)) {
    if(auto size = m_font_size_box->get_current()->get()) {
      font.setPixelSize(scale_width(*size));
    }
    m_font_size_box->setEnabled(true);
  } else {
    m_font_size_box->setEnabled(false);
  }
  auto blocker = shared_connection_block(m_font_connection);
  m_current->set(font);
}

void FontBox::on_size_current(const optional<int>& value) {
  if(!value) {
    return;
  }
  auto current_font = m_current->get();
  auto size = scale_width(*value);
  if(size == current_font.pixelSize()) {
    return;
  }
  current_font.setPixelSize(size);
  auto blocker = shared_connection_block(m_font_connection);
  m_current->set(current_font);
}
