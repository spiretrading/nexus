#ifndef SPIRE_BUILT_IN_CATALOG_ENTRY_HPP
#define SPIRE_BUILT_IN_CATALOG_ENTRY_HPP
#include <vector>
#include "Spire/Catalog/CatalogEntry.hpp"

namespace Spire {

  /** CatalogEntry for a built-in CanvasNode. */
  class BuiltInCatalogEntry : public CatalogEntry {
    public:
      static const boost::uuids::uuid ABS_UUID;
      static const boost::uuids::uuid ADDITION_UUID;
      static const boost::uuids::uuid AGGREGATE_TASK_UUID;
      static const boost::uuids::uuid ALARM_UUID;
      static const boost::uuids::uuid ASK_ORDER_TASK_UUID;
      static const boost::uuids::uuid BBO_QUOTE_QUERY_UUID;
      static const boost::uuids::uuid BID_ORDER_TASK_UUID;
      static const boost::uuids::uuid BOOLEAN_UUID;
      static const boost::uuids::uuid CEIL_UUID;
      static const boost::uuids::uuid CHAIN_UUID;
      static const boost::uuids::uuid COUNT_UUID;
      static const boost::uuids::uuid CURRENCY_UUID;
      static const boost::uuids::uuid CURRENT_DATE_UUID;
      static const boost::uuids::uuid CURRENT_DATE_TIME_UUID;
      static const boost::uuids::uuid CURRENT_TIME_UUID;
      static const boost::uuids::uuid DATE_TIME_UUID;
      static const boost::uuids::uuid DECIMAL_UUID;
      static const boost::uuids::uuid DEFAULT_CURRENCY_UUID;
      static const boost::uuids::uuid DESTINATION_UUID;
      static const boost::uuids::uuid DISTINCT_UUID;
      static const boost::uuids::uuid DIVISION_UUID;
      static const boost::uuids::uuid DURATION_UUID;
      static const boost::uuids::uuid EQUAL_UUID;
      static const boost::uuids::uuid EXECUTION_REPORT_MONITOR_UUID;
      static const boost::uuids::uuid EXECUTION_REPORT_RECORD_UUID;
      static const boost::uuids::uuid FILE_PATH_UUID;
      static const boost::uuids::uuid FILE_READER_UUID;
      static const boost::uuids::uuid FILTER_UUID;
      static const boost::uuids::uuid FIRST_UUID;
      static const boost::uuids::uuid FLOOR_UUID;
      static const boost::uuids::uuid FOLD_UUID;
      static const boost::uuids::uuid GREATER_EQUAL_UUID;
      static const boost::uuids::uuid GREATER_UUID;
      static const boost::uuids::uuid IF_UUID;
      static const boost::uuids::uuid INTEGER_UUID;
      static const boost::uuids::uuid IS_TERMINAL_UUID;
      static const boost::uuids::uuid LAST_UUID;
      static const boost::uuids::uuid LESS_EQUAL_UUID;
      static const boost::uuids::uuid LESS_UUID;
      static const boost::uuids::uuid LIMIT_ASK_ORDER_TASK_UUID;
      static const boost::uuids::uuid LIMIT_BID_ORDER_TASK_UUID;
      static const boost::uuids::uuid LIMIT_ORDER_TASK_UUID;
      static const boost::uuids::uuid LUA_SCRIPT_UUID;
      static const boost::uuids::uuid MARKET_ASK_ORDER_TASK_UUID;
      static const boost::uuids::uuid MARKET_BID_ORDER_TASK_UUID;
      static const boost::uuids::uuid MARKET_ORDER_IMBALANCE_UUID;
      static const boost::uuids::uuid MARKET_ORDER_TASK_UUID;
      static const boost::uuids::uuid MARKET_UUID;
      static const boost::uuids::uuid MAX_FLOOR_UUID;
      static const boost::uuids::uuid MAX_UUID;
      static const boost::uuids::uuid MIN_UUID;
      static const boost::uuids::uuid MONEY_UUID;
      static const boost::uuids::uuid MULTIPLICATION_UUID;
      static const boost::uuids::uuid NONE_UUID;
      static const boost::uuids::uuid NOT_EQUAL_UUID;
      static const boost::uuids::uuid NOT_UUID;
      static const boost::uuids::uuid OPTIONAL_PRICE_UUID;
      static const boost::uuids::uuid ORDER_FIELDS_RECORD_UUID;
      static const boost::uuids::uuid ORDER_STATUS_UUID;
      static const boost::uuids::uuid ORDER_TYPE_UUID;
      static const boost::uuids::uuid PORTFOLIO_ENTRY_RECORD_UUID;
      static const boost::uuids::uuid PREVIOUS_UUID;
      static const boost::uuids::uuid QUERY_UUID;
      static const boost::uuids::uuid QUOTE_RECORD_UUID;
      static const boost::uuids::uuid RANGE_UUID;
      static const boost::uuids::uuid REFERENCE_UUID;
      static const boost::uuids::uuid ROUND_UUID;
      static const boost::uuids::uuid SECURITY_PORTFOLIO_UUID;
      static const boost::uuids::uuid SECURITY_UUID;
      static const boost::uuids::uuid SIDE_UUID;
      static const boost::uuids::uuid SINGLE_ORDER_TASK_UUID;
      static const boost::uuids::uuid SPAWN_TASK_UUID;
      static const boost::uuids::uuid SUBTRACTION_UUID;
      static const boost::uuids::uuid TASK_STATE_UUID;
      static const boost::uuids::uuid TASK_STATE_MONITOR_UUID;
      static const boost::uuids::uuid TEXT_UUID;
      static const boost::uuids::uuid TIME_AND_SALE_QUERY_UUID;
      static const boost::uuids::uuid TIME_IN_FORCE_UUID;
      static const boost::uuids::uuid TIME_RANGE_PARAMETER_UUID;
      static const boost::uuids::uuid TIMER_UUID;
      static const boost::uuids::uuid UNTIL_TASK_UUID;
      static const boost::uuids::uuid WHEN_TASK_UUID;

