#include "Spire/SignIn/SignInUpdateBox.hpp"
#include <QTimer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ProxyValueModel.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Styles/CubicBezierExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using Transitioned = StateSelector<void, struct TransitionedStyleTag>;

  auto make_time_left_message(
      std::shared_ptr<ValueModel<time_duration>> time_left) {
    return make_transform_value_model(std::move(time_left),
      [] (const auto& time_left) {
        if(time_left == pos_infin) {
          return QString();
        } else if(time_left.total_seconds() <= 10) {
          return QObject::tr("A few seconds left.");
        }
        auto hours = time_left.hours();
        auto minutes = time_left.minutes();
        auto seconds =
          (time_left.total_seconds() - 3600 * hours - 60 * minutes) / 10;
        auto parts = QStringList();
        if(hours >= 1) {
          parts << QStringLiteral("%1h").arg(hours);
        }
        if(minutes >= 1 || hours >= 1) {
          parts << QStringLiteral("%1m").arg(minutes);
        }
        if(hours < 1 && minutes < 1) {
          parts << QStringLiteral("%1s").arg(10 * seconds);
        }
        auto duration_text = parts.join("");
        return QStringLiteral("About %1 left.").arg(duration_text);
      });
  }

  auto make_time_left_label(
      std::shared_ptr<ValueModel<time_duration>> current) {
    auto label = make_label(make_time_left_message(current));
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    update_style(*label, [] (auto& style) {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Medium);
      font.setPixelSize(scale_width(10));
      style.get(Any()).
        set(text_style(font, QColor(0xE2E0FF))).
        set(PaddingTop(scale_height(4)));
    });
    return label;
  }

  auto make_action_button(QString label) {
    auto button = make_label_button(std::move(label));
    update_style(*button, [] (auto& style) {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Bold);
      font.setPixelSize(scale_width(14));
      style.get(Any() > Body()).
        set(BackgroundColor(QColor(0x684BC7))).
        set(border_color(QColor(0x684BC7))).
        set(text_style(font, QColor(0xE2E0FF)));
      style.get(Hover() > Body()).
        set(BackgroundColor(QColor(0x8D78EC))).
        set(border_color(QColor(0x8D78EC))).
        set(TextColor(QColor(0xFFFFFF)));
      style.get(Press() > Body()).
        set(BackgroundColor(QColor(0x4B23A0))).
        set(TextColor(QColor(0xFFFFFF)));
      style.get(FocusVisible() > Body()).
        set(border_color(QColor(0x8D78EC)));
      style.get(Disabled() > Body()).
        set(BackgroundColor(QColor(0x4B23A0))).
        set(TextColor(QColor(0x684BC7)));
    });
    button->setFixedSize(scale(120, 30));
    return button;
  }
}

SignInUpdateBox::SignInUpdateBox(
    std::shared_ptr<ProgressModel> download_progress,
    std::shared_ptr<ProgressModel> installation_progress,
    std::shared_ptr<ValueModel<time_duration>> time_left)
    : m_download_progress(std::move(download_progress)),
      m_installation_progress(std::move(installation_progress)),
      m_proxy_progress(
        make_proxy_value_model(std::make_shared<LocalProgressModel>(0))),
      m_time_left(std::move(time_left)),
      m_activity(std::make_shared<LocalValueModel<Activity>>(Activity::NONE)) {
  auto body = new QWidget();
  auto layout = make_vbox_layout(body);
  auto box = new Box(body);
  enclose(*this, *box);
  proxy_style(*this, *box);
  update_style(*this, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0x4B23A0)));
    style.get(Any() > is_a<ProgressBar>()).
      set(BackgroundColor(QColor(0x321471)));
  });
  layout_activity();
  m_download_progress_connection =
    m_download_progress->connect_update_signal(
      std::bind_front(&SignInUpdateBox::on_download_progress, this));
  m_installation_progress_connection =
    m_installation_progress->connect_update_signal(
      std::bind_front(&SignInUpdateBox::on_installation_progress, this));
  m_activity->connect_update_signal(
    std::bind_front(&SignInUpdateBox::on_activity, this));
}

connection SignInUpdateBox::connect_retry_signal(
    const RetrySignal::slot_type& slot) const {
  return m_retry_signal.connect(slot);
}

connection SignInUpdateBox::connect_cancel_signal(
    const CancelSignal::slot_type& slot) const {
  return m_cancel_signal.connect(slot);
}

