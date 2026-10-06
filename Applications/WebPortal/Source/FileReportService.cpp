#include "WebPortal/FileReportService.hpp"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <limits>
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
#include "WebPortal/ReportAccess.hpp"

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

  std::filesystem::path output_path(
      const ReportJob& job, const std::filesystem::path& directory) {
    static const auto PATTERN = std::regex("[A-Za-z0-9][A-Za-z0-9._-]*");
    auto& extension = job.m_definition.m_output.m_extension;
    if(!std::regex_match(extension, PATTERN)) {
      throw std::runtime_error("Invalid report output extension.");
    }
    return job_directory(job, directory) / ("output." + extension);
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

GeneratedReports FileReportService::load_reports(
    const DirectoryEntry& account, const GeneratedReportQuery& query) {
  m_open_state.ensure_open();
  return query_generated_reports(load_jobs(), account, query, m_client);
}

ReportFile FileReportService::load_file(
    const DirectoryEntry& account, const std::string& id) {
  m_open_state.ensure_open();
  auto job = load_job(id);
  auto access = ReportAccess(account, m_client);
  if(!job || !access.is_accessible(*job)) {
    throw ReportNotFoundException();
  }
  auto output = output_path(*job, m_jobs_directory);
  auto expected =
    std::filesystem::canonical(m_jobs_directory) / id / output.filename();
  auto error = std::error_code();
  auto path = std::filesystem::canonical(output, error);
  if(error || path != expected || !std::filesystem::is_regular_file(path)) {
    throw ReportNotFoundException();
  }
  auto stream = std::ifstream(path, std::ios::binary | std::ios::ate);
  if(!stream) {
    throw std::runtime_error("Unable to open report output.");
  }
  auto size = static_cast<std::streamoff>(stream.tellg());
  if(size < 0 || static_cast<std::uintmax_t>(size) >
      std::numeric_limits<std::size_t>::max() || size >
      std::numeric_limits<std::streamsize>::max()) {
    throw std::runtime_error("Invalid report output size.");
  }
  auto content = SharedBuffer();
  content.grow(static_cast<std::size_t>(size));
  stream.exceptions(std::ios::failbit | std::ios::badbit);
  stream.seekg(0);
  if(size != 0) {
    stream.read(content.get_mutable_data(), static_cast<std::streamsize>(size));
  }
  return ReportFile(make_report_filename(*job),
    job->m_definition.m_output.m_media_type, std::move(content));
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

void FileReportService::cancel(
    const DirectoryEntry& account, const std::vector<std::string>& ids) {
  m_open_state.ensure_open();
  m_jobs->cancel(account, ids);
}

void FileReportService::retry(
    const DirectoryEntry& account, const std::vector<std::string>& ids) {
  m_open_state.ensure_open();
  m_jobs->retry(account, ids, [&] (const auto& jobs) {
    validate_report_retries(jobs, load_definitions(account), m_client);
  });
}

std::optional<ReportJob> FileReportService::load_job(const std::string& id) {
  auto job = ReportJob();
  job.m_id = id;
  return read_job(job_directory(job, m_jobs_directory));
}

void FileReportService::store(const ReportJob& job) {
  auto path = job_directory(job, m_jobs_directory);
  auto text = to_json(job);
  auto lock = std::lock_guard(m_mutex);
  if(job.m_status == ReportJob::Status::QUEUED) {
    std::filesystem::create_directories(path);
  }
  auto temporary = path / "metadata.json.tmp";
  auto stream = std::ofstream();
  stream.exceptions(std::ios::failbit | std::ios::badbit);
  stream.open(temporary, std::ios::binary | std::ios::trunc);
  stream << text;
  stream.close();
#ifdef _WIN32
  if(std::filesystem::exists(path / "metadata.json")) {
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
  auto output = output_path(job, m_jobs_directory);
  if(stop.stop_requested()) {
    throw std::runtime_error("Report execution stopped.");
  }
  auto diagnostics = output.parent_path() / "diagnostics.txt";
  for(auto& file : {output, diagnostics}) {
    auto stream = std::ofstream();
    stream.exceptions(std::ios::failbit | std::ios::badbit);
    stream.open(file, std::ios::binary | std::ios::trunc);
    stream.close();
  }
  auto context = io_context();
  auto group = ProcessGroup();
  auto launcher = default_process_launcher();
#ifdef _WIN32
  launcher.creation_flags |= CREATE_SUSPENDED;
#endif
  auto child = launcher(context.get_executor(), job.m_definition.m_command,
    job.m_arguments, process_stdio(
      nullptr, filesystem::path(output.native()),
      filesystem::path(diagnostics.native()))
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
    throw std::runtime_error("Report execution stopped.");
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
    if(auto job = read_job(entry.path())) {
      jobs.push_back(std::move(*job));
    }
  }
  return jobs;
}

std::optional<ReportJob> FileReportService::read_job(
    const std::filesystem::path& path) {
  auto text = std::string();
  {
    auto lock = std::lock_guard(m_mutex);
    if(!std::filesystem::exists(path / "metadata.json")) {
      return std::nullopt;
    }
    auto stream = std::ifstream(path / "metadata.json", std::ios::binary);
    stream.exceptions(std::ios::badbit);
    if(!stream) {
      throw std::runtime_error("Unable to read report job metadata.");
    }
    text.assign(std::istreambuf_iterator<char>(stream), {});
  }
  auto job = from_json<ReportJob>(text);
  if(job_directory(job, m_jobs_directory) != path) {
    throw std::runtime_error("Report job directory does not match its id.");
  }
  return job;
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
