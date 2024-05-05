#include "Spire/Toolbar/SettingsPanel.hpp"
#include <QEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/EnumSetTestModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

SettingsPanel::SettingsPanel(Mode mode, QWidget& parent)
  : SettingsPanel(
      mode, std::make_shared<UserSettings::LocalCategoriesModel>(), parent) {}

SettingsPanel::SettingsPanel(Mode mode,
    std::shared_ptr<UserSettings::CategoriesModel> categories, QWidget& parent)
    : QWidget(&parent),
      m_categories(std::move(categories)) {
  auto header = [&] {
    if(mode == Mode::IMPORT) {
      return tr("Import Settings");
    }
    return tr("Export Settings");
  }();
  auto heading = make_label(std::move(header));
  heading->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  update_style(*heading, [] (auto& styles) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(12));
    styles.get(Any()).
      set(text_style(font, QColor(0x808080))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(10)));
  });
  auto settings_list =
    std::make_shared<ArrayListModel<UserSettings::Category>>();
  auto& settings_bitset = m_categories->get().GetBitset();
  for(auto i = 0; i < static_cast<int>(settings_bitset.size()); ++i) {
    settings_list->push(static_cast<UserSettings::Category>(i));
  }
  auto settings_view = new ListView(settings_list,
    [=] (const std::shared_ptr<ListModel<UserSettings::Category>>& list,
        auto index) {
      auto check_box = new CheckBox(
        std::make_shared<EnumSetTestModel<UserSettings::Categories>>(
          m_categories, list->get(index)));
      check_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      check_box->set_label(to_text(list->get(index)));
      return check_box;
    });
  for(auto i = 0; i < settings_list->get_size(); ++i) {
    update_style(*settings_view->get_list_item(i), [] (auto& styles) {
      styles.get(Any()).
        set(border_size(0)).
        set(horizontal_padding(scale_width(8))).
        set(vertical_padding(scale_height(5)));
    });
  }
  auto list_box = new ScrollableListBox(*settings_view);
  list_box->setAttribute(Qt::WA_NoMousePropagation);
  list_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto field_set = new Box(list_box);
  update_style(*field_set, [] (auto& styles) {
    styles.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(10)));
  });
  auto cancel_button = make_label_button(tr("Cancel"));
  cancel_button->setAttribute(Qt::WA_NoMousePropagation);
  cancel_button->setFixedSize(scale(100, 26));
  cancel_button->connect_click_signal(
    std::bind_front(&SettingsPanel::on_cancel, this));
  auto commit_label = [&] {
    if(mode == Mode::IMPORT) {
      return tr("Import");
    }
    return tr("Export");
  }();
  m_commit_button = make_label_button(std::move(commit_label));
  m_commit_button->setAttribute(Qt::WA_NoMousePropagation);
  m_commit_button->setFixedSize(scale(100, 26));
  m_commit_button->connect_click_signal(
    std::bind_front(&SettingsPanel::on_commit, this));
  auto actions_body = new QWidget();
  auto actions_layout = make_hbox_layout(actions_body);
  actions_layout->setSpacing(scale_height(8));
  actions_layout->addWidget(cancel_button);
  actions_layout->addWidget(m_commit_button);
  auto actions_box = new Box(actions_body);
  update_style(*actions_box, [] (auto& styles) {
    styles.get(Any()).
      set(PaddingTop(scale_height(10))).
      set(PaddingBottom(scale_height(8))).
      set(horizontal_padding(scale_width(8)));
  });
  auto layout = make_vbox_layout(this);
  layout->addWidget(heading);
  layout->addWidget(field_set);
  layout->addWidget(actions_box);
  m_panel = new OverlayPanel(*this, parent);
  m_panel->setWindowModality(Qt::WindowModal);
  m_panel->set_positioning(OverlayPanel::Positioning::NONE);
  m_panel->set_closed_on_focus_out(false);
  m_panel->set_is_draggable(true);
  m_panel->installEventFilter(this);
  on_update(m_categories->get());
  m_connection = m_categories->connect_update_signal(
    std::bind_front(&SettingsPanel::on_update, this));
  setFocusProxy(list_box);
}

const std::shared_ptr<UserSettings::CategoriesModel>&
    SettingsPanel::get_categories() const {
  return m_categories;
}

connection SettingsPanel::connect_commit_signal(
    const CommitSignal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

bool SettingsPanel::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Close) {
    hide();
  }
  return QWidget::eventFilter(watched, event);
}

bool SettingsPanel::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_panel->show();
    m_panel->activateWindow();
  } else if(event->type() == QEvent::HideToParent) {
    m_panel->hide();
  }
  return QWidget::event(event);
}

void SettingsPanel::on_update(const UserSettings::Categories& categories) {
  m_commit_button->setEnabled(categories.GetBitset().count() != 0);
}

void SettingsPanel::on_cancel() {
  close();
}

void SettingsPanel::on_commit() {
  m_commit_signal(m_categories->get());
  close();
}
