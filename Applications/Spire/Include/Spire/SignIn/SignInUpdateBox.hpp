#ifndef SPIRE_SIGN_IN_UPDATE_BOX_HPP
#define SPIRE_SIGN_IN_UPDATE_BOX_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include <QWidget>
#include "Spire/Styles/PeriodicEvaluator.hpp"
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/ProgressBar.hpp"

namespace Spire {
  template<typename> class ProxyValueModel;
  class TextBox;

  /** Displays the status of an update operation during sign-in. */
  class SignInUpdateBox : public QWidget {
    public:

      /** Lists the operations performed during an update. */
      enum class Operation {

        /** The update is being downloaded. */
        DOWNLOAD,

        /** The update is being installed. */
        INSTALL
      };

      /**
       * Signals a request to retry an operation.
       * @param operation The operation to retry.
       */
      using RetrySignal = Signal<void (Operation operation)>;

      /** Signals a request to cancel the update. */
      using CancelSignal = Signal<void ()>;

      /**
       * Constructs a SignInUpdateBox.
       * @param download_progress The amount of progress downloading the update.
       * @param installation_progress The amount of progress performing the
       *        installation.
       * @param time_left The amount of time left to perform the current
       *        operation.
       */
      SignInUpdateBox(std::shared_ptr<ProgressModel> download_progress,
        std::shared_ptr<ProgressModel> installation_progress,
        std::shared_ptr<ValueModel<boost::posix_time::time_duration>>
          time_left);

      /** Returns a connection to the RetrySignal. */
      boost::signals2::connection connect_retry_signal(
        const RetrySignal::slot_type& slot) const;

      /** Returns a connection to the CancelSignal. */
      boost::signals2::connection connect_cancel_signal(
          const CancelSignal::slot_type& slot) const;

    protected:
      void showEvent(QShowEvent* event) override;

    private:
      enum class Activity {
        NONE,
        DOWNLOADING,
        DOWNLOAD_COMPLETE,
        INSTALLING,
        DOWNLOAD_ERROR,
        INSTALLATION_ERROR
      };
      using ActivityModel = ValueModel<Activity>;
      using ActivityStyle =
        Styles::StateSelector<Activity, struct ActivityStyleTag>;
      mutable RetrySignal m_retry_signal;
      mutable CancelSignal m_cancel_signal;
      std::shared_ptr<ProgressModel> m_download_progress;
      std::shared_ptr<ProgressModel> m_installation_progress;
      std::shared_ptr<ProxyValueModel<ProgressModel::Type>> m_proxy_progress;
      std::shared_ptr<ValueModel<boost::posix_time::time_duration>> m_time_left;
      ProgressBar* m_progress_bar;
      boost::optional<Styles::PeriodicEvaluator<int>>
        m_progress_width_evaluator;
      TextBox* m_time_left_label;
      std::shared_ptr<ActivityModel> m_activity;
      boost::optional<Activity> m_last_activity;
      TextBox* m_activity_label;
      boost::signals2::scoped_connection m_download_progress_connection;
      boost::signals2::scoped_connection m_installation_progress_connection;

      static auto make_activity_message(
        std::shared_ptr<ActivityModel> activity);
      static auto make_activity_label(std::shared_ptr<ActivityModel> activity);
      static auto make_error_message(std::shared_ptr<ActivityModel> activity);
      static auto make_error_message_label(
        std::shared_ptr<ActivityModel> activity);
      QWidget& get_body();
      void clear_layout();
      void layout_activity();
      void layout_error();
      void on_download_progress(int progress);
      void on_installation_progress(int progress);
      void on_activity(Activity activity);
      void on_progress_width_evaluated(int width);
      void on_cancel();
      void on_retry();
  };
}

#endif
