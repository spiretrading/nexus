#include "Spire/Ui/SecurityView.hpp"
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

class SecurityView::SecuritySearchWindow : public QWidget {
  public:
    explicit SecuritySearchWindow(
        std::shared_ptr<ComboBox::QueryModel> query_model,
        QWidget* parent = nullptr)
        : QWidget(parent) {
      auto header = make_label(tr("Security"));
      update_style(*header, [] (auto& style) {
        auto font = QFont("Roboto");
        font.setWeight(QFont::Medium);
        font.setPixelSize(scale_width(12));
        style.get(ReadOnly() && Disabled()).
          set(text_style(font, QColor(0x808080))).
          set(PaddingBottom(scale_height(8)));
      });
      m_security_box = new SecurityBox(std::move(query_model));
      update_style(*m_security_box, [] (auto& style) {
        style.get(Any() > (is_a<TextBox>() && !(+Any() << is_a<ListItem>()))).
          set(vertical_padding(scale_height(7)));
      });
      auto layout = make_vbox_layout(this);
      layout->addWidget(header);
      layout->addWidget(m_security_box);
      setFocusProxy(m_security_box);
      setFixedWidth(scale_width(180));
      m_panel = new OverlayPanel(*this, *parent);
      update_style(*m_panel, [] (auto& style) {
        style.get(Any()).
          set(horizontal_padding(scale_width(8))).
          set(vertical_padding(scale_height(8)));
      });
      m_panel->set_positioning(OverlayPanel::Positioning::NONE);
      m_panel->installEventFilter(this);
      m_input_box = find_focus_proxy(*m_security_box);
      m_input_box->installEventFilter(this);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(watched == m_panel && event->type() == QEvent::Close) {
        hide();
      } else if(watched == m_input_box && event->type() == QEvent::KeyPress) {
        auto& key_event = static_cast<QKeyEvent&>(*event);
        if(key_event.key() == Qt::Key_Escape) {
          QApplication::sendEvent(m_panel, event);
          return true;
        }
      }
      return QWidget::eventFilter(watched, event);
    }

    bool event(QEvent* event) override {
      if(event->type() == QEvent::ShowToParent) {
        m_panel->show();
        auto parent_size = m_panel->parentWidget()->size();
        auto x = (parent_size.width() - m_panel->width()) / 2;
        auto y = (parent_size.height() - m_panel->height()) / 2;
        m_panel->move(m_panel->parentWidget()->mapToGlobal({x, y}));
        m_panel->activateWindow();
      } else if(event->type() == QEvent::HideToParent) {
        m_security_box->get_current()->set(Security());
        setFocus();
        m_panel->close();
      }
      return QWidget::event(event);
    }

    const std::shared_ptr<ComboBox::QueryModel>& get_query_model() const {
      return m_security_box->get_query_model();
    }

    connection connect_submit_signal(
        const SecurityBox::SubmitSignal::slot_type& slot) const {
      return m_security_box->connect_submit_signal(slot);
    }

  private:
    SecurityBox* m_security_box;
    OverlayPanel* m_panel;
    QWidget* m_input_box;
};

SecurityView::SecurityView(std::shared_ptr<ComboBox::QueryModel> query_model,
  QWidget& body, QWidget* parent)
  : SecurityView(std::move(query_model),
      std::make_shared<LocalValueModel<Security>>(), body, parent) {}

SecurityView::SecurityView(std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<CurrentModel> current, QWidget& body, QWidget* parent)
    : QWidget(parent),
      m_search_window(new SecuritySearchWindow(std::move(query_model), this)),
      m_current(std::move(current)),
      m_body(&body),
      m_current_index(-1) {
  setFocusPolicy(Qt::StrongFocus);
  m_prompt = make_label(tr("Enter a ticker symbol."));
  update_style(*m_prompt, [] (auto& style) {
    style.get(ReadOnly() && Disabled()).
      set(TextAlign(Qt::AlignCenter)).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  m_layers = new QStackedWidget();
  m_layers->addWidget(m_prompt);
  m_layers->addWidget(m_body);
  enclose(*this, *m_layers);
  m_search_window->connect_submit_signal(
    std::bind_front(&SecurityView::on_submit, this));
}

const std::shared_ptr<ComboBox::QueryModel>&
    SecurityView::get_query_model() const {
  return m_search_window->get_query_model();
}

const std::shared_ptr<SecurityView::CurrentModel>&
    SecurityView::get_current() const {
  return m_current;
}

const QWidget& SecurityView::get_body() const {
  return *m_body;
}

QWidget& SecurityView::get_body() {
  return *m_body;
}

void SecurityView::keyPressEvent(QKeyEvent* event) {
  if(auto text = event->text();
      text.size() == 1 && (text[0].isLetterOrNumber() || text[0] == '_')) {
    m_search_window->show();
    QApplication::sendEvent(find_focus_proxy(*m_search_window), event);
  } else if(event->key() == Qt::Key_PageUp && !m_securities.empty()) {
    m_current_index =
      (m_securities.size() + m_current_index - 1) % m_securities.size();
    m_current->set(m_securities[m_current_index]);
  } else if(event->key() == Qt::Key_PageDown && !m_securities.empty()) {
    m_current_index = (m_current_index + 1) % m_securities.size();
    m_current->set(m_securities[m_current_index]);
  }
  QWidget::keyPressEvent(event);
}

void SecurityView::on_submit(const Security& security) {
  if(auto i = std::find(m_securities.begin(), m_securities.end(), security);
      i != m_securities.end()) {
    if(std::distance(m_securities.begin(), i) <= m_current_index) {
      --m_current_index;
    }
    m_securities.erase(i);
  }
  ++m_current_index;
  m_securities.insert(m_securities.begin() + m_current_index, security);
  m_current->set(security);
  m_search_window->hide();
  if(!m_securities.empty() && m_layers->currentWidget() != m_body) {
    m_layers->setCurrentWidget(m_body);
  }
}
