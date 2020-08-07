#include "Spire/Ui/FontSelectorWidget.hpp"
#include <algorithm>
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
    : QWidget(parent) {
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
  m_font_list = new DropDownMenu(
    std::vector<QString>(fonts.begin(), fonts.end()), this);
  m_font_list->setFixedSize(scale(162, 26));
  m_font_list->connect_selected_signal([=] (const auto& font) {
    on_font_selected(font);
  });
  m_font_list->connect_highlighted_signal([=] (const auto& font) {
    on_font_preview(font);
  });
  m_font_list->connect_menu_closed_signal([=] { on_font_list_closed(); });
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
  m_size_input = new IntegerInputWidget(0, this);
  m_size_input->setRange(6, 72);
  m_size_input->setFixedHeight(scale_height(26));
  m_size_input->connect_change_signal([=] (auto value) {
    on_size_selected(value);
  });
  grid_layout->addWidget(m_size_input, 1, 3, 1, 2);
  set_font(current_font);
}

const QFont& FontSelectorWidget::get_font() const {
  return m_current_font;
}

void FontSelectorWidget::set_font(const QFont& font) {
  m_current_font = font;
  m_font_list->set_current_text(m_current_font.family());
  m_size_input->setValue(m_current_font.pointSize());
  m_bold_button->set_toggled(m_current_font.bold());
  m_italics_button->set_toggled(m_current_font.italic());
  m_underline_button->set_toggled(m_current_font.underline());
}

connection FontSelectorWidget::connect_font_preview_signal(
    const FontPreviewSignal::slot_type& slot) const {
  return m_font_preview_signal.connect(slot);
}

connection FontSelectorWidget::connect_font_selected_signal(
    const FontSelectedSignal::slot_type& slot) const {
  return m_font_selected_signal.connect(slot);
}

void FontSelectorWidget::on_bold_button_clicked() {
  m_current_font.setBold(m_bold_button->is_toggled());
  m_font_selected_signal(m_current_font);
}

void FontSelectorWidget::on_italics_button_clicked() {
  m_current_font.setItalic(m_italics_button->is_toggled());
  m_font_selected_signal(m_current_font);
}

void FontSelectorWidget::on_underline_button_clicked() {
  m_current_font.setUnderline(m_underline_button->is_toggled());
  m_font_selected_signal(m_current_font);
}

void FontSelectorWidget::on_font_list_closed() {
  m_font_selected_signal(m_current_font);
}

void FontSelectorWidget::on_font_preview(const QString& family) {
  auto preview_font = m_current_font;
  preview_font.setFamily(family);
  m_font_preview_signal(preview_font);
}

void FontSelectorWidget::on_font_selected(const QString& family) {
  m_current_font.setFamily(family);
  m_font_selected_signal(m_current_font);
}

void FontSelectorWidget::on_size_selected(int size) {
  m_current_font.setPointSize(size);
  m_font_selected_signal(m_current_font);
}
