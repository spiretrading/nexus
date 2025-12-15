#include "Spire/Ui/InfoPanel.hpp"
#include <QEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/TextAreaBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto SEVERITY_ICON_SIZE() {
    static auto size = scale(14, 14);
    return size;
  }

  const auto& INFO_IMAGE() {
    static auto image =
      image_from_svg(":/Icons/info_panel/info.svg", SEVERITY_ICON_SIZE());
    return image;
  }

  const auto& SUCCESS_IMAGE() {
    static auto image =
      image_from_svg(":/Icons/info_panel/success.svg", SEVERITY_ICON_SIZE());
    return image;
  }

  const auto& WARNING_IMAGE() {
    static auto image =
      image_from_svg(":/Icons/info_panel/warning.svg", SEVERITY_ICON_SIZE());
    return image;
  }

  const auto& ERROR_IMAGE() {
    static auto image =
      image_from_svg(":/Icons/info_panel/error.svg", SEVERITY_ICON_SIZE());
    return image;
  }

  auto SEVERITY_STYLE(InfoPanel::Severity severity) {
    auto style = StyleSheet();
    if(severity == InfoPanel::Severity::INFO) {
      style.get(Any()).set(Fill(QColor(0x4392D6)));
    } else if(severity == InfoPanel::Severity::SUCCESS) {
      style.get(Any()).set(Fill(QColor(0x26BF4A)));
    } else if(severity == InfoPanel::Severity::WARNING) {
      style.get(Any()).set(Fill(QColor(0xFFBB00)));
    } else if(severity == InfoPanel::Severity::ERROR) {
      style.get(Any()).set(Fill(QColor(0xE63F44)));
    }
    return style;
  }
}

InfoPanel::InfoPanel(QString message, QWidget& parent)
  : InfoPanel(Severity::INFO, std::move(message), parent) {}

InfoPanel::InfoPanel(Severity severity, QString message, QWidget& parent)
    : QWidget(&parent),
      m_severity(severity),
      m_message(std::move(message)) {
  setAttribute(Qt::WA_DeleteOnClose);
  auto image = [&] {
    if(m_severity == Severity::INFO) {
      return INFO_IMAGE();
    } else if(m_severity == Severity::SUCCESS) {
      return SUCCESS_IMAGE();
    } else if(m_severity == Severity::WARNING) {
      return WARNING_IMAGE();
    } else if(m_severity == Severity::ERROR) {
      return ERROR_IMAGE();
    }
    return QImage();
  }();
  auto severity_icon = new Icon(image);
  severity_icon->setFixedSize(SEVERITY_ICON_SIZE());
  set_style(*severity_icon, SEVERITY_STYLE(m_severity));
  auto message_area_layout = make_hbox_layout();
  message_area_layout->addWidget(severity_icon, 0, Qt::AlignTop);
  message_area_layout->addSpacing(scale_width(4));
  m_label = make_text_area_label(m_message);
  message_area_layout->addWidget(m_label);
  auto left_area_layout = make_vbox_layout();
  left_area_layout->addSpacing(scale_height(1));
  left_area_layout->addLayout(message_area_layout);
  auto close_button = make_icon_button(
    image_from_svg(":/Icons/info_panel/close.svg", scale(16, 16)));
  close_button->setFixedSize(scale(16, 16));
  close_button->connect_click_signal([=] { close(); });
  auto layout = make_hbox_layout(this);
  layout->addLayout(left_area_layout);
  layout->addSpacing(scale_width(8));
  layout->addWidget(close_button, 0, Qt::AlignTop);
  m_panel = new OverlayPanel(*this, parent);
  update_style(*m_panel, [] (auto& styles) {
    styles.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  m_panel->set_closed_on_focus_out(false);
  m_panel->set_is_draggable(false);
  m_panel->installEventFilter(this);
  on_message_style();
  connect_style_signal(*m_label,
    std::bind_front(&InfoPanel::on_message_style, this));
}

InfoPanel::Severity InfoPanel::get_severity() const {
  return m_severity;
}

const QString& InfoPanel::get_message() const {
  return m_message;
}

bool InfoPanel::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Close) {
    close();
  }
  return QWidget::eventFilter(watched, event);
}

bool InfoPanel::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_panel->show();
  } else if(event->type() == QEvent::HideToParent) {
    m_panel->close();
  }
  return QWidget::event(event);
}

void InfoPanel::on_message_style() {
  auto text_font = std::make_shared<QFont>();
  auto has_update = std::make_shared<bool>(false);
  auto& stylist = find_stylist(*m_label);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const Font& font) {
        stylist.evaluate(font, [=] (auto font) {
          *text_font = font;
          *has_update = true;
        });
      });
  }
  if(*has_update) {
    m_label->setFixedWidth(45 * get_character_width(*text_font));
  }
}
