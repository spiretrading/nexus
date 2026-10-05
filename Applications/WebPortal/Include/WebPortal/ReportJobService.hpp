#ifndef NEXUS_REPORT_JOB_SERVICE_HPP
#define NEXUS_REPORT_JOB_SERVICE_HPP
#include <concepts>
#include <iostream>
#include <mutex>
#include <stop_token>
#include <utility>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/ThreadPool.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "WebPortal/ReportJob.hpp"

namespace Nexus::Details {

  /** Provides report job storage and execution. */
  template<typename T>
  concept IsReportJobBackend = requires(T& backend) {
    { backend.store(std::declval<const ReportJob&>()) } -> std::same_as<void>;
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

      /** Stops execution and marks unfinished jobs as failed. */
      void close();

    private:
      Backend* m_backend;
      Beam::local_ptr_t<TimeClient> m_time_client;
      mutable Beam::Mutex m_mutex;
      bool m_is_closed;
      std::stop_source m_stop_source;
      Beam::Queue<ReportJob> m_jobs;
      Beam::RoutineHandler m_routine;

      void run(std::stop_token stop);
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
      run(m_stop_source.get_token());
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
    auto buffer = Beam::from<Beam::SharedBuffer>(Beam::to_json(submission));
    auto receiver = Beam::JsonReceiver<Beam::SharedBuffer>();
    receiver.set(Beam::Ref(buffer));
    auto job = Beam::receive<ReportJob>(receiver);
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
    m_jobs.push(std::move(job));
    return id;
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  void ReportJobService<B, T>::close() {
    {
      auto lock = std::lock_guard(m_mutex);
      if(m_is_closed) {
        return;
      }
      m_is_closed = true;
      m_stop_source.request_stop();
      m_jobs.close();
    }
    m_routine.wait();
  }

  template<typename B, typename T> requires
    Beam::IsTimeClient<Beam::dereference_t<T>>
  void ReportJobService<B, T>::run(std::stop_token stop) {
    while(true) {
      auto job = ReportJob();
      try {
        job = m_jobs.pop();
      } catch(const Beam::PipeBrokenException&) {
        return;
      }
      try {
        if(stop.stop_requested()) {
          throw std::runtime_error("Server stopped before report execution.");
        }
        job.m_status = ReportJob::Status::RUNNING;
        job.m_modified = m_time_client->get_time();
        Beam::park([&] {
          m_backend->store(job);
        });
        job.m_exit_code = Beam::park([&] {
          return m_backend->execute(job, stop);
        });
        if(stop.stop_requested()) {
          throw std::runtime_error("Report execution stopped during shutdown.");
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
      job.m_completed = m_time_client->get_time();
      job.m_modified = job.m_completed;
      try {
        Beam::park([&] {
          m_backend->store(job);
        });
      } catch(const std::exception&) {
        std::cerr << "Failed to store report job " << job.m_id <<
          " with status " << job.m_status << ".\n" <<
          BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
      }
    }
  }
}

#endif
