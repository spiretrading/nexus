#ifndef SPIRE_SALE_CONDITION_LIST_ITEM_HPP
#define SPIRE_SALE_CONDITION_LIST_ITEM_HPP
#include <QWidget>
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Stores info about the sale condition. */
  struct SaleConditionInfo {

    /** The sale condition represented. */
    Nexus::TimeAndSale::Condition m_condition;

    /** The name for the condition. */
    QString m_name;
  };

  /** Represents a sale condition list item in the SaleConditionBox. */
  class SaleConditionListItem : public QWidget {
    public:

      /**
       * Constructs a SaleConditionListItem.
       * @param condition_info The condition to be displayed in the
       *        SaleConditionListItem.
       * @param parent The parent widget.
       */
      explicit SaleConditionListItem(SaleConditionInfo condition_info,
        QWidget* parent = nullptr);

      /** Returns the sale condition. */
      const SaleConditionInfo& get_condition() const;

    private:
      SaleConditionInfo m_condition_info;
  };
}

#endif