      /** Loads the list of BuiltInCatalogEntries. */
      static std::vector<std::unique_ptr<CatalogEntry>>
        LoadBuiltInCatalogEntries();

      /**
       * Returns the meta-data value used to indicate the CanvasNode's source.
       */
      static const std::string& GetBuiltInSourceValue();

      /**
       * Constructs a BuiltInCatalogEntry.
       * @param name The name of the entry.
       * @param uid The entry's unique id.
       * @param iconPath The path to the entry's icon.
       * @param description The description of the entry.
       * @param node The CanvasNode this entry represents.
       */
      BuiltInCatalogEntry(const std::string& name,
        const boost::uuids::uuid& uid, const std::string& iconPath,
        const std::string& description, const CanvasNode& node);

      /**
       * Constructs a BuiltInCatalogEntry.
       * @param uid The entry's unique id.
       * @param iconPath The path to the entry's icon.
       * @param description The description of the entry.
       * @param node The CanvasNode this entry represents.
       */
      BuiltInCatalogEntry(const boost::uuids::uuid& uid,
        const std::string& iconPath, const std::string& description,
        const CanvasNode& node);

      std::string GetName() const override;

      std::unique_ptr<CatalogEntry>
        SetName(const std::string& name) const override;

      const std::string& GetIconPath() const override;

      std::unique_ptr<CatalogEntry>
        SetIconPath(const std::string& iconPath) const override;

      QIcon GetIcon() const override;

      std::string GetDescription() const override;

      const CanvasNode& GetNode() const override;

      std::unique_ptr<CatalogEntry>
        SetNode(const CanvasNode& node) const override;

      bool IsReadOnly() const override;

    private:
      std::string m_name;
      std::string m_iconPath;
      QIcon m_icon;
      std::string m_description;
      std::unique_ptr<CanvasNode> m_node;
  };
}

#endif
