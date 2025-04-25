#ifndef SPIRE_HPP
#define SPIRE_HPP
#include <concepts>
#include <boost/signals2/dummy_mutex.hpp>
#include <boost/signals2/signal_type.hpp>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/PortfolioController.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/MarketDataClientBox.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#ifdef _DEBUG
  #include <QDebug>
#endif

/** TODO legacy definitions. */
namespace Spire {
  class SpireServiceClients;
  using SpirePosition = Nexus::Accounting::Position<Nexus::Security>;
  using SpireInventory = Nexus::Accounting::Inventory<SpirePosition>;
  using SpireBookkeeper = Nexus::Accounting::TrueAverageBookkeeper<
    SpireInventory>;
  using SpirePortfolio = Nexus::Accounting::Portfolio<SpireBookkeeper>;
  using SpirePortfolioController = Nexus::Accounting::PortfolioController<
    SpirePortfolio, Nexus::MarketDataService::MarketDataClientBox>;
  class UserProfile;
}

namespace Spire {
  class AnyListModel;
  class AnyListValueModel;
  class AnyQueryModel;
  class AnyValueModel;
  class AnyRef;
  template<typename T> class ArrayListModel;
  class ArrayTableModel;
  template<typename T> class ArrayValueToListModel;
  template<typename T> class ColumnViewListModel;
  template<typename T> class CompositeModel;
  template<typename T> class ConstantValueModel;
  class Definitions;
  template<typename T> class EnumSetTestModel;
  class Field;
  template<typename T> class FieldValueModel;
  template<typename T> class FilteredListModel;
  class FilteredTableModel;
  class ListCurrentIndexModel;
  class ListIndexTracker;
  template<typename T> class ListIndexValueModel;
  template<typename T> class ListModel;
  template<typename T> class ListModelIterator;
  template<typename T> class ListModelReference;
  template<typename T> class ListToTableModel;
  template<typename T> class ListValueModel;
  template<typename T> class LocalQueryModel;
  template<typename T> class LocalScalarValueModel;
  class LocalTechnicalsModel;
  template<typename T> class LocalValueModel;
  template<typename T> class ModelTransactionLog;
  template<typename T> class OptionalScalarValueModelDecorator;
  template<typename T> class ProxyScalarValueModel;
  template<typename T> class ProxyValueModel;
  template<typename T> class QueryModel;
  template<typename T> class ReversedListModel;
  class RowView;
  template<typename T> class RowViewListModel;
  class Scalar;
  template<typename T> class ScalarValueModel;
  template<typename T> class ScalarValueModelDecorator;
  class SecurityDeck;
  class ServiceSecurityInfoQueryModel;
  template<typename T> class SortedListModel;
  class SortedTableModel;
  template<typename T> struct SubscriptionResult;
  class TableCurrentIndexModel;
  class TableModel;
  class TableRowIndexTracker;
  class TableToListModel;
  class TechnicalsModel;
  template<typename T> class ToDecimalModel;
  template<typename T> class ToTextModel;
  template<typename T, typename U, typename F, typename G>
    class TransformListModel;
  template<typename T, typename U, typename F, typename G>
    class TransformValueModel;
  template<typename T> class TranslatedListModel;
  class TranslatedTableModel;
  class UnsignedQuantityModel;
  template<typename T, std::invocable<const T&> F> class ValidatedValueModel;
  template<typename T> class ValueModel;

  /** Defines the common type of boost signal used throughout Spire. */
  template<typename F>
  using Signal = typename boost::signals2::signal_type<F,
    boost::signals2::keywords::mutex_type<boost::signals2::dummy_mutex>>::type;
}

#endif
