#include <QApplication>
#include <QTimer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/TransformValueModel.hpp"
#include "Spire/Styles/CubicBezierExpression.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

enum class Activity {
  NONE,
  DOWNLOADING,
  DOWNLOAD_COMPLETE,
  INSTALLING
};

using ActivityModel = ValueModel<Activity>;
using ActivityStyle = StateSelector<Activity, struct ActivityStyleTag>;

template<typename T>
struct ExpressionExecutor : private QObject {
  using Type = T;
  using UpdateSignal = Signal<void (const Type& value)>;
  static inline const auto FRAME_DURATION = time_duration(seconds(1) / 60);
  mutable UpdateSignal m_update;
  Evaluator<Type> m_evaluator;
  int m_frame_count;
  QTimer m_timer;

  explicit ExpressionExecutor(Evaluator<Type> evaluator)
    : m_evaluator(std::move(evaluator)),
      m_frame_count(0) {
    m_timer.setInterval(FRAME_DURATION.total_milliseconds());
    connect(&m_timer, &QTimer::timeout, this,
      std::bind_front(&ExpressionExecutor::on_timeout, this));
    m_timer.start();
  }

  connection connect_update_signal(const UpdateSignal::slot_type& slot) const {
    return m_update.connect(slot);
  }

  void on_timeout() {
    ++m_frame_count;
    auto evaluation = m_evaluator(FRAME_DURATION * m_frame_count);
    m_update(evaluation.m_value);
    if(evaluation.m_next_frame == pos_infin) {
      m_timer.stop();
    }
  }
};

auto make_activity_message(std::shared_ptr<ActivityModel> activity) {
  return make_transform_value_model(std::move(activity), [] (auto activity) {
    if(activity == Activity::NONE) {
      return QObject::tr("Update available.");
    } else if(activity == Activity::DOWNLOADING) {
      return QObject::tr("Downloading update...");
    } else if(activity == Activity::DOWNLOAD_COMPLETE) {
      return QObject::tr("Download complete");
    }
    return QObject::tr("Installing update...");
  });
}

auto make_activity_label(std::shared_ptr<ActivityModel> activity) {
  using Label = StateSelector<void, struct LabelStateTag>;
  auto label = make_label(make_activity_message(activity));
  label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  match(*label, Label());
  update_style(*label, [] (auto& style) {
    style.get(Any()).
      set(PaddingBottom(scale_height(6))).
      set(PaddingLeft(scale_width(76))).
      set(TextColor(QColor(0xFFFFFF)));
    style.get(ActivityStyle(Activity::DOWNLOADING)).
      set(PaddingLeft(ease(scale_width(76), 0, milliseconds(800))));
    style.get(ActivityStyle(Activity::DOWNLOAD_COMPLETE)).
      set(PaddingLeft(0));
    style.get(ActivityStyle(Activity::INSTALLING)).
      set(PaddingLeft(0));
  });
  return label;
}

struct ProgressBox : QWidget {
  ProgressBox() {
    auto box = new Box();
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    enclose(*this, *box);
    proxy_style(*this, *box);
    update_style(*this, [] (auto& style) {
      style.get(Any()).
        set(BackgroundColor(QColor(0x321471)));
    });
  }

  QSize sizeHint() const {
    return scale(1, 4);
  }
};

auto make_time_left_message(
    std::shared_ptr<ValueModel<time_duration>> time_left) {
  return make_transform_value_model(std::move(time_left),
    [] (const auto& time_left) {
      if(time_left == pos_infin) {
        return QString();
      } else if(time_left.total_seconds() <= 10) {
        return QObject::tr("A few seconds left");
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
      return QStringLiteral("About %1 left").arg(duration_text);
    });
}

auto make_time_left_label(std::shared_ptr<ValueModel<time_duration>> current) {
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

struct UpdateBox : QWidget {
  std::shared_ptr<ActivityModel> m_activity;
  Activity m_last_activity;
  TextBox* m_activity_label;
  ProgressBox* m_progress_box;
  optional<ExpressionExecutor<int>> m_progress_width_executor;
  TextBox* m_time_left_label;
  scoped_connection m_activity_connection;

  UpdateBox(std::shared_ptr<ActivityModel> activity,
      std::shared_ptr<ValueModel<time_duration>> time_left)
      : m_activity(std::move(activity)) {
    auto body = new QWidget();
    auto layout = make_vbox_layout(body);
    layout->addSpacing(scale_height(38));
    m_activity_label = make_activity_label(m_activity);
    layout->addWidget(m_activity_label);
    m_progress_box = new ProgressBox();
    m_progress_box->setSizePolicy(
      QSizePolicy::Fixed, m_progress_box->sizePolicy().verticalPolicy());
    m_progress_box->setFixedWidth(scale_width(140));
    m_progress_box->hide();
    layout->addWidget(m_progress_box);
    m_time_left_label = make_time_left_label(std::move(time_left));
    layout->addWidget(m_time_left_label);
    m_time_left_label->hide();
    auto box = new Box(body);
    enclose(*this, *box);
    proxy_style(*this, *box);
    update_style(*this, [] (auto& style) {
      style.get(Any()).set(BackgroundColor(QColor(0x4B23A0)));
    });
    setFixedSize(scale(280, 232));
    if(m_activity->get() == Activity::NONE) {
      m_last_activity = Activity::DOWNLOADING;
    } else {
      m_last_activity = Activity::NONE;
    }
    on_activity(m_activity->get());
    m_activity_connection = m_activity->connect_update_signal(
      std::bind_front(&UpdateBox::on_activity, this));
  }

  void on_activity(Activity activity) {
    if(activity == m_last_activity) {
      return;
    }
    unmatch(*m_activity_label, ActivityStyle(m_last_activity));
    m_last_activity = activity;
    if(activity == Activity::DOWNLOADING) {
      m_progress_box->show();
      m_progress_width_executor.emplace(make_evaluator(
        ease(scale_width(140), scale_width(280), milliseconds(800)),
        find_stylist(*m_progress_box)));
      m_progress_width_executor->connect_update_signal(
        std::bind_front(&UpdateBox::on_progress_width_update, this));
      QTimer::singleShot(2000, this, [=] {
        if(m_last_activity == Activity::DOWNLOADING) {
          m_time_left_label->show();
        }
      });
    } else if(activity == Activity::DOWNLOAD_COMPLETE) {
      m_time_left_label->hide();
    } else if(activity == Activity::INSTALLING) {
      QTimer::singleShot(2000, this, [=] {
        if(m_last_activity == Activity::INSTALLING) {
          m_time_left_label->show();
        }
      });
    }
    match(*m_activity_label, ActivityStyle(activity));
  }

  void on_progress_width_update(int width) {
    m_progress_box->setFixedWidth(width);
  }
};

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto activity = std::make_shared<LocalValueModel<Activity>>(Activity::NONE);
  auto time_left =
    std::make_shared<LocalValueModel<time_duration>>(seconds(40));
  auto window = UpdateBox(activity, time_left);
  window.show();
  QTimer::singleShot(1000, [&] {
    activity->set(Activity::DOWNLOADING);
    QTimer::singleShot(6000, [&] {
      time_left->set(seconds(5));
      QTimer::singleShot(6000, [&] {
        activity->set(Activity::DOWNLOAD_COMPLETE);
        QTimer::singleShot(2000, [&] {
          activity->set(Activity::INSTALLING);
          QTimer::singleShot(4000, [&] {
            window.close();
          });
        });
      });
    });
  });
  application.exec();
}
