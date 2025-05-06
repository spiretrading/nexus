#ifndef SPIRE_UPDATE_DOWNLOAD_CHANNEL_FACTORY_HPP
#define SPIRE_UPDATE_DOWNLOAD_CHANNEL_FACTORY_HPP
#include <memory>
#include <Beam/IO/ChannelBox.hpp>
#include <Beam/WebServices/Uri.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Spire/SignIn/SignIn.hpp"
#include "Spire/Ui/ProgressBar.hpp"

namespace Spire {

  /**
   * Builds a Channel suitable for downloading Spire updates. Will notify the
   * download progress and estimated time remaining.
   */
  class UpdateDownloadChannelFactory {
    public:

      /**
       * Constructs an UpdateDownloadChannelFactory.
       * @param download_size The size of the update in bytes.
       * @param download_progress Used to set the download progress.
       * @param time_left Used to set the estimated time left to download the
       *        update.
       */
      UpdateDownloadChannelFactory(std::size_t download_size,
        std::shared_ptr<ProgressModel> download_progress,
        std::shared_ptr<ValueModel<boost::posix_time::time_duration>>
          time_left);

      std::unique_ptr<Beam::IO::ChannelBox> operator ()(
        const Beam::WebServices::Uri& uri) const;

    private:
      std::size_t m_download_size;
      std::shared_ptr<ProgressModel> m_download_progress;
      std::shared_ptr<ValueModel<boost::posix_time::time_duration>> m_time_left;
  };
}

#endif
