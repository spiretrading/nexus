#ifndef NEXUS_REPORT_PROCESS_GROUP_HPP
#define NEXUS_REPORT_PROCESS_GROUP_HPP
#include <algorithm>
#include <array>
#include <charconv>
#include <filesystem>
#include <limits>
#include <system_error>
#include <boost/process/process.hpp>
#ifdef _WIN32
  #include <boost/process/windows/show_window.hpp>
#else
  #include <cerrno>
  #include <csignal>
  #include <fcntl.h>
  #include <pthread.h>
  #include <sys/socket.h>
  #include <sys/wait.h>
  #include <unistd.h>
  #ifdef __linux__
    #include <sys/syscall.h>
  #endif
#endif

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
      ProcessGroup()
          : m_descriptor(-1),
            m_watchdog(-1) {
        auto maximum = sysconf(_SC_OPEN_MAX);
        auto error = std::error_code();
        auto i = std::filesystem::directory_iterator("/proc/self/fd", error);
        if(error) {
          error.clear();
          i = std::filesystem::directory_iterator("/dev/fd", error);
        }
        while(!error && i != std::filesystem::directory_iterator()) {
          auto name = i->path().filename().string();
          auto descriptor = int();
          auto result = std::from_chars(
            name.data(), name.data() + name.size(), descriptor);
          if(result.ec == std::errc() && maximum >= 0) {
            maximum = std::max(maximum, static_cast<long>(descriptor) + 1);
          }
          i.increment(error);
        }
        if(error) {
          maximum = -1;
        }
        auto descriptors = std::array<int, 2>();
#ifdef SOCK_CLOEXEC
        auto type = SOCK_STREAM | SOCK_CLOEXEC;
#else
        auto type = SOCK_STREAM;
#endif
        if(socketpair(AF_UNIX, type, 0, descriptors.data()) == -1) {
          throw std::system_error(errno, std::system_category());
        }
        try {
          for(auto& descriptor : descriptors) {
            if(descriptor <= STDERR_FILENO) {
              auto replacement = fcntl(
                descriptor, F_DUPFD_CLOEXEC, STDERR_FILENO + 1);
              if(replacement == -1) {
                throw std::system_error(errno, std::system_category());
              }
              ::close(descriptor);
              descriptor = replacement;
            }
#ifndef SOCK_CLOEXEC
            if(fcntl(descriptor, F_SETFD, FD_CLOEXEC) == -1) {
              throw std::system_error(errno, std::system_category());
            }
#endif
          }
          auto signals = sigset_t();
          auto previous = sigset_t();
          sigfillset(&signals);
          auto error = pthread_sigmask(SIG_BLOCK, &signals, &previous);
          if(error != 0) {
            throw std::system_error(error, std::system_category());
          }
          m_watchdog = fork();
          error = errno;
          if(m_watchdog == 0) {
            watch(descriptors[1], maximum);
          }
          pthread_sigmask(SIG_SETMASK, &previous, nullptr);
          if(m_watchdog == -1) {
            throw std::system_error(error, std::system_category());
          }
        } catch(...) {
          ::close(descriptors[0]);
          ::close(descriptors[1]);
          throw;
        }
        m_descriptor = descriptors[0];
        ::close(descriptors[1]);
        auto ready = char();
        auto count = ssize_t();
        do {
          count = read(m_descriptor, &ready, sizeof(ready));
        } while(count == -1 && errno == EINTR);
        if(count != static_cast<ssize_t>(sizeof(ready)) || ready != 1) {
          terminate();
          ::close(m_descriptor);
          reap();
          throw std::runtime_error("Unable to start report process watchdog.");
        }
      }

      ~ProcessGroup() {
        terminate();
        ::close(m_descriptor);
        reap();
      }

      void terminate() {
        shutdown(m_descriptor, SHUT_WR);
      }

      boost::system::error_code on_exec_setup(
          boost::process::posix::default_launcher& launcher,
          const boost::filesystem::path& executable,
          const char* const* arguments) {
        if(setpgid(0, m_watchdog) == -1) {
          return boost::system::error_code(
            errno, boost::system::generic_category());
        }
        ::close(m_descriptor);
        return {};
      }
#endif

    private:
#ifdef _WIN32
      HANDLE m_handle;
#else
      int m_descriptor;
      pid_t m_watchdog;

      static void watch(int descriptor, long maximum) {
        if(setpgid(0, 0) == -1 || dup2(descriptor, STDIN_FILENO) == -1) {
          _exit(1);
        }
        auto is_closed = false;
#if defined(__linux__) && defined(SYS_close_range)
        is_closed = syscall(SYS_close_range, STDIN_FILENO + 1,
          std::numeric_limits<unsigned int>::max(), 0) == 0;
#endif
        if(!is_closed) {
          if(maximum < 0) {
            _exit(1);
          }
          for(auto i = long(STDIN_FILENO + 1); i < maximum; ++i) {
            ::close(static_cast<int>(i));
          }
        }
        auto ready = char(1);
        auto count = ssize_t();
        do {
          count = write(STDIN_FILENO, &ready, sizeof(ready));
        } while(count == -1 && errno == EINTR);
        if(count == static_cast<ssize_t>(sizeof(ready))) {
          do {
            count = read(STDIN_FILENO, &ready, sizeof(ready));
          } while(count > 0 || (count == -1 && errno == EINTR));
        }
        kill(-getpid(), SIGKILL);
        _exit(1);
      }
#endif

      ProcessGroup(const ProcessGroup&) = delete;
      ProcessGroup& operator =(const ProcessGroup&) = delete;
#ifndef _WIN32
      void reap() {
        auto status = int();
        while(waitpid(m_watchdog, &status, 0) == -1 && errno == EINTR) {}
      }
#endif
  };
}

#endif
