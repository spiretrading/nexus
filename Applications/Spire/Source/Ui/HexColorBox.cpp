#include "Spire/Ui/HexColorBox.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QKeyEvent>
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

//struct HexColorModel : LocalTextModel {
//  std::shared_ptr<TextModel> m_source;
//  QValidator::State m_state;
//  scoped_connection m_current_connection;
//
//  HexColorModel(std::shared_ptr<TextModel> source)
//    : m_source(std::move(source)),
//      m_current_connection(m_source->connect_update_signal(
//        std::bind_front(&HexColorModel::on_current, this))) {
//  }
//
//  QValidator::State get_state() const override {
//    if(QRegExp("[#]?[0-9a-fA-F]{1,6}").exactMatch(get())) {
//      return QValidator::Acceptable;
//    }
//    return QValidator::Invalid;
//  }
//
//  void on_current(const QString& current) {
//    if(QRegExp("[#]?[0-9a-fA-F]{0,6}").exactMatch(current)) {
//      set(current);
//    } else {
//      m_source->set(get());
//    }
//  }
//};

HexColorBox::HexColorBox(QWidget* parent)
  : HexColorBox(std::make_shared<LocalTextModel>(), parent) {}

HexColorBox::HexColorBox(QString current, QWidget* parent)
  : HexColorBox(std::make_shared<LocalTextModel>(std::move(current)), parent) {}

HexColorBox::HexColorBox(std::shared_ptr<TextModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_submission(m_current->get()),
      m_is_rejected(false),
      m_current_connection(m_current->connect_update_signal(
        std::bind_front(&HexColorBox::on_current, this))) {
  m_text_box = new TextBox();
  m_text_box_current_connection = m_text_box->get_current()->connect_update_signal(
    std::bind_front(&HexColorBox::on_text_box_current, this));
  m_text_box_submit_connection = m_text_box->connect_submit_signal(
    std::bind_front(&HexColorBox::on_text_box_submission, this));
  enclose(*this, *m_text_box);
  on_current(m_current->get());
}

const std::shared_ptr<TextModel>& HexColorBox::get_current() const {
  return m_current;
}

connection HexColorBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

connection HexColorBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}

bool HexColorBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto& key_event = static_cast<QKeyEvent&>(*event);
    if(key_event.key() == Qt::Key_Escape) {
      m_current->set(m_submission);
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void HexColorBox::showEvent(QShowEvent* event) {
  m_text_box->focusProxy()->installEventFilter(this);
  QWidget::showEvent(event);
}

void HexColorBox::on_current(const QString& current) {
  auto blocker = shared_connection_block(m_text_box_current_connection);
  m_text_box->get_current()->set(current);
  if(m_is_rejected) {
    m_is_rejected = false;
    unmatch(*m_text_box, Rejected());
  }
}

void HexColorBox::on_text_box_current(const QString& current) {
  if(QRegExp("[#]?[0-9a-fA-F]{0,6}").exactMatch(current)) {
    auto blocker = shared_connection_block(m_current_connection);
    m_current->set(current);
  } else {
    m_text_box->get_current()->set(m_current->get());
  }
  if(m_is_rejected) {
    m_is_rejected = false;
    unmatch(*m_text_box, Rejected());
  }
}

void HexColorBox::on_text_box_submission(const QString& submission) {
  if(QRegExp("[#]?[0-9a-fA-F]{1,6}").exactMatch(submission)) {
    if(m_submission != submission) {
      auto value = submission;
      value = value.replace('#', "");
      value = value.toUpper();
      auto length = value.length();
      if(length == 1 || length == 2) {
        value = value.repeated(6 / length);
      } else if(length == 3) {
        value = QString(value[0]).repeated(2) + QString(value[1]).repeated(2) +
          QString(value[2]).repeated(2);
      } else if(length == 4 || length == 5) {
        value = QString("0").repeated(6 - length) + value;
      }
      value = "#" + value;
      m_current->set(value);
    }
    m_submission = m_current->get();
    m_submit_signal(m_submission);
  } else {
    m_reject_signal(submission);
    m_current->set(m_submission);
    if(!m_is_rejected) {
      m_is_rejected = true;
      match(*m_text_box, Rejected());
    }
  }
}
