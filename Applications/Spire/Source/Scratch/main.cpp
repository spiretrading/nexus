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
  using Fill = StateSelector<void, struct FillTag>;
  Box* m_fill;
  std::shared_ptr<ValueModel<int>> m_current;
  int m_last_current;
  optional<ExpressionExecutor<int>> m_fill_width;
  scoped_connection m_connection;

  ProgressBox(std::shared_ptr<ValueModel<int>> current)
      : m_current(std::move(current)),
        m_last_current(m_current->get()) {
    m_fill = new Box();
    match(*m_fill, Fill());
    m_fill->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_fill->setFixedWidth(compute_fill_width());
    auto box = new Box(m_fill);
    link(*this, *m_fill);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    enclose(*this, *box);
    proxy_style(*this, *box);
    update_style(*this, [] (auto& style) {
      style.get(Any()).
        set(BackgroundColor(QColor(0x321471)));
      style.get(Any() > Fill()).
        set(BackgroundColor(QColor(0x8D78EC)));
    });
    on_current(m_current->get());
    m_connection = m_current->connect_update_signal(
      std::bind_front(&ProgressBox::on_current, this));
  }

  QSize sizeHint() const {
    return scale(1, 4);
  }

  void resizeEvent(QResizeEvent* event) override {
    m_fill->setFixedWidth(compute_fill_width());
  }

  int compute_fill_width() const {
    return (width() * m_current->get()) / 100;
  }

  void on_current(int current) {
    if(current == m_last_current) {
      return;
    }
    m_fill_width = none;
    m_fill_width.emplace(make_evaluator(
      ease(m_fill->width(), compute_fill_width(), milliseconds(400)),
      find_stylist(*m_fill)));
    m_fill_width->connect_update_signal(
      std::bind_front(&ProgressBox::on_fill_width, this));
  }

  void on_fill_width(int width) {
    m_fill->setFixedWidth(width);
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
  std::shared_ptr<ValueModel<int>> m_download_progress;
  std::shared_ptr<ValueModel<int>> m_installation_progress;
  ProgressBox* m_progress_box;
  optional<ExpressionExecutor<int>> m_progress_width_executor;
  TextBox* m_time_left_label;
  scoped_connection m_download_progress_connection;
  scoped_connection m_installation_progress_connection;

  UpdateBox(std::shared_ptr<ValueModel<int>> download_progress,
      std::shared_ptr<ValueModel<int>> installation_progress,
      std::shared_ptr<ValueModel<time_duration>> time_left)
      : m_activity(std::make_shared<LocalValueModel<Activity>>(Activity::NONE)),
        m_download_progress(std::move(download_progress)),
        m_installation_progress(std::move(installation_progress)) {
    auto body = new QWidget();
    auto layout = make_vbox_layout(body);
    layout->addSpacing(scale_height(38));
    m_activity_label = make_activity_label(m_activity);
    layout->addWidget(m_activity_label);
    m_progress_box = new ProgressBox(m_download_progress);
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
    on_download_progress(m_download_progress->get());
    m_download_progress_connection = m_download_progress->connect_update_signal(
      std::bind_front(&UpdateBox::on_download_progress, this));
    on_activity(m_activity->get());
    m_activity->connect_update_signal(
      std::bind_front(&UpdateBox::on_activity, this));
  }

  void showEvent(QShowEvent* event) override {
    QTimer::singleShot(1000, this, [=] {
      m_activity->set(Activity::DOWNLOADING);
    });
  }

  void on_download_progress(int progress) {
    if(m_activity->get() != Activity::DOWNLOADING) {
      return;
    }
    if(progress >= 100) {
      m_activity->set(Activity::DOWNLOAD_COMPLETE);
      QTimer::singleShot(2000, this, [=] {
        m_activity->set(Activity::INSTALLING);
      });
    }
  }

  void on_installation_progress(int progress) {
    if(m_activity->get() != Activity::INSTALLING) {
      return;
    }
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
      auto installation_progress_box = new ProgressBox(m_installation_progress);
      installation_progress_box->setSizePolicy(
        QSizePolicy::Fixed, m_progress_box->sizePolicy().verticalPolicy());
      installation_progress_box->setFixedWidth(scale_width(280));
      auto progress_item =
        m_progress_box->parentWidget()->layout()->replaceWidget(
          m_progress_box, installation_progress_box);
      m_progress_box = installation_progress_box;
      delete progress_item->widget();
      delete progress_item;
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
  auto download_progress = std::make_shared<LocalValueModel<int>>(0);
  auto installation_progress = std::make_shared<LocalValueModel<int>>(0);
  auto time_left =
    std::make_shared<LocalValueModel<time_duration>>(seconds(40));
  auto window = UpdateBox(download_progress, installation_progress, time_left);
  window.show();
  auto time = 0;
  time += 1000;
  QTimer::singleShot(time, [&] {
    download_progress->set(20);
    time_left->set(seconds(8));
  });
  time += 2000;
  QTimer::singleShot(time, [&] {
    download_progress->set(40);
    time_left->set(seconds(6));
  });
  time += 2000;
  QTimer::singleShot(time, [&] {
    download_progress->set(60);
    time_left->set(seconds(4));
  });
  time += 2000;
  QTimer::singleShot(time, [&] {
    download_progress->set(70);
    time_left->set(seconds(2));
  });
  time += 2000;
  QTimer::singleShot(time, [&] {
    download_progress->set(100);
  });
  time += 2000;
  QTimer::singleShot(time, [&] {
    installation_progress->set(20);
    time_left->set(seconds(2));
  });
  time += 2000;
  QTimer::singleShot(time, [&] {
    installation_progress->set(100);
  });
  time += 3000;
  QTimer::singleShot(time, [&] {
    window.close();
  });
  application.exec();
}
