#include "Spire/SignIn/UpdateDownloadChannelFactory.hpp"
#include <boost/optional/optional.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/WebServices/TcpChannelFactory.hpp>

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::WebServices;
using namespace boost;
using namespace boost::chrono;
using namespace boost::posix_time;
using namespace Spire;

namespace {
  struct ProgressReader {
    std::shared_ptr<ProgressModel> m_download_progress;
    std::shared_ptr<ValueModel<time_duration>> m_time_left;
    std::size_t m_download_size;
    ReaderBox* m_reader;
    std::size_t m_total_read_bytes;
    optional<steady_clock::time_point> m_start_time;
    steady_clock::time_point m_last_read_time;
    std::size_t m_last_block;

    explicit ProgressReader(std::shared_ptr<ProgressModel> download_progress,
      std::shared_ptr<ValueModel<time_duration>> time_left,
      std::size_t download_size, ReaderBox& reader)
      : m_download_progress(std::move(download_progress)),
        m_time_left(std::move(time_left)),
        m_download_size(download_size),
        m_reader(&reader),
        m_total_read_bytes(0),
        m_last_block(0) {}

    bool IsDataAvailable() const {
      return m_reader->IsDataAvailable();
    }

    template<typename R>
    std::size_t Read(Out<R> destination) {
      return update_download_progress(m_reader->Read(Store(destination)));
    }

    std::size_t Read(char* destination, std::size_t size) {
      return update_download_progress(m_reader->Read(destination, size));
    }

    template<typename R>
    std::size_t Read(Out<R> destination, std::size_t size) {
      return update_download_progress(m_reader->Read(Store(destination), size));
    }

    std::size_t update_download_progress(std::size_t size) {
      const auto BLOCK_SIZE = 1024 * 1024;
      m_total_read_bytes += size;
      if(!m_start_time) {
        m_last_read_time = steady_clock::now();
        m_start_time = m_last_read_time;
      }
      auto last_duration = steady_clock::now() - m_last_read_time;
      if(m_total_read_bytes - m_last_block > BLOCK_SIZE &&
          last_duration >= chrono::seconds(3)) {
        auto progress = std::min<std::size_t>(99, std::max<std::size_t>(
          1, (100 * m_total_read_bytes) / m_download_size));
        auto elapsed_time = steady_clock::now() - *m_start_time;
        auto total_time =
          elapsed_time / (static_cast<double>(progress) / 100.0);
        auto remaining_time =
          posix_time::milliseconds(duration_cast<chrono::milliseconds>(
            total_time - elapsed_time).count());
        m_time_left->set(remaining_time);
        m_download_progress->set(progress);
        m_last_block = m_total_read_bytes;
        m_last_read_time = steady_clock::now();
      }
      return size;
    }
  };
}

namespace Beam {
  template<>
  struct ImplementsConcept<ProgressReader, IO::Reader> : std::true_type {};
}

UpdateDownloadChannelFactory::UpdateDownloadChannelFactory(
  std::size_t download_size, std::shared_ptr<ProgressModel> download_progress,
  std::shared_ptr<ValueModel<posix_time::time_duration>> time_left)
  : m_download_size(download_size),
    m_download_progress(std::move(download_progress)),
    m_time_left(std::move(time_left)) {}

std::unique_ptr<ChannelBox>
    UpdateDownloadChannelFactory::operator ()(const Uri& uri) const {
  auto tcp_channel = TcpSocketChannelFactory()(uri);
  auto reader = std::make_unique<ProgressReader>(m_download_progress,
    m_time_left, m_download_size, tcp_channel->GetReader());
  auto progress_channel = std::make_unique<ChannelBox>(
    std::in_place_type<WrapperChannel<std::unique_ptr<ChannelBox>,
      std::unique_ptr<ProgressReader>>>, std::move(tcp_channel),
      std::move(reader));
  m_download_progress->set(0);
  return progress_channel;
}