void SignInUpdateBox::showEvent(QShowEvent* event) {
  static const auto TRANSITION_TIME_MS = 1000;
  QTimer::singleShot(TRANSITION_TIME_MS, this, [=] {
    if(m_activity->get() == Activity::NONE) {
      m_activity->set(Activity::DOWNLOADING);
    }
  });
}

auto SignInUpdateBox::make_activity_message(
    std::shared_ptr<ActivityModel> activity) {
  return make_transform_value_model(std::move(activity), [] (auto activity) {
    if(activity == Activity::NONE) {
      return QObject::tr("Update available.");
    } else if(activity == Activity::DOWNLOADING) {
      return QObject::tr("Downloading update...");
    } else if(activity == Activity::DOWNLOAD_COMPLETE) {
      return QObject::tr("Download complete.");
    }
    return QObject::tr("Installing update...");
  });
}

auto SignInUpdateBox::make_activity_label(
    std::shared_ptr<ActivityModel> activity) {
  using Label = StateSelector<void, struct LabelStateTag>;
  auto label = make_label(make_activity_message(activity));
  label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  match(*label, Label());
  update_style(*label, [] (auto& style) {
    style.get(Any()).
      set(PaddingBottom(scale_height(6))).
      set(PaddingLeft(scale_width(94))).
      set(TextColor(QColor(0xFFFFFF)));
    style.get(ActivityStyle(Activity::DOWNLOADING)).
      set(PaddingLeft(ease(scale_width(94), 0, milliseconds(800))));
    style.get(Transitioned() || ActivityStyle(Activity::DOWNLOAD_COMPLETE) ||
      ActivityStyle(Activity::INSTALLING)).set(PaddingLeft(0));
  });
  return label;
}

auto SignInUpdateBox::make_error_message(
    std::shared_ptr<ActivityModel> activity) {
  return make_transform_value_model(std::move(activity), [] (auto activity) {
    if(activity == Activity::DOWNLOAD_ERROR) {
      return QObject::tr("Unable to complete download.");
    }
    return QObject::tr("Unable to complete installation.");
  });
}

auto SignInUpdateBox::make_error_message_label(
    std::shared_ptr<ActivityModel> activity) {
  auto label = make_label(make_error_message(activity));
  label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  update_style(*label, [] (auto& style) {
    style.get(Any()).
      set(TextColor(QColor(0xFAEB96))).
      set(TextAlign(Qt::AlignCenter)).
      set(PaddingBottom(scale_height(15)));
  });
  return label;
}

QWidget& SignInUpdateBox::get_body() {
  return *static_cast<Box*>(layout()->itemAt(0)->widget())->get_body();
}

void SignInUpdateBox::clear_layout() {
  auto layout = static_cast<QBoxLayout*>(get_body().layout());
  clear(*layout);
  m_progress_bar = nullptr;
  m_progress_width_evaluator = none;
  m_time_left_label = nullptr;
  m_activity_label = nullptr;
}

void SignInUpdateBox::layout_activity() {
  auto layout = static_cast<QBoxLayout*>(get_body().layout());
  layout->addSpacing(scale_height(38));
  m_activity_label = make_activity_label(m_activity);
  layout->addWidget(m_activity_label);
  m_progress_bar = new ProgressBar(m_proxy_progress);
  m_progress_bar->setSizePolicy(
    QSizePolicy::Fixed, m_progress_bar->sizePolicy().verticalPolicy());
  m_progress_bar->setFixedWidth(scale_width(140));
  m_progress_bar->hide();
  layout->addWidget(m_progress_bar);
  link(*this, *m_progress_bar);
  m_time_left_label = make_time_left_label(m_time_left);
  m_time_left_label->hide();
  layout->addWidget(m_time_left_label);
  layout->addStretch(1);
  on_download_progress(m_download_progress->get());
  on_installation_progress(m_installation_progress->get());
  on_activity(m_activity->get());
}

void SignInUpdateBox::layout_error() {
  auto layout = static_cast<QBoxLayout*>(get_body().layout());
  layout->addSpacing(scale_height(20));
  layout->addWidget(make_error_message_label(m_activity));
  layout->addStretch(1);
  auto button_layout = make_hbox_layout();
  auto cancel_button = make_action_button(tr("Cancel"));
  cancel_button->connect_click_signal(
    std::bind_front(&SignInUpdateBox::on_cancel, this));
  button_layout->addWidget(cancel_button);
  button_layout->addStretch(1);
  auto retry_button = make_action_button(tr("Retry"));
  retry_button->connect_click_signal(
    std::bind_front(&SignInUpdateBox::on_retry, this));
  button_layout->addWidget(retry_button);
  layout->addLayout(button_layout);
}

