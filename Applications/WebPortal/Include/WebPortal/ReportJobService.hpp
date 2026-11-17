#ifndef NEXUS_REPORT_JOB_SERVICE_HPP
#define NEXUS_REPORT_JOB_SERVICE_HPP
#include <algorithm>
#include <concepts>
#include <condition_variable>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <stop_token>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/Threading/ThreadPool.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/scope/scope_exit.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "WebPortal/ReportSubmission.hpp"

namespace Nexus {

  /** Provides report job storage and execution. */
  template<typename T>
  concept IsReportJobBackend = requires(T& backend) {
    { backend.load_job(std::declval<const std::string&>()) } ->
        std::same_as<std::optional<ReportJob>>;
    { backend.store(std::declval<const ReportJob&>()) } -> std::same_as<void>;
    { backend.remove(std::declval<const std::string&>()) } ->
        std::same_as<void>;
    { backend.execute(std::declval<const ReportJob&>(),
        std::declval<std::stop_token&>()) } -> std::convertible_to<int>;
  };

  /**
   * Queues reports by account and limits concurrent execution.
   * Executes at most one report per account.
   * @tparam B The backend providing job storage and execution.
   * @tparam T The time client or pointer supplying job timestamps.
   */
  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  class ReportJobService {
    public:

      /** The backend providing job storage and execution. */
      using Backend = B;

      /** The time client or pointer supplying job timestamps. */
      using TimeClient = T;

      /**
       * Constructs a report worker.
       * @tparam TF The time client initializer type.
       * @param backend The backend providing storage and execution.
       * @param time_client Initializes the time client.
       * @param max_concurrency The positive global execution limit.
       * @param timer The timer used to retry saving completed jobs.
       * @param administration_client The client used to send notifications.
       * @param notification_timer The timer checking notification deadlines.
       * @param notification_delay The grace period after saving a result.
       */
      template<Beam::Initializes<T> TF>
      ReportJobService(Beam::Ref<Backend> backend, TF&& time_client,
        std::size_t max_concurrency, Beam::Timer timer,
        AdministrationClient administration_client,
        Beam::Timer notification_timer,
        boost::posix_time::time_duration notification_delay) requires
          IsReportJobBackend<Backend>;

      ~ReportJobService();

      /**
       * Records a fetch of a final result by its submitting account.
       * @param account The account that fetched the result.
       * @param job The fetched job snapshot.
       */
      void mark_loaded(
        const Beam::DirectoryEntry& account, const ReportJob& job);

      /** Stores a prepared job and returns its identifier without waiting. */
      std::string submit(const ReportJob& submission);

      /**
       * Activates staged or queued jobs unless already pending, and marks
       * interrupted running jobs as failed.
       */
      void resume(const std::string& id);

      /**
       * Requeues an account's failed jobs using their saved definitions.
       * @tparam V The validator for the failed job snapshots.
       * @param account The submitting account.
       * @param ids The job identifiers to retry.
       * @param validate Checks access to all failed jobs before any changes.
       */
      template<typename V> requires std::invocable<V&, std::vector<ReportJob>&>
      void retry(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids, V validate);

      /** Adds resolved recipients to completed reports owned by an account. */
      void share(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids,
        const std::vector<Beam::DirectoryEntry>& recipients);

      /** Cancels an account's jobs and removes them from activity. */
      void cancel(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids);

      /** Deletes completed reports owned by an account. */
      void remove(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids);

      /** Stops execution and marks unfinished jobs as failed. */
      void close();

    private:
      struct Job {
        ReportJob m_job;
        std::stop_source m_stop;
        boost::posix_time::ptime m_previous_completion;
      };
      struct PendingResult {
        ReportJob m_job;
        boost::posix_time::ptime m_previous_completion;
      };
      struct PendingNotification {
        ReportJob m_job;
        boost::posix_time::ptime m_deadline;
      };
      Backend* m_backend;
      Beam::local_ptr_t<TimeClient> m_time_client;
      Beam::Timer m_timer;
      std::shared_ptr<Beam::Queue<Beam::Timer::Result>> m_ticks;
      AdministrationClient m_administration_client;
      Beam::Timer m_notification_timer;
      boost::posix_time::time_duration m_notification_delay;
      std::shared_ptr<Beam::Queue<Beam::Timer::Result>> m_notification_ticks;
      mutable std::mutex m_retry_mutex;
      mutable std::mutex m_mutex;
      bool m_is_closed;
      std::unordered_map<std::string, std::shared_ptr<Job>> m_pending;
      std::unordered_map<std::string, PendingResult> m_results;
      std::unordered_map<std::string, PendingNotification> m_notifications;
      std::unordered_map<unsigned int, std::deque<std::shared_ptr<Job>>> m_jobs;
      std::deque<unsigned int> m_accounts;
      std::condition_variable m_is_available;
      Beam::RoutineHandlerGroup m_routines;
      Beam::RoutineHandler m_retry_routine;
      Beam::RoutineHandler m_notification_routine;

      std::vector<ReportJob> load_jobs(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids);
      bool store_result(PendingResult& result);
      void flush_results();
      void enqueue(const std::shared_ptr<Job>& entry);
      void finish(const std::shared_ptr<Job>& entry);
      void schedule_notification(const ReportJob& job);
      void run();
      void run_retries();
      void run_notifications();
  };

