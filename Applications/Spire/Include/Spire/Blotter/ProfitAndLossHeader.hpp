#ifndef SPIRE_PROFITANDLOSSHEADER_HPP
#define SPIRE_PROFITANDLOSSHEADER_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/connection.hpp>
#include <QWidget>
#include "Nexus/Definitions/Money.hpp"
#include "Spire/Blotter/Blotter.hpp"

class Ui_ProfitAndLossHeader;

namespace Spire {

  /*! \class ProfitAndLossHeader
      \brief Displays a single profit and loss header.
   */
  class ProfitAndLossHeader : public QWidget {
    public:

      //! Constructs a ProfitAndLossHeader.
      /*!
        \param model The profit and loss model to represent.
        \param parent The parent widget.
      */
      ProfitAndLossHeader(Beam::Ref<ProfitAndLossEntryModel> model,
        QWidget* parent = nullptr);

      virtual ~ProfitAndLossHeader();

    private:
      friend class ProfitAndLossEntry;
      std::unique_ptr<Ui_ProfitAndLossHeader> m_ui;
      ProfitAndLossEntryModel* m_model;
      boost::signals2::scoped_connection m_profitAndLossConnection;
      boost::signals2::scoped_connection m_volumeConnection;

      void OnProfitAndLossChanged(Nexus::Money profitAndLoss);
      void OnVolumeChanged(Nexus::Quantity volume);
  };
}

#endif
