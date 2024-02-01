#ifndef SPIRE_PROFITANDLOSSTABLE_HPP
#define SPIRE_PROFITANDLOSSTABLE_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/LegacyUI/LegacyUI.hpp"
#include "Spire/Spire/Spire.hpp"

class QModelIndex;
class Ui_ProfitAndLossTable;

namespace Spire {

  /*! \class ProfitAndLossTable
      \brief Displays the profit and loss table.
   */
  class ProfitAndLossTable : public QWidget {
    public:

      //! Constructs a ProfitAndLossTable.
      /*!
        \param userProfile The user's profile.
        \param model The profit and loss model to represent.
        \param parent The parent widget.
      */
      ProfitAndLossTable(Beam::Ref<UserProfile> userProfile,
        Beam::Ref<ProfitAndLossEntryModel> model,
        QWidget* parent = nullptr);

      virtual ~ProfitAndLossTable();

    private:
      std::unique_ptr<Ui_ProfitAndLossTable> m_ui;
      ProfitAndLossEntryModel* m_model;
      LegacyUI::CustomVariantSortFilterProxyModel* m_proxyModel;

      void OnHeaderResized(int logicalIndex, int oldSize, int newSize);
      void OnRowCountChanged(const QModelIndex& parent, int start, int end);
  };
}

#endif
