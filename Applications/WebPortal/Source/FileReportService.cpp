#include "WebPortal/FileReportService.hpp"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <regex>
#include <system_error>
#include <unordered_set>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/process/process.hpp>
#include <boost/process/stdio.hpp>
#ifdef _WIN32
  #include <boost/process/windows/show_window.hpp>
#else
  #include <cerrno>
  #include <csignal>
  #include <unistd.h>
#endif

using namespace Beam;
using namespace boost;
using namespace boost::asio;
using namespace boost::posix_time;
using namespace boost::process;
using namespace Nexus;

namespace {
  class ProcessGroup {
    public:
#ifdef _WIN32
      ProcessGroup()
          : m_handle(CreateJobObjectW(nullptr, nullptr)) {
        if(!m_handle) {
          throw std::system_error(GetLastError(), std::system_category());
        }
        auto limits = JOBOBJECT_EXTENDED_LIMIT_INFORMATION();
        limits.BasicLimitInformation.LimitFlags =
          JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        if(!SetInformationJobObject(m_handle,
            JobObjectExtendedLimitInformation, &limits, sizeof(limits))) {
          auto error = GetLastError();
          CloseHandle(m_handle);
          throw std::system_error(error, std::system_category());
        }
      }

      ~ProcessGroup() {
        CloseHandle(m_handle);
      }

      void attach(auto& child) {
        if(!AssignProcessToJobObject(m_handle, child.native_handle())) {
          throw std::system_error(GetLastError(), std::system_category());
        }
        child.resume();
      }

      void terminate() {
        TerminateJobObject(m_handle, 1);
      }
#else
      ProcessGroup() noexcept
        : m_id(-1) {}

      ~ProcessGroup() {
        terminate();
      }

      void attach(auto& child) {
        m_id = child.id();
      }

      void terminate() {
        if(m_id > 0) {
          kill(-m_id, SIGKILL);
        }
      }

      system::error_code on_exec_setup(
          boost::process::posix::default_launcher& launcher,
          const filesystem::path& executable, const char* const* arguments) {
        if(setsid() == -1) {
          return system::error_code(errno, system::generic_category());
        }
        return {};
      }
#endif

    private:
#ifdef _WIN32
      HANDLE m_handle;
#else
      pid_t m_id;
#endif

      ProcessGroup(const ProcessGroup&) = delete;
      ProcessGroup& operator =(const ProcessGroup&) = delete;
  };

  std::filesystem::path job_directory(
      const ReportJob& job, const std::filesystem::path& directory) {
    static const auto PATTERN = std::regex(
      "[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}");
    if(!std::regex_match(job.m_id, PATTERN)) {
      throw std::invalid_argument("Invalid report job identifier.");
    }
    return directory / job.m_id;
  }
}

FileReportService::FileReportService(
    std::filesystem::path definitions_directory,
    std::filesystem::path jobs_directory, ServiceLocatorClient client,
    TimeClient time_client)
    : m_definitions_directory(std::filesystem::absolute(definitions_directory)),
      m_jobs_directory(std::filesystem::absolute(jobs_directory)),
      m_client(std::move(client)),
      m_time_client(std::move(time_client)) {
  try {
    recover();
    m_jobs = std::make_unique<
      Nexus::Details::ReportJobService<FileReportService, TimeClient>>(
        Ref(*this), m_time_client);
  } catch(const std::exception&) {
    m_open_state.close();
    throw;
  }
}

FileReportService::~FileReportService() {
  close();
}

ReportActivities FileReportService::load_activities(
    const DirectoryEntry& account, const ReportActivityQuery& query) {
  m_open_state.ensure_open();
  return query_report_activities(load_jobs(), account, query);
}

std::vector<ReportDefinition> FileReportService::load_definitions(
    const DirectoryEntry& account) {
  m_open_state.ensure_open();
  return filter_report_definitions(load_definitions(), account, m_client);
}

std::string FileReportService::submit(const DirectoryEntry& account,
    const ReportSubmission& submission) {
  m_open_state.ensure_open();
  return m_jobs->submit(
    prepare_report_job(load_definitions(), account, submission, m_client));
}

void FileReportService::store(const ReportJob& job) {
  auto path = job_directory(job, m_jobs_directory);
  auto text = to_json(job);
  auto lock = std::lock_guard(m_mutex);
  if(job.m_status == ReportJob::Status::QUEUED) {
    std::filesystem::create_directories(m_jobs_directory);
    if(!std::filesystem::create_directory(path)) {
      throw std::runtime_error("Report job already exists.");
    }
  }
  auto temporary = path / "metadata.json.tmp";
  auto stream = std::ofstream();
  stream.exceptions(std::ios::failbit | std::ios::badbit);
  stream.open(temporary, std::ios::binary | std::ios::trunc);
  stream << text;
  stream.close();
#ifdef _WIN32
  if(job.m_status != ReportJob::Status::QUEUED) {
    if(!ReplaceFileW((path / "metadata.json").c_str(), temporary.c_str(),
        nullptr, 0, nullptr, nullptr)) {
      throw std::system_error(GetLastError(), std::system_category());
    }
    return;
  }
#endif
  std::filesystem::rename(temporary, path / "metadata.json");
}