void SignInUpdateBox::on_download_progress(int progress) {
  if(m_activity->get() == Activity::DOWNLOAD_ERROR) {
    if(progress != -1) {
      m_activity->set(Activity::DOWNLOADING);
    }
  } else if(m_activity->get() == Activity::DOWNLOADING) {
    if(progress == -1) {
      m_activity->set(Activity::DOWNLOAD_ERROR);
    } else if(progress >= 100) {
      m_activity->set(Activity::DOWNLOAD_COMPLETE);
      QTimer::singleShot(2000, this, [=] {
        m_activity->set(Activity::INSTALLING);
      });
    }
  }
}

void SignInUpdateBox::on_installation_progress(int progress) {
  if(m_activity->get() == Activity::INSTALLATION_ERROR) {
    if(progress != -1) {
      m_activity->set(Activity::INSTALLING);
    }
  } else if(m_activity->get() == Activity::INSTALLING) {
    if(progress == -1) {
      m_activity->set(Activity::INSTALLATION_ERROR);
    }
  }
}

void SignInUpdateBox::on_activity(Activity activity) {
  if(activity == m_last_activity) {
    return;
  }
  if(m_last_activity && m_activity_label) {
    unmatch(*m_activity_label, ActivityStyle(*m_last_activity));
  }
  if(activity == Activity::DOWNLOADING) {
    if(m_last_activity == Activity::DOWNLOAD_ERROR) {
      m_last_activity = activity;
      clear_layout();
      layout_activity();
      match(*m_activity_label, Transitioned());
    }
    m_progress_bar->show();
    if(!is_match(*m_activity_label, Transitioned())) {
      m_progress_width_evaluator.emplace(make_evaluator(
        ease(scale_width(140), scale_width(280), milliseconds(800)),
        find_stylist(*m_progress_bar)));
      m_progress_width_evaluator->connect_evaluated_signal(
        std::bind_front(&SignInUpdateBox::on_progress_width_evaluated, this));
    } else {
      m_progress_bar->setFixedWidth(scale_width(280));
    }
    QTimer::singleShot(2000, this, [=] {
      if(m_last_activity == Activity::DOWNLOADING) {
        m_time_left_label->show();
      }
    });
    auto last_activity = m_last_activity;
    on_download_progress(m_download_progress->get());
    if(last_activity != m_last_activity) {
      return;
    }
  } else if(activity == Activity::DOWNLOAD_ERROR ||
      activity == Activity::INSTALLATION_ERROR) {
    clear_layout();
    layout_error();
  } else if(activity == Activity::DOWNLOAD_COMPLETE) {
    m_time_left_label->hide();
  } else if(activity == Activity::INSTALLING) {
    if(m_last_activity == Activity::INSTALLATION_ERROR) {
      m_last_activity = activity;
      clear_layout();
      layout_activity();
      match(*m_activity_label, Transitioned());
    }
    auto installation_progress_bar = new ProgressBar(m_installation_progress);
    installation_progress_bar->setSizePolicy(
      QSizePolicy::Fixed, m_progress_bar->sizePolicy().verticalPolicy());
    installation_progress_bar->setFixedWidth(scale_width(280));
    auto progress_item =
      m_progress_bar->parentWidget()->layout()->replaceWidget(
        m_progress_bar, installation_progress_bar);
    m_progress_bar = installation_progress_bar;
    link(*this, *m_progress_bar);
    delete progress_item->widget();
    delete progress_item;
    QTimer::singleShot(2000, this, [=] {
      if(m_last_activity == Activity::INSTALLING) {
        m_time_left_label->show();
      }
    });
    auto last_activity = m_last_activity;
    on_installation_progress(m_installation_progress->get());
    if(last_activity != m_last_activity) {
      return;
    }
  }
  if(m_activity_label) {
    match(*m_activity_label, ActivityStyle(activity));
  }
  m_last_activity = activity;
}

void SignInUpdateBox::on_progress_width_evaluated(int width) {
  m_progress_bar->setFixedWidth(width);
  if(m_progress_width_evaluator->is_complete()) {
    m_proxy_progress->set_source(m_download_progress);
  }
}

void SignInUpdateBox::on_cancel() {
  m_cancel_signal();
}

void SignInUpdateBox::on_retry() {
  if(m_activity->get() == Activity::DOWNLOAD_ERROR) {
    m_retry_signal(Operation::DOWNLOAD);
  } else {
    m_retry_signal(Operation::INSTALL);
  }
}
