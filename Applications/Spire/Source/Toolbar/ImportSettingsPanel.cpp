#include "Spire/Toolbar/ImportSettingsPanel.hpp"
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

ImportSettingsPanel::ImportSettingsPanel(QWidget& parent)
  : ImportSettingsPanel(std::make_shared<LocalSettingsModel>(), parent) {}

ImportSettingsPanel::ImportSettingsPanel(
    std::shared_ptr<SettingsModel> settings, QWidget& parent)
    : QWidget(&parent),
      m_settings(std::move(settings)) {
  auto heading = make_label(tr("Import Settings"));
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
  auto settings_list = std::make_shared<ArrayListModel<Setting>>();
  auto& settings_bitset = m_settings->get().GetBitset();
  for(auto i = 0; i < static_cast<int>(settings_bitset.size()); ++i) {
    settings_list->push(static_cast<Setting>(i));
  }
  auto settings_view = new ListView(settings_list,
    [=] (const std::shared_ptr<ListModel<Setting>>& list, auto index) {
      auto check_box = new CheckBox(
        std::make_shared<EnumSetTestModel<Settings>>(
          m_settings, list->get(index)));
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
    std::bind_front(&ImportSettingsPanel::on_cancel, this));
  m_import_button = make_label_button(tr("Import"));
  m_import_button->setAttribute(Qt::WA_NoMousePropagation);
  m_import_button->setFixedSize(scale(100, 26));
  m_import_button->connect_click_signal(
    std::bind_front(&ImportSettingsPanel::on_import, this));
  auto actions_body = new QWidget();
  auto actions_layout = make_hbox_layout(actions_body);
  actions_layout->setSpacing(scale_height(8));
  actions_layout->addWidget(cancel_button);
  actions_layout->addWidget(m_import_button);
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
  on_update(m_settings->get());
  m_connection = m_settings->connect_update_signal(
    std::bind_front(&ImportSettingsPanel::on_update, this));
  setFocusProxy(list_box);
}

const std::shared_ptr<SettingsModel>&
    ImportSettingsPanel::get_settings() const {
  return m_settings;
}

connection ImportSettingsPanel::connect_import_signal(
    const ImportSignal::slot_type& slot) const {
  return m_import_signal.connect(slot);
}

bool ImportSettingsPanel::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Close) {
    hide();
  }
  return QWidget::eventFilter(watched, event);
}

bool ImportSettingsPanel::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_panel->show();
    m_panel->activateWindow();
  } else if(event->type() == QEvent::HideToParent) {
    m_panel->hide();
  }
  return QWidget::event(event);
}

void ImportSettingsPanel::on_update(const Settings& settings) {
  m_import_button->setEnabled(settings.GetBitset().count() != 0);
}

void ImportSettingsPanel::on_cancel() {
  close();
}

void ImportSettingsPanel::on_import() {
  m_import_signal(m_settings->get());
  close();
}

const QString& Spire::to_text(Setting setting) {
  if(setting == Setting::BOOK_VIEW) {
    static const auto value = QObject::tr("Book View");
    return value;
  } else if(setting == Setting::IMBALANCE_INDICATOR) {
    static const auto value = QObject::tr("Imbalance Indicator");
    return value;
  } else if(setting == Setting::INTERACTIONS) {
    static const auto value = QObject::tr("Interactions");
    return value;
  } else if(setting == Setting::KEY_BINDINGS) {
    static const auto value = QObject::tr("Key Bindings");
    return value;
  } else if(setting == Setting::PORTFOLIO_VIEWER) {
    static const auto value = QObject::tr("Portfolio View");
    return value;
  } else if(setting == Setting::TIME_AND_SALES) {
    static const auto value = QObject::tr("Time and Sales");
    return value;
  } else if(setting == Setting::WATCHLIST) {
    static const auto value = QObject::tr("Watchlists");
    return value;
  } else if(setting == Setting::LAYOUT) {
    static const auto value = QObject::tr("Window Layout");
    return value;
  } else {
    static const auto value = QObject::tr("None");
    return value;
  }
}