  template<typename B, typename T>
  ReportJobService(Beam::Ref<B>, T&&, std::size_t, Beam::Timer,
    AdministrationClient, Beam::Timer, boost::posix_time::time_duration) ->
      ReportJobService<B, std::remove_cvref_t<T>>;

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  template<Beam::Initializes<T> TF>
  ReportJobService<B, T>::ReportJobService(
      Beam::Ref<Backend> backend, TF&& time_client,
      std::size_t max_concurrency, Beam::Timer timer,
      AdministrationClient administration_client,
      Beam::Timer notification_timer,
      boost::posix_time::time_duration notification_delay) requires
      IsReportJobBackend<Backend>
      : m_backend(backend.get()),
        m_time_client(std::forward<TF>(time_client)),
        m_timer(std::move(timer)),
        m_ticks(std::make_shared<Beam::Queue<Beam::Timer::Result>>()),
        m_administration_client(std::move(administration_client)),
        m_notification_timer(std::move(notification_timer)),
        m_notification_delay(notification_delay),
        m_notification_ticks(
          std::make_shared<Beam::Queue<Beam::Timer::Result>>()),
        m_is_closed(false) {
    if(max_concurrency == 0) {
      throw std::invalid_argument("Report concurrency must be positive.");
    }
    if(notification_delay.is_special() || notification_delay.is_negative()) {
      throw std::invalid_argument("Invalid report notification delay.");
    }
    try {
      m_timer.get_publisher().monitor(m_ticks);
      m_retry_routine = Beam::spawn([&] { run_retries(); });
      m_notification_timer.get_publisher().monitor(m_notification_ticks);
      m_notification_routine = Beam::spawn([&] { run_notifications(); });
      for(auto i = std::size_t(0); i != max_concurrency; ++i) {
        m_routines.spawn([&] {
          run();
        });
      }
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  ReportJobService<B, T>::~ReportJobService() {
    close();
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  void ReportJobService<B, T>::mark_loaded(
      const Beam::DirectoryEntry& account, const ReportJob& job) {
    if(account != job.m_account ||
        (job.m_status != ReportJob::Status::COMPLETED &&
          job.m_status != ReportJob::Status::FAILED)) {
      return;
    }
    auto lock = std::lock_guard(m_mutex);
    auto i = m_notifications.find(job.m_id);
    if(i != m_notifications.end() &&
        i->second.m_job.m_completed == job.m_completed &&
        i->second.m_job.m_status == job.m_status) {
      m_notifications.erase(i);
    }
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  std::string ReportJobService<B, T>::submit(const ReportJob& submission) {
    auto entry = std::make_shared<Job>();
    entry->m_job = submission;
    auto& job = entry->m_job;
    job.m_id = boost::uuids::to_string(boost::uuids::random_generator()());
    job.m_created = m_time_client->get_time();
    job.m_modified = job.m_created;
    job.m_completed = boost::posix_time::not_a_date_time;
    job.m_status = ReportJob::Status::QUEUED;
    job.m_exit_code.reset();
    job.m_error.clear();
    auto id = job.m_id;
    auto lock = std::lock_guard(m_mutex);
    if(m_is_closed) {
      throw std::runtime_error("Report job service is closed.");
    }
    Beam::park([&] {
      m_backend->store(job);
    });
    enqueue(entry);
    return id;
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  void ReportJobService<B, T>::resume(const std::string& id) {
    auto lock = std::lock_guard(m_mutex);
    if(m_is_closed) {
      throw std::runtime_error("Report job service is closed.");
    }
    if(m_pending.contains(id)) {
      return;
    }
    auto job = Beam::park([&] { return m_backend->load_job(id); });
    if(!job) {
      return;
    }
    if(job->m_status == ReportJob::Status::RUNNING) {
      job->m_status = ReportJob::Status::FAILED;
      job->m_error = "Server stopped before report completion.";
      job->m_completed = m_time_client->get_time();
      job->m_modified = job->m_completed;
      Beam::park([&] { m_backend->store(*job); });
      schedule_notification(*job);
    } else if(job->m_status == ReportJob::Status::STAGED) {
      if(job->m_is_prepared) {
        job->m_status = ReportJob::Status::QUEUED;
      } else {
        job->m_status = ReportJob::Status::FAILED;
      }
      Beam::park([&] {
        m_backend->store(*job);
      });
      if(job->m_status == ReportJob::Status::FAILED) {
        schedule_notification(*job);
      }
    }
    if(job->m_status != ReportJob::Status::QUEUED) {
      return;
    }
    auto entry = std::make_shared<Job>();
    entry->m_job = std::move(*job);
    enqueue(entry);
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  template<typename V> requires std::invocable<V&, std::vector<ReportJob>&>
  void ReportJobService<B, T>::retry(const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids, V validate) {
    auto retry_lock = std::lock_guard(m_retry_mutex);
    auto jobs = [&] {
      auto lock = std::lock_guard(m_mutex);
      if(m_is_closed) {
        throw std::runtime_error("Report job service is closed.");
      }
      return load_jobs(account, ids);
    }();
    std::erase_if(jobs, [] (const auto& job) {
      return job.m_status != ReportJob::Status::FAILED;
    });
    if(jobs.empty()) {
      return;
    }
    Beam::park([&] {
      std::invoke(validate, jobs);
    });
    auto retry_ids = std::vector<std::string>();
    retry_ids.reserve(jobs.size());
    for(auto& job : jobs) {
      retry_ids.push_back(job.m_id);
    }
    auto lock = std::lock_guard(m_mutex);
    if(m_is_closed) {
      throw std::runtime_error("Report job service is closed.");
    }
    auto current = load_jobs(account, retry_ids);
    for(auto i = std::size_t(0); i != jobs.size(); ++i) {
      auto& job = jobs[i];
      if(current[i].m_status != ReportJob::Status::FAILED ||
          m_pending.contains(job.m_id)) {
        continue;
      }
      auto entry = std::make_shared<Job>();
      entry->m_previous_completion = current[i].m_completed;
      job.m_status = ReportJob::Status::QUEUED;
      job.m_modified = m_time_client->get_time();
      job.m_completed = boost::posix_time::not_a_date_time;
      job.m_exit_code.reset();
      job.m_error.clear();
      entry->m_job = std::move(job);
      Beam::park([&] {
        m_backend->store(entry->m_job);
      });
      m_results.erase(entry->m_job.m_id);
      m_notifications.erase(entry->m_job.m_id);
      enqueue(entry);
    }
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  void ReportJobService<B, T>::share(
      const Beam::DirectoryEntry& account, const std::vector<std::string>& ids,
      const std::vector<Beam::DirectoryEntry>& recipients) {
    auto lock = std::lock_guard(m_mutex);
    if(m_is_closed) {
      throw std::runtime_error("Report job service is closed.");
    }
    auto jobs = load_jobs(account, ids);
    auto is_unfinished = std::ranges::any_of(jobs, [] (const auto& job) {
      return job.m_status != ReportJob::Status::COMPLETED;
    });
    if(is_unfinished) {
      throw ReportNotFoundException();
    }
    for(auto& job : jobs) {
      auto count = job.m_recipients.size();
      for(auto& recipient : recipients) {
        if(!std::ranges::contains(job.m_recipients, recipient)) {
          job.m_recipients.push_back(recipient);
        }
      }
      if(job.m_recipients.size() != count) {
        Beam::park([&] {
          m_backend->store(job);
        });
        m_results.erase(job.m_id);
      }
    }
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  void ReportJobService<B, T>::cancel(const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids) {
    auto stops = std::vector<std::stop_source>();
    stops.reserve(ids.size());
    auto cancel = boost::scope::scope_exit([&] {
      for(auto& stop : stops) {
        stop.request_stop();
      }
    });
    auto lock = std::lock_guard(m_mutex);
    if(m_is_closed) {
      throw std::runtime_error("Report job service is closed.");
    }
    auto jobs = load_jobs(account, ids);
    for(auto& job : jobs) {
      if(job.m_status == ReportJob::Status::COMPLETED ||
          job.m_status == ReportJob::Status::CANCELLED) {
        continue;
      }
      job.m_status = ReportJob::Status::CANCELLED;
      job.m_completed = m_time_client->get_time();
      job.m_modified = job.m_completed;
      Beam::park([&] {
        m_backend->store(job);
      });
      m_results.erase(job.m_id);
      m_notifications.erase(job.m_id);
      auto i = m_pending.find(job.m_id);
      if(i != m_pending.end()) {
        i->second->m_job = std::move(job);
        stops.push_back(i->second->m_stop);
      }
    }
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  void ReportJobService<B, T>::remove(const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids) {
    auto lock = std::lock_guard(m_mutex);
    if(m_is_closed) {
      throw std::runtime_error("Report job service is closed.");
    }
    auto jobs = load_jobs(account, ids);
    auto is_unfinished = std::ranges::any_of(jobs, [] (const auto& job) {
      return job.m_status != ReportJob::Status::COMPLETED;
    });
    if(is_unfinished) {
      throw ReportNotFoundException();
    }
    for(auto& job : jobs) {
      Beam::park([&] {
        m_backend->remove(job.m_id);
      });
      m_results.erase(job.m_id);
      m_notifications.erase(job.m_id);
    }
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  void ReportJobService<B, T>::close() {
    auto stops = std::vector<std::stop_source>();
    {
      auto lock = std::lock_guard(m_mutex);
      if(m_is_closed) {
        return;
      }
      m_is_closed = true;
      m_timer.cancel();
      m_ticks->close();
      m_notification_timer.cancel();
      m_notification_ticks->close();
      m_notifications.clear();
      for(auto& [id, entry] : m_pending) {
        stops.push_back(entry->m_stop);
      }
      m_is_available.notify_all();
    }
    for(auto& stop : stops) {
      stop.request_stop();
    }
    m_routines.wait();
    m_retry_routine.wait();
    m_notification_routine.wait();
    auto lock = std::lock_guard(m_mutex);
    flush_results();
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  std::vector<ReportJob> ReportJobService<B, T>::load_jobs(
      const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids) {
    auto loaded = std::vector<std::optional<ReportJob>>();
    auto pending = std::vector<std::pair<std::size_t, const std::string*>>();
    auto seen = std::unordered_set<std::string>();
    for(auto& id : ids) {
      if(!seen.insert(id).second) {
        continue;
      }
      auto i = m_results.find(id);
      if(i != m_results.end()) {
        loaded.emplace_back(i->second.m_job);
      } else {
        pending.emplace_back(loaded.size(), &id);
        loaded.emplace_back();
      }
    }
    if(!pending.empty()) {
      Beam::park([&] {
        for(auto& [index, id] : pending) {
          loaded[index] = m_backend->load_job(*id);
        }
      });
    }
    auto jobs = std::vector<ReportJob>();
    jobs.reserve(loaded.size());
    for(auto& job : loaded) {
      if(!job || job->m_account != account ||
          account.m_type != Beam::DirectoryEntry::Type::ACCOUNT) {
        throw ReportNotFoundException();
      }
      jobs.push_back(std::move(*job));
    }
    return jobs;
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  bool ReportJobService<B, T>::store_result(PendingResult& result) {
    auto& job = result.m_job;
    try {
      if(job.m_completed.is_special()) {
        job.m_completed = m_time_client->get_time();
        if(!result.m_previous_completion.is_special()) {
          job.m_completed =
            std::max(job.m_completed, result.m_previous_completion +
              boost::posix_time::time_duration::unit());
        }
        job.m_modified = job.m_completed;
      }
      Beam::park([&] {
        m_backend->store(job);
      });
      return true;
    } catch(const std::exception&) {
      std::cerr << "Failed to store report job " << job.m_id <<
        " with status " << job.m_status << ".\n" <<
        BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
      return false;
    }
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  void ReportJobService<B, T>::flush_results() {
    for(auto i = m_results.begin(); i != m_results.end();) {
      if(store_result(i->second)) {
        schedule_notification(i->second.m_job);
        i = m_results.erase(i);
      } else {
        ++i;
      }
    }
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  void ReportJobService<B, T>::enqueue(const std::shared_ptr<Job>& entry) {
    m_pending.emplace(entry->m_job.m_id, entry);
    auto account = entry->m_job.m_account.m_id;
    auto [i, is_new] = m_jobs.try_emplace(account);
    i->second.push_back(entry);
    if(is_new) {
      m_accounts.push_back(account);
      m_is_available.notify_one();
    }
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  void ReportJobService<B, T>::finish(const std::shared_ptr<Job>& entry) {
    m_pending.erase(entry->m_job.m_id);
    auto account = entry->m_job.m_account.m_id;
    auto i = m_jobs.find(account);
    if(i->second.empty()) {
      m_jobs.erase(i);
    } else {
      m_accounts.push_back(account);
      m_is_available.notify_one();
    }
    if(m_is_closed && m_pending.empty()) {
      m_is_available.notify_all();
    }
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  void ReportJobService<B, T>::schedule_notification(const ReportJob& job) {
    if(m_is_closed) {
      return;
    }
    try {
      auto is_first = m_notifications.empty();
      m_notifications.insert_or_assign(job.m_id, PendingNotification(
        job, m_time_client->get_time() + m_notification_delay));
      if(is_first) {
        m_notification_timer.start();
      }
    } catch(const std::exception&) {
      m_notifications.erase(job.m_id);
      std::cerr << "Failed to schedule notification for report " << job.m_id <<
        ".\n" << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
    }
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  void ReportJobService<B, T>::run() {
    while(true) {
      auto entry = std::shared_ptr<Job>();
      {
        auto lock = std::unique_lock(m_mutex);
        while(m_accounts.empty()) {
          if(m_is_closed && m_pending.empty()) {
            return;
          }
          m_is_available.wait(lock);
        }
        auto account = m_accounts.front();
        m_accounts.pop_front();
        auto& queue = m_jobs.at(account);
        entry = queue.front();
        queue.pop_front();
      }
      auto job = ReportJob();
      auto stop = entry->m_stop.get_token();
      try {
        {
          auto lock = std::lock_guard(m_mutex);
          if(entry->m_job.m_status == ReportJob::Status::CANCELLED) {
            finish(entry);
            continue;
          }
          job = entry->m_job;
          if(m_is_closed) {
            throw std::runtime_error("Server stopped before report execution.");
          }
          job.m_status = ReportJob::Status::RUNNING;
          job.m_modified = m_time_client->get_time();
          Beam::park([&] {
            m_backend->store(job);
          });
          entry->m_job = job;
        }
        if(stop.stop_requested()) {
          throw std::runtime_error("Report execution stopped.");
        }
        {
          auto result = Beam::Async<int>();
          auto execution = std::jthread([&] {
            auto evaluation = result.get_eval();
            try {
              evaluation.set(m_backend->execute(job, stop));
            } catch(const std::exception&) {
              evaluation.set_exception(std::current_exception());
            }
          });
          job.m_exit_code = result.get();
        }
        if(stop.stop_requested()) {
          throw std::runtime_error("Report execution stopped.");
        }
        if(*job.m_exit_code != 0) {
          throw std::runtime_error("Report command exited with code " +
            std::to_string(*job.m_exit_code) + '.');
        }
        job.m_status = ReportJob::Status::COMPLETED;
      } catch(const std::exception& exception) {
        job.m_status = ReportJob::Status::FAILED;
        job.m_error = exception.what();
      }
      auto lock = std::lock_guard(m_mutex);
      auto complete = boost::scope::scope_exit([&] { finish(entry); });
      if(entry->m_job.m_status == ReportJob::Status::CANCELLED) {
        continue;
      }
      auto result = PendingResult(std::move(job), entry->m_previous_completion);
      if(store_result(result)) {
        schedule_notification(result.m_job);
      } else {
        auto is_first = m_results.empty();
        m_results.emplace(result.m_job.m_id, std::move(result));
        if(is_first && !m_is_closed) {
          m_timer.start();
        }
      }
    }
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  void ReportJobService<B, T>::run_retries() {
    while(true) {
      try {
        if(m_ticks->pop() == Beam::Timer::Result::CANCELED) {
          return;
        }
      } catch(const Beam::PipeBrokenException&) {
        return;
      }
      auto lock = std::lock_guard(m_mutex);
      if(m_is_closed) {
        return;
      }
      flush_results();
      if(!m_results.empty()) {
        m_timer.start();
      }
    }
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  void ReportJobService<B, T>::run_notifications() {
    while(true) {
      try {
        if(m_notification_ticks->pop() == Beam::Timer::Result::CANCELED) {
          return;
        }
      } catch(const Beam::PipeBrokenException&) {
        return;
      }
      auto jobs = std::vector<ReportJob>();
      {
        auto lock = std::lock_guard(m_mutex);
        if(m_is_closed) {
          return;
        }
        try {
          auto now = m_time_client->get_time();
          for(auto i = m_notifications.begin(); i != m_notifications.end();) {
            if(i->second.m_deadline <= now) {
              jobs.push_back(std::move(i->second.m_job));
              i = m_notifications.erase(i);
            } else {
              ++i;
            }
          }
        } catch(const std::exception&) {
          std::cerr << "Failed to check report notifications.\n" <<
            BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        }
        if(!m_notifications.empty()) {
          m_notification_timer.start();
        }
      }
      for(auto& job : jobs) {
        try {
          auto notification = make_report_notification(job);
          m_administration_client.send_notification(
            notification.m_account, notification.m_description,
            notification.m_data, notification.m_category);
        } catch(const std::exception&) {
          std::cerr << "Failed to send notification for report " << job.m_id <<
            ".\n" << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        }
      }
    }
  }
}

#endif
