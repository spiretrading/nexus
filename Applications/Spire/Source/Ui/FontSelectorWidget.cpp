#include "Spire/Ui/FontSelectorWidget.hpp"
#include <vector>
#include <QFontDatabase>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto BUTTON_SIZE() {
    static auto button_size = scale(26, 26);
    return button_size;
  }

   auto HORIZONTAL_SPACING() {
    static auto spacing = scale_width(8);
    return spacing;
  }

  auto VERTICAL_SPACING() {
    static auto spacing = scale_height(8);
    return spacing;
  }
}

FontSelectorWidget::FontSelectorWidget(const QFont& current_font,
    QWidget* parent)
    : QWidget(parent),
      m_current_font(current_font) {
  setFixedSize(scale(162, 78));
  setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(scale_height(4));
  auto typeface_label = new QLabel(tr("Typeface"), this);
  typeface_label->setStyleSheet(QString(R"(
    QLabel {
      font-family: Roboto;
      font-size: %1px;
    })").arg(scale_height(12)));
  typeface_label->setFixedHeight(scale_height(14));
  layout->addWidget(typeface_label);
  auto grid_layout = new QGridLayout();
  grid_layout->setHorizontalSpacing(HORIZONTAL_SPACING());
  grid_layout->setVerticalSpacing(VERTICAL_SPACING());
  layout->addLayout(grid_layout);
  auto fonts = QFontDatabase().families();
  auto font_list = [&] {
    auto list = std::vector<QString>();
    for(auto& font : fonts) {
      list.push_back(font);
    }
    return list;
  }();
  m_font_list = new DropDownMenu(font_list, this);
  m_font_list->set_current_text(m_current_font.family());
  m_font_list->setFixedSize(scale(162, 26));
  m_font_list->connect_selected_signal([=] (const auto& font) {
    on_font_selected(font);
  });
  grid_layout->addWidget(m_font_list, 0, 0, 1, 5);
  m_bold_button = new FontSelectorButton(
    imageFromSvg(":/Icons/text_effects/bold-inactive.svg", BUTTON_SIZE()),
    imageFromSvg(":/Icons/text_effects/bold-active.svg", BUTTON_SIZE()),
    imageFromSvg(":/Icons/text_effects/bold-hover.svg", BUTTON_SIZE()), this);
  m_bold_button->setFixedSize(BUTTON_SIZE());
  m_bold_button->connect_clicked_signal([=] { on_bold_button_clicked(); });
  grid_layout->addWidget(m_bold_button, 1, 0);
  m_italics_button = new FontSelectorButton(
    imageFromSvg(":/Icons/text_effects/italic-inactive.svg", BUTTON_SIZE()),
    imageFromSvg(":/Icons/text_effects/italic-active.svg", BUTTON_SIZE()),
    imageFromSvg(":/Icons/text_effects/italic-hover.svg", BUTTON_SIZE()),
    this);
  m_italics_button->setFixedSize(BUTTON_SIZE());
  m_italics_button->connect_clicked_signal(
    [=] { on_italics_button_clicked(); });
  grid_layout->addWidget(m_italics_button, 1, 1);
  m_underline_button = new FontSelectorButton(
    imageFromSvg(":/Icons/text_effects/underline-inactive.svg", BUTTON_SIZE()),
    imageFromSvg(":/Icons/text_effects/underline-active.svg", BUTTON_SIZE()),
    imageFromSvg(":/Icons/text_effects/underline-hover.svg", BUTTON_SIZE()),
    this);
  m_underline_button->setFixedSize(BUTTON_SIZE());
  m_underline_button->connect_clicked_signal(
    [=] { on_underline_button_clicked(); });
  grid_layout->addWidget(m_underline_button, 1, 2);
  m_size_list = new DropDownMenu({"6", "7", "8", "9", "10", "11", "12", "14",
    "16", "18", "20", "22", "24", "26", "28", "36", "48", "72"}, this);
  m_size_list->set_current_text(QString::number(current_font.pointSize()));
  m_size_list->setFixedHeight(scale_height(26));
  m_size_list->connect_selected_signal([=] (const auto& size) {
    on_size_selected(size);
  });
  grid_layout->addWidget(m_size_list, 1, 3, 1, 2);
}

const QFont& FontSelectorWidget::get_font() const {
  return m_current_font;
}

connection FontSelectorWidget::connect_font_signal(
    const FontSignal::slot_type& slot) const {
  return m_font_signal.connect(slot);
}

void FontSelectorWidget::on_bold_button_clicked() {
  m_current_font.setBold(m_bold_button->is_toggled());
  m_font_signal(m_current_font);
}

void FontSelectorWidget::on_italics_button_clicked() {
  m_current_font.setItalic(m_italics_button->is_toggled());
  m_font_signal(m_current_font);
}

void FontSelectorWidget::on_underline_button_clicked() {
  m_current_font.setUnderline(m_underline_button->is_toggled());
  m_font_signal(m_current_font);
}

void FontSelectorWidget::on_font_selected(const QString& family) {
  m_current_font.setFamily(family);
  m_font_signal(m_current_font);
}

void FontSelectorWidget::on_size_selected(const QString& size) {
  m_current_font.setPointSize(size.toInt());
  m_font_signal(m_current_font);
}
