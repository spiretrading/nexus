#include "Spire/KeyBindings/TaskKeysPage.hpp"
#include "Spire/KeyBindings/TaskKeysTableView.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/SearchBox.hpp"
#include "Spire/Ui/TextAreaBox.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_help_text_box() {
    auto help_text_box = make_text_area_label(
      QObject::tr("Allowed keys are: <b>F1–F12</b> and <b>Ctrl, Shift, "
        "Alt  +  F1–F12</b> and <b>Ctrl, Shift, Alt  +  0–9</b>"));
    help_text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    update_style(*help_text_box, [] (auto& style) {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Normal);
      font.setPixelSize(scale_width(10));
      style.get(Any()).
        set(BackgroundColor(QColor(0xFFFFFF))).
        set(text_style(font, QColor(0x808080))).
        set(horizontal_padding(scale_width(8))).
        set(PaddingTop(scale_height(10))).
        set(PaddingBottom(scale_height(5)));
    });
    return help_text_box;
  }

  auto make_button(const QString& path, const QString& tooltip) {
    auto button = make_icon_button(imageFromSvg(path, scale(16, 16)), tooltip);
    button->setFixedSize(scale(26, 26));
    return button;
  }
}

TaskKeysPage::TaskKeysPage(std::shared_ptr<KeyBindingsModel> key_bindings,
    DestinationDatabase destinations, MarketDatabase markets, QWidget* parent)
    : QWidget(parent),
      m_key_bindings(std::move(key_bindings)) {
  auto toolbar_body = new QWidget();
  auto toolbar_layout = make_hbox_layout(toolbar_body);
  auto search_box = new SearchBox();
  search_box->set_placeholder(tr("Search tasks"));
  search_box->setFixedWidth(scale_width(368));
  toolbar_layout->addWidget(search_box);
  toolbar_layout->addStretch();
  toolbar_layout->addSpacing(scale_width(18));
  toolbar_layout->addWidget(
    make_button(":/Icons/add.svg", tr("Add Task (Shift + Enter)")));
  toolbar_layout->addSpacing(scale_width(4));
  toolbar_layout->addWidget(
    make_button(":/Icons/duplicate.svg", tr("Duplicate (Ctrl + D)")));
  toolbar_layout->addSpacing(scale_width(4));
  auto delete_button =
    make_button(":/Icons/delete3.svg", tr("Delete (Del)"));
  update_style(*delete_button, [] (auto& style) {
    style.get((Hover() || Press()) > Body() > is_a<Icon>()).
      set(Fill(QColor(0xB71C1C)));
  });
  toolbar_layout->addWidget(delete_button);
  auto toolbar = new Box(toolbar_body);
  update_style(*toolbar, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(horizontal_padding(scale_width(8))).
      set(PaddingTop(scale_height(5))).
      set(PaddingBottom(scale_height(10))).
      set(BorderBottomSize(scale_height(1))).
      set(BorderBottomColor(QColor(0xE0E0E0)));
  });
  auto layout = make_vbox_layout(this);
  layout->addWidget(make_help_text_box());
  layout->addWidget(toolbar);
  layout->addWidget(make_task_keys_table_view(
    m_key_bindings->get_order_task_arguments(),
    std::make_shared<LocalComboBoxQueryModel>(), std::move(destinations),
    std::move(markets)));
}

const std::shared_ptr<KeyBindingsModel>&
    TaskKeysPage::get_key_bindings() const {
  return m_key_bindings;
}
