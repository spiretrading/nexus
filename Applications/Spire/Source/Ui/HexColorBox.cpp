#include "Spire/Ui/HexColorBox.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QKeyEvent>
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  QColor to_color(const QString& text) {
    auto color = [&] {
      auto name = text;
      name = name.replace("#", "");
      auto length = name.length();
      if(length == 1 || length == 2) {
        return name.repeated(6 / length);
      } else if(length == 3) {
        return QString(name[0]).repeated(2) + QString(name[1]).repeated(2) +
          QString(name[2]).repeated(2);
      } else if(length == 4 || length == 5) {
        return QString("0").repeated(6 - length) + name;
      } else if(length == 6) {
        return name;
      }
      return QString("000000");
    }();
    return "#" + color;
  }
}

struct HexColorBox::ColorToTextModel : TextModel {
  mutable UpdateSignal m_update_signal;
  std::shared_ptr<ValueModel<QColor>> m_source;
  QString m_current;
  QRegExp m_validator;
  bool m_is_rejected;
  scoped_connection m_current_connection;

  ColorToTextModel(std::shared_ptr<ValueModel<QColor>> source)
    : m_source(std::move(source)),
      m_current(m_source->get().name()),
      m_validator("[#]?[0-9a-fA-F]{0,6}"),
      m_is_rejected(false),
      m_current_connection(m_source->connect_update_signal(
        std::bind_front(&ColorToTextModel::on_current, this))) {}

  const QColor& submit() {
    if(auto value = m_source->get().name(); value != m_current) {
      m_current = std::move(value);
      m_update_signal(m_current);
    }
    return m_source->get();
  }

  void reject() {
    m_is_rejected = true;
  }

  QValidator::State get_state() const override {
    return m_source->get_state();
  }

  const QString& get() const {
    return m_current;
  }

  QValidator::State test(const QString& value) const override {
    if(m_validator.exactMatch(value)) {
      return QValidator::Acceptable;
    }
    return QValidator::Invalid;
  }

  QValidator::State set(const QString& value) override {
    if(test(value) == QValidator::Invalid) {
      m_is_rejected = false;
      return QValidator::Invalid;
    }
    if(auto color = to_color(value); color != m_source->get()) {
      auto blocker = shared_connection_block(m_current_connection);
      if(m_source->set(color) == QValidator::Invalid) {
        m_is_rejected = false;
        return QValidator::Invalid;
      }
    }
    if(m_is_rejected) {
      m_current = m_source->get().name();
    } else {
      m_current = value;
    }
    m_is_rejected = false;
    m_update_signal(m_current);
    return QValidator::Acceptable;
  }

  connection connect_update_signal(
      const typename UpdateSignal::slot_type& slot) const override {
    return m_update_signal.connect(slot);
  }

  void on_current(const QColor& color) {
    set(color.name());
  }
};

HexColorBox::HexColorBox(QWidget* parent)
  : HexColorBox(std::make_shared<LocalValueModel<QColor>>(), parent) {}

HexColorBox::HexColorBox(QColor current, QWidget* parent)
  : HexColorBox(std::make_shared<LocalValueModel<QColor>>(std::move(current)),
      parent) {}

HexColorBox::HexColorBox(std::shared_ptr<ValueModel<QColor>> current,
    QWidget* parent)
    : QWidget(parent),
      m_adaptor_model(std::make_shared<ColorToTextModel>(std::move(current))),
      m_submission(m_adaptor_model->get()) {
  m_text_box = new TextBox(m_adaptor_model);
  enclose(*this, *m_text_box);
  m_submit_connection = m_text_box->connect_submit_signal(
    std::bind_front(&HexColorBox::on_submit, this));
  m_reject_connection = m_text_box->connect_reject_signal(
    std::bind_front(&HexColorBox::on_reject, this));
}

const std::shared_ptr<ValueModel<QColor>>& HexColorBox::get_current() const {
  return m_adaptor_model->m_source;
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
      m_adaptor_model->set(m_submission.name());
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void HexColorBox::showEvent(QShowEvent* event) {
  m_text_box->focusProxy()->installEventFilter(this);
  QWidget::showEvent(event);
}

void HexColorBox::on_submit(const QString& submission) {
  m_submission = m_adaptor_model->submit();
  m_submit_signal(m_submission);
}

void HexColorBox::on_reject(const QString& value) {
  m_reject_signal(to_color(value));
  m_adaptor_model->reject();
}