int FileReportService::execute(const ReportJob& job, std::stop_token stop) {
  auto path = job_directory(job, m_jobs_directory);
  auto& extension = job.m_definition.m_output.m_extension;
  static const auto PATTERN = std::regex("[A-Za-z0-9][A-Za-z0-9._-]*");
  if(!std::regex_match(extension, PATTERN)) {
    throw std::runtime_error("Invalid report output extension.");
  }
  if(stop.stop_requested()) {
    throw std::runtime_error("Server stopped before report execution.");
  }
  auto context = io_context();
  auto group = ProcessGroup();
  auto launcher = default_process_launcher();
#ifdef _WIN32
  launcher.creation_flags |= CREATE_SUSPENDED;
#endif
  auto child = launcher(context.get_executor(), job.m_definition.m_command,
    job.m_arguments, process_stdio(
      nullptr, filesystem::path((path / ("output." + extension)).native()),
      filesystem::path((path / "diagnostics.txt").native()))
#ifdef _WIN32
    , boost::process::windows::show_window_hide
#else
    , group
#endif
  );
  group.attach(child);
  auto result = -1;
  auto error = system::error_code();
  child.async_wait([&] (auto code, auto exit_code) {
    error = code;
    result = exit_code;
  });
  auto cancel = std::stop_callback(stop, [&] {
    post(context, [&] {
      group.terminate();
    });
  });
  context.run();
  if(stop.stop_requested()) {
    throw std::runtime_error(
      "Report execution stopped during server shutdown.");
  }
  if(error) {
    throw system::system_error(error);
  }
  return result;
}

void FileReportService::close() {
  if(m_open_state.set_closing()) {
    return;
  }
  m_jobs->close();
  m_open_state.close();
}

std::vector<ReportDefinition> FileReportService::load_definitions() {
  if(!std::filesystem::exists(m_definitions_directory)) {
    return {};
  }
  auto files = std::vector<std::filesystem::path>();
  for(auto& entry :
      std::filesystem::directory_iterator(m_definitions_directory)) {
    auto extension = entry.path().extension();
    if(entry.is_regular_file() &&
        (extension == ".yml" || extension == ".yaml")) {
      files.push_back(entry.path());
    }
  }
  std::ranges::sort(files);
  auto definitions = std::vector<ReportDefinition>();
  auto identifiers = std::unordered_set<std::string>();
  for(auto& file : files) {
    auto definition = try_or_nest([&] {
      auto stream = std::ifstream(file);
      if(!stream) {
        throw std::runtime_error("Unable to open report definition.");
      }
      return load_report_definition(stream);
    }, std::runtime_error(
      "Failed to load report definition: " + file.string()));
    if(!identifiers.insert(definition.m_id).second) {
      throw std::runtime_error("Duplicate report identifier: " +
        definition.m_id + " in " + file.string());
    }
    definitions.push_back(std::move(definition));
  }
  return definitions;
}

std::vector<ReportJob> FileReportService::load_jobs() {
  auto jobs = std::vector<ReportJob>();
  if(!std::filesystem::exists(m_jobs_directory)) {
    return jobs;
  }
  for(auto& entry : std::filesystem::directory_iterator(m_jobs_directory)) {
    if(!entry.is_directory()) {
      continue;
    }
    auto text = std::string();
    {
      auto lock = std::lock_guard(m_mutex);
      if(!std::filesystem::exists(entry.path() / "metadata.json")) {
        continue;
      }
      auto stream =
        std::ifstream(entry.path() / "metadata.json", std::ios::binary);
      stream.exceptions(std::ios::badbit);
      if(!stream) {
        throw std::runtime_error("Unable to read report job metadata.");
      }
      text.assign(std::istreambuf_iterator<char>(stream), {});
    }
    auto buffer = from<SharedBuffer>(text);
    auto receiver = JsonReceiver<SharedBuffer>();
    receiver.set(Ref(buffer));
    auto job = receive<ReportJob>(receiver);
    if(job_directory(job, m_jobs_directory) != entry.path()) {
      throw std::runtime_error("Report job directory does not match its id.");
    }
    jobs.push_back(std::move(job));
  }
  return jobs;
}

void FileReportService::recover() {
  for(auto& job : load_jobs()) {
    if(job.m_status == ReportJob::Status::QUEUED ||
        job.m_status == ReportJob::Status::RUNNING) {
      job.m_status = ReportJob::Status::FAILED;
      job.m_error = "Server stopped before report completion.";
      job.m_completed = m_time_client.get_time();
      job.m_modified = job.m_completed;
      store(job);
    }
  }
}
