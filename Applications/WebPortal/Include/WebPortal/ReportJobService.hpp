#ifndef NEXUS_REPORT_JOB_SERVICE_HPP
#define NEXUS_REPORT_JOB_SERVICE_HPP
#include <algorithm>
#include <concepts>
#include <functional>
#include <iostream>
#include <mutex>
#include <stop_token>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Serialization/ShuttleClone.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/ThreadPool.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/scope/scope_exit.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
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
   * Stores submitted reports and executes them in the background.
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
       */
      template<Beam::Initializes<T> TF>
      ReportJobService(Beam::Ref<Backend> backend, TF&& time_client) requires
        IsReportJobBackend<Backend>;

      ~ReportJobService();

      /** Stores a prepared job and returns its identifier without waiting. */
      std::string submit(const ReportJob& submission);

      /** Adds resolved recipients to completed reports owned by an account. */
      void share(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids,
        const std::vector<Beam::DirectoryEntry>& recipients);

      /** Deletes completed reports owned by an account. */
      void remove(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids);

      /** Cancels an account's jobs and removes them from activity. */
      void cancel(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids);

      /**
       * Requeues an account's failed jobs using their saved definitions.
       * @tparam V The validator for the failed job snapshots.
       * @param account The submitting account.
       * @param ids The job identifiers to retry.
       * @param validate Checks access to all failed jobs before any changes.
       */
      template<std::invocable<const std::vector<ReportJob>&> V>
      void retry(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids, V validate);

      /** Stops execution and marks unfinished jobs as failed. */
      void close();

    private:
      struct Job {
        ReportJob m_job;
        std::stop_source m_stop;
      };
      Backend* m_backend;
      Beam::local_ptr_t<TimeClient> m_time_client;
      mutable Beam::Mutex m_mutex;
      bool m_is_closed;
      std::unordered_map<std::string, std::shared_ptr<Job>> m_pending;
      Beam::Queue<std::shared_ptr<Job>> m_jobs;
      Beam::RoutineHandler m_routine;

      std::vector<ReportJob> load_jobs(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids);
      void run();
  };

  template<typename B, typename T>
  ReportJobService(Beam::Ref<B>, T&&) ->
    ReportJobService<B, std::remove_cvref_t<T>>;

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  template<Beam::Initializes<T> TF>
  ReportJobService<B, T>::ReportJobService(
      Beam::Ref<Backend> backend, TF&& time_client) requires
      IsReportJobBackend<Backend>
      : m_backend(backend.get()),
        m_time_client(std::forward<TF>(time_client)),
        m_is_closed(false) {
    m_routine = Beam::spawn([&] {
      run();
    });
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  ReportJobService<B, T>::~ReportJobService() {
    close();
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  std::string ReportJobService<B, T>::submit(const ReportJob& submission) {
    auto entry = std::make_shared<Job>();
    entry->m_job = Beam::shuttle_clone(submission);
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
    m_pending.emplace(id, entry);
    m_jobs.push(entry);
    return id;
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
      auto entry = m_pending.find(job.m_id);
      if(entry != m_pending.end()) {
        entry->second->m_job = std::move(job);
        stops.push_back(entry->second->m_stop);
      }
    }
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  template<std::invocable<const std::vector<ReportJob>&> V>
  void ReportJobService<B, T>::retry(const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids, V validate) {
    auto lock = std::lock_guard(m_mutex);
    if(m_is_closed) {
      throw std::runtime_error("Report job service is closed.");
    }
    auto jobs = load_jobs(account, ids);
    std::erase_if(jobs, [] (const auto& job) {
      return job.m_status != ReportJob::Status::FAILED;
    });
    if(jobs.empty()) {
      return;
    }
    Beam::park([&] {
      std::invoke(validate, std::as_const(jobs));
    });
    for(auto& job : jobs) {
      auto entry = std::make_shared<Job>();
      job.m_status = ReportJob::Status::QUEUED;
      job.m_modified = m_time_client->get_time();
      job.m_completed = boost::posix_time::not_a_date_time;
      job.m_exit_code.reset();
      job.m_error.clear();
      entry->m_job = std::move(job);
      Beam::park([&] {
        m_backend->store(entry->m_job);
      });
      m_pending.emplace(entry->m_job.m_id, entry);
      m_jobs.push(entry);
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
      for(auto& [id, entry] : m_pending) {
        stops.push_back(entry->m_stop);
      }
      m_jobs.close();
    }
    for(auto& stop : stops) {
      stop.request_stop();
    }
    m_routine.wait();
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  std::vector<ReportJob> ReportJobService<B, T>::load_jobs(
      const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids) {
    auto jobs = std::vector<ReportJob>();
    auto seen = std::unordered_set<std::string>();
    for(auto& id : ids) {
      if(!seen.insert(id).second) {
        continue;
      }
      auto job = Beam::park([&] {
        return m_backend->load_job(id);
      });
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
  void ReportJobService<B, T>::run() {
    while(true) {
      auto entry = std::shared_ptr<Job>();
      try {
        entry = m_jobs.pop();
      } catch(const Beam::PipeBrokenException&) {
        return;
      }
      auto job = ReportJob();
      auto stop = entry->m_stop.get_token();
      try {
        {
          auto lock = std::lock_guard(m_mutex);
          if(entry->m_job.m_status == ReportJob::Status::CANCELLED) {
            m_pending.erase(entry->m_job.m_id);
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
      if(entry->m_job.m_status == ReportJob::Status::CANCELLED) {
        m_pending.erase(job.m_id);
        continue;
      }
      try {
        job.m_completed = m_time_client->get_time();
        job.m_modified = job.m_completed;
        Beam::park([&] {
          m_backend->store(job);
        });
      } catch(const std::exception&) {
        std::cerr << "Failed to store report job " << job.m_id <<
          " with status " << job.m_status << ".\n" <<
          BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
      }
      m_pending.erase(job.m_id);
    }
  }
}

#endif
