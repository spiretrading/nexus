#include "Spire/Canvas/Operations/CanvasNodeTranslator.hpp"
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/Parsers/ParserPublisher.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Reactors/AggregateReactor.hpp>
#include <Beam/Reactors/AlarmReactor.hpp>
#include <Beam/Reactors/BasicReactor.hpp>
#include <Beam/Reactors/ChainReactor.hpp>
#include <Beam/Reactors/ConstantReactor.hpp>
#include <Beam/Reactors/Expressions.hpp>
#include <Beam/Reactors/FilterReactor.hpp>
#include <Beam/Reactors/FoldReactor.hpp>
#include <Beam/Reactors/LuaReactor.hpp>
#include <Beam/Reactors/NativeLuaReactorParameter.hpp>
#include <Beam/Reactors/NoneReactor.hpp>
#include <Beam/Reactors/PublisherReactor.hpp>
#include <Beam/Reactors/QueueReactor.hpp>
#include <Beam/Reactors/RangeReactor.hpp>
#include <Beam/Reactors/Reactor.hpp>
#include <Beam/Reactors/ReactorError.hpp>
#include <Beam/Reactors/StaticReactor.hpp>
#include <Beam/Reactors/SwitchReactor.hpp>
#include <Beam/Reactors/ThrowReactor.hpp>
#include <Beam/Reactors/TimerReactor.hpp>
#include <Beam/Tasks/AggregateTask.hpp>
#include <Beam/Tasks/IdleTask.hpp>
#include <Beam/Tasks/ReactorTask.hpp>
#include <Beam/Tasks/SpawnTask.hpp>
#include <Beam/Tasks/UntilTask.hpp>
#include <Beam/Tasks/WhenTask.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include <Beam/Utilities/DateTime.hpp>
#include <Beam/Utilities/Math.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/include/copy.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/include/transform.hpp>
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"
#include "Nexus/Tasks/OrderWrapperTask.hpp"
#include "Nexus/Tasks/SingleOrderTask.hpp"
#include "Nexus/Tasks/SingleRedisplayableOrderTask.hpp"
#include "Spire/Canvas/Common/BreadthFirstCanvasNodeIterator.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/CustomNode.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Common/SignatureNode.hpp"
#include "Spire/Canvas/ControlNodes/AggregateNode.hpp"
#include "Spire/Canvas/ControlNodes/ChainNode.hpp"
#include "Spire/Canvas/ControlNodes/SpawnNode.hpp"
#include "Spire/Canvas/ControlNodes/UntilNode.hpp"
#include "Spire/Canvas/ControlNodes/WhenNode.hpp"
#include "Spire/Canvas/IONodes/FilePathNode.hpp"
#include "Spire/Canvas/IONodes/FileReaderNode.hpp"
#include "Spire/Canvas/LuaNodes/LuaInterop.hpp"
#include "Spire/Canvas/LuaNodes/LuaScriptNode.hpp"
#include "Spire/Canvas/LuaNodes/RecordLuaReactorParameter.hpp"
#include "Spire/Canvas/MarketDataNodes/BboQuoteNode.hpp"
#include "Spire/Canvas/MarketDataNodes/OrderImbalanceQueryNode.hpp"
#include "Spire/Canvas/MarketDataNodes/TimeAndSaleQueryNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/DefaultCurrencyNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/ExecutionReportMonitorNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderWrapperTaskNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SecurityPortfolioNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/Records/QueryNode.hpp"
#include "Spire/Canvas/Records/RecordNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ProxyNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/StandardNodes/AbsNode.hpp"
#include "Spire/Canvas/StandardNodes/AdditionNode.hpp"
#include "Spire/Canvas/StandardNodes/AlarmNode.hpp"
#include "Spire/Canvas/StandardNodes/CeilNode.hpp"
#include "Spire/Canvas/StandardNodes/ComparisonSignatures.hpp"
#include "Spire/Canvas/StandardNodes/CurrentDateNode.hpp"
#include "Spire/Canvas/StandardNodes/CurrentDateTimeNode.hpp"
#include "Spire/Canvas/StandardNodes/CurrentTimeNode.hpp"
#include "Spire/Canvas/StandardNodes/DivisionNode.hpp"
#include "Spire/Canvas/StandardNodes/EqualsNode.hpp"
#include "Spire/Canvas/StandardNodes/FilterNode.hpp"
#include "Spire/Canvas/StandardNodes/FloorNode.hpp"
#include "Spire/Canvas/StandardNodes/FoldNode.hpp"
#include "Spire/Canvas/StandardNodes/FoldOperandNode.hpp"
#include "Spire/Canvas/StandardNodes/GreaterNode.hpp"
#include "Spire/Canvas/StandardNodes/GreaterOrEqualsNode.hpp"
#include "Spire/Canvas/StandardNodes/IfNode.hpp"
#include "Spire/Canvas/StandardNodes/LesserNode.hpp"
#include "Spire/Canvas/StandardNodes/LesserOrEqualsNode.hpp"
#include "Spire/Canvas/StandardNodes/MathSignatures.hpp"
#include "Spire/Canvas/StandardNodes/MaxNode.hpp"
#include "Spire/Canvas/StandardNodes/MinNode.hpp"
#include "Spire/Canvas/StandardNodes/MultiplicationNode.hpp"
#include "Spire/Canvas/StandardNodes/NotNode.hpp"
#include "Spire/Canvas/StandardNodes/RangeNode.hpp"
#include "Spire/Canvas/StandardNodes/RoundNode.hpp"
#include "Spire/Canvas/StandardNodes/StaticNode.hpp"
#include "Spire/Canvas/StandardNodes/SubtractionNode.hpp"
#include "Spire/Canvas/StandardNodes/TimeRangeParameterNode.hpp"
#include "Spire/Canvas/StandardNodes/TimerNode.hpp"
#include "Spire/Canvas/StandardNodes/UnequalNode.hpp"
#include "Spire/Canvas/SystemNodes/BlotterTaskMonitorNode.hpp"
#include "Spire/Canvas/SystemNodes/InteractionsNode.hpp"
#include "Spire/Canvas/TaskNodes/CanvasNodeTask.hpp"
#include "Spire/Canvas/TaskNodes/IndirectTask.hpp"
#include "Spire/Canvas/TaskNodes/IsTerminalNode.hpp"
#include "Spire/Canvas/TaskNodes/OrderExecutionPublisherTask.hpp"
#include "Spire/Canvas/TaskNodes/TaskStateMonitorNode.hpp"
#include "Spire/Canvas/Types/ExecutionReportRecordType.hpp"
#include "Spire/Canvas/Types/OrderImbalanceRecordType.hpp"
#include "Spire/Canvas/Types/ParserTypes.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/TaskType.hpp"
#include "Spire/Canvas/Types/TimeAndSaleRecordType.hpp"
#include "Spire/Canvas/ValueNodes/BooleanNode.hpp"
#include "Spire/Canvas/ValueNodes/CurrencyNode.hpp"
#include "Spire/Canvas/ValueNodes/DateTimeNode.hpp"
#include "Spire/Canvas/ValueNodes/DecimalNode.hpp"
#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/ValueNodes/DurationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MarketNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderStatusNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderTypeNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TaskStateNode.hpp"
#include "Spire/Canvas/ValueNodes/TextNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeRangeNode.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"
#include "Spire/Spire/ServiceClients.hpp"
#include "Spire/Spire/UserProfile.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Beam::Queries;
using namespace Beam::Reactors;
using namespace Beam::Tasks;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tasks;
using namespace Spire;
using namespace std;

namespace {
  class CanvasNodeTranslationVisitor : private CanvasNodeVisitor {
    public:
      CanvasNodeTranslationVisitor(
        RefType<CanvasNodeTranslationContext> context,
        RefType<const CanvasNode> node);

      Translation Translate();
      virtual void Visit(const AbsNode& node);
      virtual void Visit(const AdditionNode& node);
      virtual void Visit(const AggregateNode& node);
      virtual void Visit(const AlarmNode& node);
      virtual void Visit(const BlotterTaskMonitorNode& node);
      virtual void Visit(const BooleanNode& node);
      virtual void Visit(const CanvasNode& node);
      virtual void Visit(const CeilNode& node);
      virtual void Visit(const ChainNode& node);
      virtual void Visit(const CurrencyNode& node);
      virtual void Visit(const CurrentDateNode& node);
      virtual void Visit(const CurrentDateTimeNode& node);
      virtual void Visit(const CurrentTimeNode& node);
      virtual void Visit(const CustomNode& node);
      virtual void Visit(const DateTimeNode& node);
      virtual void Visit(const DecimalNode& node);
      virtual void Visit(const DefaultCurrencyNode& node);
      virtual void Visit(const DestinationNode& node);
      virtual void Visit(const DivisionNode& node);
      virtual void Visit(const DurationNode& node);
      virtual void Visit(const EqualsNode& node);
      virtual void Visit(const ExecutionReportMonitorNode& node);
      virtual void Visit(const FilePathNode& node);
      virtual void Visit(const FileReaderNode& node);
      virtual void Visit(const FilterNode& node);
      virtual void Visit(const FloorNode& node);
      virtual void Visit(const FoldNode& node);
      virtual void Visit(const FoldOperandNode& node);
      virtual void Visit(const GreaterNode& node);
      virtual void Visit(const GreaterOrEqualsNode& node);
      virtual void Visit(const IfNode& node);
      virtual void Visit(const IntegerNode& node);
      virtual void Visit(const IsTerminalNode& node);
      virtual void Visit(const LesserNode& node);
      virtual void Visit(const LesserOrEqualsNode& node);
      virtual void Visit(const LuaScriptNode& node);
      virtual void Visit(const MarketNode& node);
      virtual void Visit(const MaxFloorNode& node);
      virtual void Visit(const MaxNode& node);
      virtual void Visit(const MinNode& node);
      virtual void Visit(const MoneyNode& node);
      virtual void Visit(const MultiplicationNode& node);
      virtual void Visit(const NoneNode& node);
      virtual void Visit(const NotNode& node);
      virtual void Visit(const OptionalPriceNode& node);
      virtual void Visit(const OrderImbalanceQueryNode& node);
      virtual void Visit(const OrderStatusNode& node);
      virtual void Visit(const OrderTypeNode& node);
      virtual void Visit(const OrderWrapperTaskNode& node);
      virtual void Visit(const QueryNode& node);
      virtual void Visit(const RangeNode& node);
      virtual void Visit(const ReferenceNode& node);
      virtual void Visit(const RoundNode& node);
      virtual void Visit(const SecurityNode& node);
      virtual void Visit(const SideNode& node);
      virtual void Visit(const SingleOrderTaskNode& node);
      virtual void Visit(const SpawnNode& node);
      virtual void Visit(const StaticNode& node);
      virtual void Visit(const SubtractionNode& node);
      virtual void Visit(const TaskStateMonitorNode& node);
      virtual void Visit(const TaskStateNode& node);
      virtual void Visit(const TextNode& node);
      virtual void Visit(const TimeAndSaleQueryNode& node);
      virtual void Visit(const TimeInForceNode& node);
      virtual void Visit(const TimeNode& node);
      virtual void Visit(const TimeRangeNode& node);
      virtual void Visit(const TimeRangeParameterNode& node);
      virtual void Visit(const TimerNode& node);
      virtual void Visit(const UnequalNode& node);
      virtual void Visit(const UntilNode& node);
      virtual void Visit(const WhenNode& node);

    private:
      CanvasNodeTranslationContext* m_context;
      const CanvasNode* m_node;
      Translation m_translation;

      Translation InternalTranslation(const CanvasNode& node);
      template<typename Translator>
      Translation TranslateFunction(const CanvasNode& node);
  };

  template<typename Translator>
  struct ParameterCount {
    static const int value = boost::mpl::size<typename boost::mpl::front<
      typename Translator::SupportedTypes>::type>::value - 1;
  };

  template<typename Translator, std::size_t N>
  struct FunctionTranslator {};

  template<typename Translator>
  struct FunctionTranslator<Translator, 1> {
    Translation operator ()(
        const vector<std::shared_ptr<BaseReactor>>& parameters,
        const std::type_info& result,
        CanvasNodeTranslationContext& context) const {
      return Instantiate<Translator>(parameters[0]->GetType(), result)(
        parameters[0], context);
    }
  };

  template<typename Translator>
  struct FunctionTranslator<Translator, 2> {
    Translation operator ()(
        const vector<std::shared_ptr<BaseReactor>>& parameters,
        const std::type_info& result,
        CanvasNodeTranslationContext& context) const {
      return Instantiate<Translator>(parameters[0]->GetType(),
        parameters[1]->GetType(), result)(parameters[0], parameters[1],
        context);
    }
  };

  template<typename Translator>
  struct FunctionTranslator<Translator, 3> {
    Translation operator ()(
        const vector<std::shared_ptr<BaseReactor>>& parameters,
        const std::type_info& result,
        CanvasNodeTranslationContext& context) const {
      return Instantiate<Translator>(parameters[0]->GetType(),
        parameters[1]->GetType(), parameters[2]->GetType(), result)(
        parameters[0], parameters[1], parameters[2], context);
    }
  };

  struct AbsTranslator {
    template<typename T, typename R>
    struct Operation {
      R operator()(const T& arg) const {
        return std::abs(arg);
      }
    };

    template<>
    struct Operation<Nexus::Money, Nexus::Money> {
      Nexus::Money operator ()(const Nexus::Money& arg) const {
        return Nexus::Abs(arg);
      }
    };

    template<typename T, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& value,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T, R>(),
        std::static_pointer_cast<Reactor<T>>(value));
    }

    using SupportedTypes = AbsNodeSignatures::type;
  };

  struct AdditionTranslator {
    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left + right;
      }
    };

    template<typename T0, typename T1, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& left,
        const std::shared_ptr<BaseReactor>& right,
        CanvasNodeTranslationContext& context) {
      return Add(std::static_pointer_cast<Reactor<T0>>(left),
        std::static_pointer_cast<Reactor<T1>>(right));
    }

    using SupportedTypes = AdditionNodeSignatures::type;
  };

  struct CeilTranslator {
    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator ()(const T0& value, const T1& places) const {
        return Beam::Ceil(value, static_cast<int>(places));
      }
    };

    template<>
    struct Operation<Money, Quantity, Money> {
      Money operator ()(const Money& value, const Quantity& places) const {
        return Nexus::Ceil(value, places);
      }
    };

    template<typename T0, typename T1, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& left,
        const std::shared_ptr<BaseReactor>& right,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, T1, R>(),
        std::static_pointer_cast<Reactor<T0>>(left),
        std::static_pointer_cast<Reactor<T1>>(right));
    }

    using SupportedTypes = RoundingNodeSignatures::type;
  };

  struct ChainTranslator {
    template<typename T>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& initial,
        const std::shared_ptr<BaseReactor>& continuation,
        CanvasNodeTranslationContext& context) {
      return MakeChainReactor(std::static_pointer_cast<Reactor<T>>(initial),
        std::static_pointer_cast<Reactor<T>>(continuation),
        Ref(context.GetReactorMonitor().GetTrigger()));
    }

    using SupportedTypes = ValueTypes;
  };

  struct DivisionTranslator {
    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator ()(const T0& left, const T1& right) const {
        if(right == 0) {
          BOOST_THROW_EXCEPTION(ReactorError("Division by 0."));
        }
        return left / right;
      }
    };

    template<>
    struct Operation<Nexus::Money, Nexus::Money, double> {
      double operator ()(const Nexus::Money& left,
          const Nexus::Money& right) const {
        if(right == Nexus::Money::ZERO) {
          BOOST_THROW_EXCEPTION(ReactorError("Division by 0."));
        }
        return left / right;
      }
    };

    template<>
    struct Operation<boost::posix_time::time_duration,
        boost::posix_time::time_duration, double> {
      double operator ()(const boost::posix_time::time_duration& left,
          const boost::posix_time::time_duration& right) const {
        if(right.ticks() == 0) {
          BOOST_THROW_EXCEPTION(ReactorError("Division by 0."));
        }
        return static_cast<double>(left.ticks()) / right.ticks();
      }
    };

    template<typename T0, typename T1, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& left,
        const std::shared_ptr<BaseReactor>& right,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, T1, R>(),
        std::static_pointer_cast<Reactor<T0>>(left),
        std::static_pointer_cast<Reactor<T1>>(right));
    }

    using SupportedTypes = DivisionNodeSignatures::type;
  };

  struct EqualsTranslator {
    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left == right;
      }
    };

    template<typename T0, typename T1, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& left,
        const std::shared_ptr<BaseReactor>& right,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, T1, R>(),
        std::static_pointer_cast<Reactor<T0>>(left),
        std::static_pointer_cast<Reactor<T1>>(right));
    }

    using SupportedTypes = EqualitySignatures::type;
  };

  struct FileReaderTranslator {
    template<typename Parser>
    static auto BuildParser(const Parser& parser) ->
        decltype(tokenize >> parser >> ('\n' | eps_p)) {
      return tokenize >> parser >> ('\n' | eps_p);
    }

    template<typename T>
    static std::shared_ptr<BaseReactor> Template(const NativeType& nativeType,
        RefType<ReactorMonitor> reactorMonitor,
        RefType<UserProfile> userProfile, ParserErrorPolicy errorPolicy,
        const string& path) {
      using BaseParser = ParserType<T>::type;
      auto parser = BuildParser(BaseParser());
      using Parser = decltype(parser);
      auto publisher = std::make_shared<
        ParserPublisher<BasicIStreamReader<ifstream>, Parser>>(path, parser,
        errorPolicy);
      auto reactor = MakePublisherReactor(std::move(publisher),
        Ref(reactorMonitor->GetTrigger()));
      return reactor;
    }

    template<>
    static std::shared_ptr<BaseReactor> Template<CurrencyId>(
        const NativeType& nativeType, RefType<ReactorMonitor> reactorMonitor,
        RefType<UserProfile> userProfile, ParserErrorPolicy errorPolicy,
        const string& path) {
      using BaseParser = ParserType<CurrencyId>::type;
      auto parser = BuildParser(BaseParser(userProfile->GetCurrencyDatabase()));
      using Parser = decltype(parser);
      auto publisher = std::make_shared<
        ParserPublisher<BasicIStreamReader<ifstream>, Parser>>(path, parser,
        errorPolicy);
      auto reactor = MakePublisherReactor(std::move(publisher),
        Ref(reactorMonitor->GetTrigger()));
      return reactor;
    }

    template<>
    static std::shared_ptr<BaseReactor> Template<MarketCode>(
        const NativeType& nativeType, RefType<ReactorMonitor> reactorMonitor,
        RefType<UserProfile> userProfile, ParserErrorPolicy errorPolicy,
        const string& path) {
      using BaseParser = ParserType<MarketCode>::type;
      auto parser = BuildParser(BaseParser(userProfile->GetMarketDatabase()));
      using Parser = decltype(parser);
      auto publisher = std::make_shared<
        ParserPublisher<BasicIStreamReader<ifstream>, Parser>>(path, parser,
        errorPolicy);
      auto reactor = MakePublisherReactor(std::move(publisher),
        Ref(reactorMonitor->GetTrigger()));
      return reactor;
    }

    template<>
    static std::shared_ptr<BaseReactor> Template<Record>(
        const NativeType& nativeType, RefType<ReactorMonitor> reactorMonitor,
        RefType<UserProfile> userProfile, ParserErrorPolicy errorPolicy,
        const string& path) {
      using BaseParser = ParserType<Record>::type;
      auto parser = BuildParser(BaseParser(
        static_cast<const RecordType&>(nativeType), Ref(userProfile)));
      using Parser = decltype(parser);
      auto publisher = std::make_shared<
        ParserPublisher<BasicIStreamReader<ifstream>, Parser>>(path, parser,
        errorPolicy);
      auto reactor = MakePublisherReactor(std::move(publisher),
        Ref(reactorMonitor->GetTrigger()));
      return reactor;
    }

    template<>
    static std::shared_ptr<BaseReactor> Template<Security>(
        const NativeType& nativeType, RefType<ReactorMonitor> reactorMonitor,
        RefType<UserProfile> userProfile, ParserErrorPolicy errorPolicy,
        const string& path) {
      using BaseParser = ParserType<Security>::type;
      auto parser = BuildParser(BaseParser(userProfile->GetMarketDatabase()));
      using Parser = decltype(parser);
      auto publisher = std::make_shared<
        ParserPublisher<BasicIStreamReader<ifstream>, Parser>>(path, parser,
        errorPolicy);
      auto reactor = MakePublisherReactor(std::move(publisher),
        Ref(reactorMonitor->GetTrigger()));
      return reactor;
    }

    using SupportedTypes = ValueTypes;
  };

  struct FilterTranslator {
    template<typename T>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<Reactor<bool>>& filter,
        const std::shared_ptr<BaseReactor>& source) {
      return MakeFilterReactor(filter,
        std::static_pointer_cast<Reactor<T>>(source));
    }

    using SupportedTypes = ValueTypes;
  };

  struct FloorTranslator {
    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator ()(const T0& value, const T1& places) const {
        return Beam::Floor(value, static_cast<int>(places));
      }
    };

    template<>
    struct Operation<Money, Quantity, Money> {
      Money operator ()(const Money& value, const Quantity& places) const {
        return Nexus::Floor(value, places);
      }
    };

    template<typename T0, typename T1, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& left,
        const std::shared_ptr<BaseReactor>& right,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, T1, R>(),
        std::static_pointer_cast<Reactor<T0>>(left),
        std::static_pointer_cast<Reactor<T1>>(right));
    }

    using SupportedTypes = RoundingNodeSignatures::type;
  };

  struct FoldParameterTranslator {
    template<typename T>
    static std::shared_ptr<BaseReactor> Template() {
      return MakeFoldParameterReactor<T>();
    }

    using SupportedTypes = ValueTypes;
  };

  struct FoldTranslator {
    template<typename SourceType, typename CombinerType, typename Unused>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& source,
        const std::shared_ptr<BaseReactor>& combiner,
        const std::shared_ptr<BaseReactor>& leftTrigger,
        const std::shared_ptr<BaseReactor>& rightTrigger) {
      return MakeFoldReactor(
        std::static_pointer_cast<Reactor<SourceType>>(source),
        std::static_pointer_cast<Reactor<CombinerType>>(combiner),
        std::static_pointer_cast<FoldParameterReactor<CombinerType>>(
          leftTrigger),
        std::static_pointer_cast<FoldParameterReactor<SourceType>>(rightTrigger));
    }

    using SupportedTypes = FoldSignatures::type;
  };

  struct GreaterTranslator {
    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left > right;
      }
    };

    template<typename T0, typename T1, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& left,
        const std::shared_ptr<BaseReactor>& right,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, T1, R>(),
        std::static_pointer_cast<Reactor<T0>>(left),
        std::static_pointer_cast<Reactor<T1>>(right));
    }

    using SupportedTypes = ComparisonSignatures::type;
  };

  struct GreaterOrEqualsTranslator {
    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left >= right;
      }
    };

    template<typename T0, typename T1, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& left,
        const std::shared_ptr<BaseReactor>& right,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, T1, R>(),
        std::static_pointer_cast<Reactor<T0>>(left),
        std::static_pointer_cast<Reactor<T1>>(right));
    }

    using SupportedTypes = ComparisonSignatures::type;
  };

  struct IfTranslator {
    template<typename T0, typename T1, typename T2, typename R>
    struct Operation {
      R operator()(const T0& condition, const T1& consequent,
          const T2& default) const {
        return condition ? consequent : default;
      }
    };

    template<typename T0, typename T1, typename T2, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& condition,
        const std::shared_ptr<BaseReactor>& consequent,
        const std::shared_ptr<BaseReactor>& default,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, T1, T2, R>(),
        std::static_pointer_cast<Reactor<T0>>(condition),
        std::static_pointer_cast<Reactor<T1>>(consequent),
        std::static_pointer_cast<Reactor<T2>>(default));
    }

    using SupportedTypes = IfNodeSignatures::type;
  };

  struct LesserTranslator {
    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left < right;
      }
    };

    template<typename T0, typename T1, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& left,
        const std::shared_ptr<BaseReactor>& right,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, T1, R>(),
        std::static_pointer_cast<Reactor<T0>>(left),
        std::static_pointer_cast<Reactor<T1>>(right));
    }

    using SupportedTypes = ComparisonSignatures::type;
  };

  struct LesserOrEqualsTranslator {
    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left <= right;
      }
    };

    template<typename T0, typename T1, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& left,
        const std::shared_ptr<BaseReactor>& right,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, T1, R>(),
        std::static_pointer_cast<Reactor<T0>>(left),
        std::static_pointer_cast<Reactor<T1>>(right));
    }

    using SupportedTypes = ComparisonSignatures::type;
  };

  struct LuaParameterTranslator {
    template<typename T>
    static std::unique_ptr<LuaReactorParameter> Template(
        const std::shared_ptr<BaseReactor>& reactor, const CanvasType& type) {
      return MakeNativeLuaReactorParameter(
        std::static_pointer_cast<Reactor<T>>(reactor));
    }

    template<>
    static std::unique_ptr<LuaReactorParameter> Template<Record>(
        const std::shared_ptr<BaseReactor>& reactor, const CanvasType& type) {
      return make_unique<RecordLuaReactorParameter>(
        std::static_pointer_cast<Reactor<Record>>(reactor),
        static_cast<const RecordType&>(type));
    }

    using SupportedTypes = ValueTypes;
  };

  struct LuaScriptTranslator {
    template<typename T>
    static std::shared_ptr<BaseReactor> Template(string name,
        vector<unique_ptr<LuaReactorParameter>> parameters,
        lua_State& luaState) {
      return MakeLuaReactor<T>(std::move(name), std::move(parameters),
        luaState);
    }

    using SupportedTypes = ValueTypes;
  };

  struct MaxTranslator {
    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return std::max(left, right);
      }
    };

    template<typename T0, typename T1, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& left,
        const std::shared_ptr<BaseReactor>& right,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, T1, R>(),
        std::static_pointer_cast<Reactor<T0>>(left),
        std::static_pointer_cast<Reactor<T1>>(right));
    }

    using SupportedTypes = ExtremaNodeSignatures::type;
  };

  struct MinTranslator {
    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return std::max(left, right);
      }
    };

    template<typename T0, typename T1, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& left,
        const std::shared_ptr<BaseReactor>& right,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, T1, R>(),
        std::static_pointer_cast<Reactor<T0>>(left),
        std::static_pointer_cast<Reactor<T1>>(right));
    }

    using SupportedTypes = ExtremaNodeSignatures::type;
  };

  struct MultiplicationTranslator {
    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left * right;
      }
    };

    template<>
    struct Operation<Money, Quantity, Money> {
      Money operator()(const Money& left, const Quantity& right) const {
        return right * left;
      }
    };

    template<>
    struct Operation<Money, double, Money> {
      Money operator()(const Money& left, const double& right) const {
        return right * left;
      }
    };

    template<>
    struct Operation<Quantity, time_duration, time_duration> {
      time_duration operator()(const Quantity& left,
          const time_duration& right) const {
        return right * left;
      }
    };

    template<typename T0, typename T1, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& left,
        const std::shared_ptr<BaseReactor>& right,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, T1, R>(),
        std::static_pointer_cast<Reactor<T0>>(left),
        std::static_pointer_cast<Reactor<T1>>(right));
    }

    using SupportedTypes = MultiplicationNodeSignatures::type;
  };

  struct NoneTranslator {
    template<typename T>
    static std::shared_ptr<BaseReactor> Template() {
      return std::make_shared<NoneReactor<T>>();
    }

    using SupportedTypes = ValueTypes;
  };

  struct NotTranslator {
    template<typename T, typename R>
    struct Operation {
      R operator()(const T& arg) const {
        return !arg;
      }
    };

    template<>
    struct Operation<Side, Side> {
      Side operator()(const Side& arg) const {
        return GetOpposite(arg);
      }
    };

    template<typename T0, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& value,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, R>(),
        std::static_pointer_cast<Reactor<T0>>(value));
    }

    using SupportedTypes = NotNodeSignatures::type;
  };

  struct QueryTranslator {
    template<typename T>
    struct Operation {
      int m_index;

      Operation(int index)
          : m_index(index) {}

      T operator ()(const Record& record) const {
        return boost::get<T>(record.GetFields()[m_index]);
      }
    };

    template<typename T>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<Reactor<Record>>& record, int index) {
      return MakeFunctionReactor(Operation<T>(index), record);
    }

    using SupportedTypes = ValueTypes;
  };

  struct RoundTranslator {
    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& value, const T1& places) const {
        return Beam::Round(value, static_cast<int>(places));
      }
    };

    template<>
    struct Operation<Money, Quantity, Money> {
      Money operator()(const Money& value, const Quantity& places) const {
        return Nexus::Round(value, places);
      }
    };

    template<typename T0, typename T1, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& left,
        const std::shared_ptr<BaseReactor>& right,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, T1, R>(),
        std::static_pointer_cast<Reactor<T0>>(left),
        std::static_pointer_cast<Reactor<T1>>(right));
    }

    using SupportedTypes = RoundingNodeSignatures::type;
  };

  struct StaticTranslator {
    template<typename T>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& source) {
      return MakeStaticReactor(std::static_pointer_cast<Reactor<T>>(source));
    }

    using SupportedTypes = ValueTypes;
  };

  struct SubtractionTranslator {
    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left - right;
      }
    };

    template<typename T0, typename T1, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& left,
        const std::shared_ptr<BaseReactor>& right,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, T1, R>(),
        std::static_pointer_cast<Reactor<T0>>(left),
        std::static_pointer_cast<Reactor<T1>>(right));
    }

    using SupportedTypes = SubtractionNodeSignatures::type;
  };

  struct ThrowTranslator {
    template<typename T>
    static std::shared_ptr<BaseReactor> Template(
        const ReactorException& exception) {
      return MakeThrowReactor<T>(exception);
    }

    using SupportedTypes = ValueTypes;
  };

  struct UnequalTranslator {
    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left != right;
      }
    };

    template<typename T0, typename T1, typename R>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& left,
        const std::shared_ptr<BaseReactor>& right,
        CanvasNodeTranslationContext& context) {
      return MakeFunctionReactor(Operation<T0, T1, R>(),
        std::static_pointer_cast<Reactor<T0>>(left),
        std::static_pointer_cast<Reactor<T1>>(right));
    }

    using SupportedTypes = EqualitySignatures::type;
  };

  struct VariableTranslator {
    template<typename T>
    static std::shared_ptr<BaseReactor> Template(
        CanvasNodeTranslationContext& context) {
      return MakeBasicReactor<T>(Ref(context.GetReactorMonitor().GetTrigger()));
    }

    using SupportedTypes = ValueTypes;
  };

  struct MinOperation {
    Quantity operator()(const Quantity& left, const Quantity& right) const {
      return min(left, right);
    }
  };

  struct PropertyBuilder {
    template<typename T>
    static ReactorProperty Template(const string& name,
        std::shared_ptr<BaseReactor> reactor) {
      return MakeReactorProperty(name, std::static_pointer_cast<Reactor<T>>(
        reactor));
    }

    using SupportedTypes = ValueTypes;
  };

  struct DefaultCurrency {
    CurrencyId operator ()(const Security& security,
        const MarketDatabase& marketDatabase) const {
      return marketDatabase.FromCode(security.GetMarket()).m_currency;
    }
  };
}

Translation Spire::Translate(CanvasNodeTranslationContext& context,
    const CanvasNode& node) {
  CanvasNodeTranslationVisitor visitor(Ref(context), Ref(node));
  return visitor.Translate();
}

CanvasNodeTranslationVisitor::CanvasNodeTranslationVisitor(
    RefType<CanvasNodeTranslationContext> context,
    RefType<const CanvasNode> node)
    : m_context(context.Get()),
      m_node(node.Get()) {}

Translation CanvasNodeTranslationVisitor::Translate() {
  return InternalTranslation(*m_node);
}

void CanvasNodeTranslationVisitor::Visit(const AbsNode& node) {
  m_translation = TranslateFunction<AbsTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const AdditionNode& node) {
  m_translation = TranslateFunction<AdditionTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const AggregateNode& node) {
  vector<TaskFactory> factories;
  auto orderExecutionPublisher = MakeAggregateOrderExecutionPublisher();
  for(const auto& child : node.GetChildren()) {
    if(dynamic_cast<const NoneNode*>(&child) == nullptr) {
      auto translation = get<TaskTranslation>(InternalTranslation(child));
      factories.push_back(translation.m_factory);
      orderExecutionPublisher->Add(*translation.m_publisher);
    }
  }
  TaskTranslation taskTranslation;
  taskTranslation.m_publisher = orderExecutionPublisher;
  taskTranslation.m_factory = OrderExecutionPublisherTaskFactory(
    AggregateTaskFactory(factories), orderExecutionPublisher);
  m_translation = taskTranslation;
}

void CanvasNodeTranslationVisitor::Visit(const AlarmNode& node) {
  auto userProfile = &m_context->GetUserProfile();
  auto timerFactory = [=] (time_duration interval) {
    return make_unique<LiveTimer>(interval,
      Ref(userProfile->GetTimerThreadPool()));
  };
  auto expiry = boost::get<std::shared_ptr<BaseReactor>>(InternalTranslation(
    node.GetChildren().front()));
  auto reactor = MakeAlarmReactor(timerFactory,
    &m_context->GetUserProfile().GetServiceClients().GetTimeClient(),
    std::static_pointer_cast<Reactor<ptime>>(expiry),
    Ref(m_context->GetReactorMonitor().GetTrigger()));
  m_context->GetReactorMonitor().Add(reactor);
  m_translation = reactor;
}

void CanvasNodeTranslationVisitor::Visit(const BlotterTaskMonitorNode& node) {
  m_translation = InternalTranslation(node.GetChildren().front());
}

void CanvasNodeTranslationVisitor::Visit(const BooleanNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const CanvasNode& node) {
  if(node.GetType().GetCompatibility(TaskType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL) {
    TaskTranslation taskTranslation;
    taskTranslation.m_factory = IdleTaskFactory();
    taskTranslation.m_publisher =
      std::make_shared<SequencePublisher<const Order*>>();
    m_translation = taskTranslation;
  } else {
    auto& nativeType = static_cast<const NativeType&>(node.GetType());
    m_translation = Instantiate<ThrowTranslator>(nativeType.GetNativeType())(
      ReactorError("Canvas node not supported."));
  }
}

void CanvasNodeTranslationVisitor::Visit(const CeilNode& node) {
  m_translation = TranslateFunction<CeilTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const ChainNode& node) {
  auto previousReactor = boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren().front()));
  if(node.GetChildren().size() == 1) {
    m_translation = previousReactor;
    return;
  }
  auto& nativeType = static_cast<const NativeType&>(node.GetType());
  for(auto i = std::size_t{1}; i < node.GetChildren().size() - 1; ++i) {
    auto currentReactor = boost::get<std::shared_ptr<BaseReactor>>(
      InternalTranslation(node.GetChildren()[i]));
    auto chainReactor = Instantiate<ChainTranslator>(
      nativeType.GetNativeType())(previousReactor, currentReactor, *m_context);
    previousReactor = chainReactor;
  }
  m_translation = previousReactor;
}

void CanvasNodeTranslationVisitor::Visit(const CurrencyNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const CurrentDateNode& node) {
  auto currentDate = ptime(m_context->GetUserProfile().GetServiceClients().
    GetTimeClient().GetTime().date(), microseconds(0));
  m_translation = MakeConstantReactor(currentDate);
}

void CanvasNodeTranslationVisitor::Visit(const CurrentDateTimeNode& node) {
  auto currentDateTime = m_context->GetUserProfile().GetServiceClients().
    GetTimeClient().GetTime();
  m_translation = MakeConstantReactor(currentDateTime);
}

void CanvasNodeTranslationVisitor::Visit(const CurrentTimeNode& node) {
  auto currentTime = m_context->GetUserProfile().GetServiceClients().
    GetTimeClient().GetTime().time_of_day();
  m_translation = MakeConstantReactor(currentTime);
}

void CanvasNodeTranslationVisitor::Visit(const CustomNode& node) {
  m_translation = InternalTranslation(node.GetChildren().front());
  if(node.GetType().GetCompatibility(TaskType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL) {
    auto baseTranslation = boost::get<TaskTranslation>(m_translation);
    vector<ReactorProperty> properties;
    for(auto i = node.GetChildren().begin() + 1;
        i != node.GetChildren().end(); ++i) {
      auto reactor = get<std::shared_ptr<BaseReactor>>(InternalTranslation(*i));
      auto property = Instantiate<PropertyBuilder>(reactor->GetType())(
        i->GetName(), reactor);
      properties.push_back(property);
    }
    TaskTranslation taskTranslation;
    taskTranslation.m_publisher = baseTranslation.m_publisher;
    taskTranslation.m_factory = ReactorTaskFactory(baseTranslation.m_factory,
      properties, Ref(m_context->GetReactorMonitor()));
    m_translation = taskTranslation;
  }
}

void CanvasNodeTranslationVisitor::Visit(const DateTimeNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const DecimalNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const DefaultCurrencyNode& node) {
  auto source = std::static_pointer_cast<Reactor<Security>>(
    boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren().front())));
  m_translation = MakeFunctionReactor(
    std::bind(DefaultCurrency(), std::placeholders::_1,
    std::ref(m_context->GetUserProfile().GetMarketDatabase())), source);
}

void CanvasNodeTranslationVisitor::Visit(const DestinationNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const DivisionNode& node) {
  m_translation = TranslateFunction<DivisionTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const DurationNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const EqualsNode& node) {
  m_translation = TranslateFunction<EqualsTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(
    const ExecutionReportMonitorNode& node) {
  auto taskTranslation = boost::get<TaskTranslation>(InternalTranslation(
    node.GetChildren().front()));
  auto orderSubmissionReactor = MakePublisherReactor(
    *taskTranslation.m_publisher,
    Ref(m_context->GetReactorMonitor().GetTrigger()));
  auto executionReportReactor = MakeFunctionReactor(
    [reactorMonitor = &m_context->GetReactorMonitor()] (const Order* order) {
      auto executionReportReactor = MakePublisherReactor(order->GetPublisher(),
        Ref(reactorMonitor->GetTrigger()));
      return executionReportReactor;
    }, orderSubmissionReactor);
  auto aggregateReactor = MakeAggregateReactor(executionReportReactor);
  m_translation = MakeFunctionReactor(ExecutionReportToRecordConverter{},
    aggregateReactor);
}

void CanvasNodeTranslationVisitor::Visit(const FilePathNode& node) {
  m_translation = MakeConstantReactor(node.GetPath());
}

void CanvasNodeTranslationVisitor::Visit(const FileReaderNode& node) {
  auto pathTranslation = boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren().front()));
  auto path = std::dynamic_pointer_cast<ConstantReactor<string>>(
    pathTranslation);
  auto& nativeType = static_cast<const NativeType&>(node.GetType());
  m_translation = Instantiate<FileReaderTranslator>(nativeType.GetNativeType())(
    nativeType, Ref(m_context->GetReactorMonitor()),
    Ref(m_context->GetUserProfile()), node.GetErrorPolicy(), path->Eval());
}

void CanvasNodeTranslationVisitor::Visit(const FilterNode& node) {
  auto filter = std::static_pointer_cast<Reactor<bool>>(
    boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren().front())));
  auto source = boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren().back()));
  m_translation = Instantiate<FilterTranslator>(
    static_cast<const NativeType&>(node.GetType()).GetNativeType())(filter,
    source);
}

void CanvasNodeTranslationVisitor::Visit(const FloorNode& node) {
  m_translation = TranslateFunction<FloorTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const FoldNode& node) {
  auto source = boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren().front()));
  auto combiner = boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren().back()));
  auto leftTrigger = boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(*node.FindLeftOperand()));
  auto rightTrigger = boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(*node.FindRightOperand()));
  m_translation = Instantiate<FoldTranslator>(source->GetType(),
    combiner->GetType(), combiner->GetType())(source, combiner, leftTrigger,
    rightTrigger);
}

void CanvasNodeTranslationVisitor::Visit(const FoldOperandNode& node) {
  m_translation = Instantiate<FoldParameterTranslator>(
    static_cast<const NativeType&>(node.GetType()).GetNativeType())();
}

void CanvasNodeTranslationVisitor::Visit(const GreaterNode& node) {
  m_translation = TranslateFunction<GreaterTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const GreaterOrEqualsNode& node) {
  m_translation = TranslateFunction<GreaterOrEqualsTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const IfNode& node) {
  m_translation = TranslateFunction<IfTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const IntegerNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const IsTerminalNode& node) {
  auto state = boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren().front()));
  m_translation = MakeFunctionReactor(Beam::Tasks::IsTerminal,
    std::static_pointer_cast<Reactor<Task::State>>(state));
}

void CanvasNodeTranslationVisitor::Visit(const LesserNode& node) {
  m_translation = TranslateFunction<LesserTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const LesserOrEqualsNode& node) {
  m_translation = TranslateFunction<LesserOrEqualsTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const LuaScriptNode& node) {
  auto luaState = luaL_newstate();
  luaL_openlibs(luaState);
  ExportLuaTypes(*luaState);
  auto& nativeType = static_cast<const NativeType&>(node.GetType());
  if(luaL_dofile(luaState, node.GetPath().string().c_str()) != 0) {
    m_translation = Instantiate<ThrowTranslator>(nativeType.GetNativeType())(
      ReactorException(lua_tostring(luaState, -1)));
    lua_close(luaState);
    return;
  }
  vector<unique_ptr<LuaReactorParameter>> parameters;
  for(const auto& child : node.GetChildren()) {
    auto reactor = boost::get<std::shared_ptr<BaseReactor>>(
      InternalTranslation(child));
    auto parameter = Instantiate<LuaParameterTranslator>(reactor->GetType())(
      reactor, child.GetType());
    parameters.emplace_back(std::move(parameter));
  }
  m_translation = Instantiate<LuaScriptTranslator>(nativeType.GetNativeType())(
    node.GetName(), std::move(parameters), *luaState);
}

void CanvasNodeTranslationVisitor::Visit(const MarketNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const MaxFloorNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const MaxNode& node) {
  m_translation = TranslateFunction<MaxTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const MinNode& node) {
  m_translation = TranslateFunction<MinTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const MoneyNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const MultiplicationNode& node) {
  m_translation = TranslateFunction<MultiplicationTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const NoneNode& node) {
  if(node.GetType().GetCompatibility(TaskType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL) {
    TaskTranslation taskTranslation;
    taskTranslation.m_factory = IdleTaskFactory();
    taskTranslation.m_publisher =
      std::make_shared<SequencePublisher<const Order*>>();
    m_translation = taskTranslation;
  } else {
    m_translation = Instantiate<NoneTranslator>(
      static_cast<const NativeType&>(node.GetType()).GetNativeType())();
  }
}

void CanvasNodeTranslationVisitor::Visit(const NotNode& node) {
  m_translation = TranslateFunction<NotTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const OptionalPriceNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const OrderImbalanceQueryNode& node) {
  auto market = std::static_pointer_cast<Reactor<MarketCode>>(
    boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren()[0])));
  auto range = std::static_pointer_cast<Reactor<Range>>(
    boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren()[1])));
  auto marketDataClient =
    &m_context->GetUserProfile().GetServiceClients().GetMarketDataClient();
  auto reactorMonitor = &m_context->GetReactorMonitor();
  auto orderImbalancePublisher = MakeFunctionReactor(
    [=] (MarketCode market, const Range& range) {
      MarketWideDataQuery query;
      query.SetIndex(market);
      query.SetRange(range);
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto queue = std::make_shared<Queue<SequencedOrderImbalance>>();
      marketDataClient->QueryOrderImbalances(query, queue);
      auto reactor = MakeQueueReactor(queue, Ref(reactorMonitor->GetTrigger()));
      return reactor;
    }, std::move(market), std::move(range));
  auto query = MakeSwitchReactor(orderImbalancePublisher);
  m_translation = MakeFunctionReactor(OrderImbalanceToRecordConverter{}, query);
}

void CanvasNodeTranslationVisitor::Visit(const OrderStatusNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const OrderTypeNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const OrderWrapperTaskNode& node) {
  auto orderExecutionPublisher =
    std::make_shared<SequencePublisher<const Order*>>();
  OrderWrapperTaskFactory<VirtualOrderExecutionClient> orderWrapperTaskFactory(
    Ref(m_context->GetUserProfile().GetServiceClients().
    GetOrderExecutionClient()), node.GetOrder());
  vector<ReactorProperty> properties;
  for(const auto& child : node.GetChildren()) {
    auto reactor = get<std::shared_ptr<BaseReactor>>(
      InternalTranslation(child));
    auto property = Instantiate<PropertyBuilder>(reactor->GetType())(
      child.GetName(), reactor);
    properties.push_back(property);
  }
  TaskTranslation taskTranslation;
  taskTranslation.m_publisher = orderExecutionPublisher;
  taskTranslation.m_factory = ReactorTaskFactory(orderWrapperTaskFactory,
    std::move(properties), Ref(m_context->GetReactorMonitor()));
  orderExecutionPublisher->Push(&node.GetOrder());
  m_translation = taskTranslation;
}

void CanvasNodeTranslationVisitor::Visit(const QueryNode& node) {
  auto& recordNode = node.GetChildren().front();
  auto recordReactor = std::static_pointer_cast<Reactor<Record>>(
    boost::get<std::shared_ptr<BaseReactor>>(InternalTranslation(recordNode)));
  auto& recordType = static_cast<const RecordType&>(recordNode.GetType());
  auto fieldIterator = find_if(recordType.GetFields().begin(),
    recordType.GetFields().end(),
    [&] (const RecordType::Field& field) {
      return field.m_name == node.GetField();
    });
  m_translation = Instantiate<QueryTranslator>(
    static_cast<const NativeType&>(node.GetType()).GetNativeType())(
    recordReactor, distance(recordType.GetFields().begin(), fieldIterator));
}

void CanvasNodeTranslationVisitor::Visit(const RangeNode& node) {
  auto lower = std::static_pointer_cast<Reactor<Quantity>>(
    boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren().front())));
  auto upper = std::static_pointer_cast<Reactor<Quantity>>(
    boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren().back())));
  auto reactor = MakeRangeReactor(lower, upper,
    Ref(m_context->GetReactorMonitor().GetTrigger()));
  m_translation = reactor;
}

void CanvasNodeTranslationVisitor::Visit(const ReferenceNode& node) {
  auto& anchor = *FindAnchor(node);
  auto& referent = *node.FindReferent();
  m_translation = InternalTranslation(referent);
  if(referent.GetType().GetCompatibility(TaskType::GetInstance()) !=
      CanvasType::Compatibility::EQUAL) {
    auto parent = referent.GetParent();
    if(parent.is_initialized() &&
        dynamic_cast<const SpawnNode*>(&*parent) != nullptr) {
      if(&referent == &parent->GetChildren().front()) {
        m_translation = Instantiate<StaticTranslator>(
          static_cast<const NativeType&>(node.GetType()).GetNativeType())(
          boost::get<std::shared_ptr<BaseReactor>>(m_translation));
      }
    }
  }
}

void CanvasNodeTranslationVisitor::Visit(const RoundNode& node) {
  m_translation = TranslateFunction<RoundTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const SecurityNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const SideNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const SingleOrderTaskNode& node) {
  auto orderExecutionPublisher =
    std::make_shared<SequencePublisher<const Order*>>();
  SingleOrderTaskFactory<VirtualOrderExecutionClient> singleOrderTaskFactory(
    Ref(m_context->GetUserProfile().GetServiceClients().
    GetOrderExecutionClient()), orderExecutionPublisher,
    m_context->GetExecutingAccount());
  for(const auto& field : node.GetFields()) {
    if(field.m_type->GetCompatibility(IntegerType::GetInstance()) ==
        CanvasType::Compatibility::EQUAL) {
      singleOrderTaskFactory.AddField<Quantity>(field.m_name, field.m_key);
    } else if(field.m_type->GetCompatibility(
        Spire::DecimalType::GetInstance()) ==
        CanvasType::Compatibility::EQUAL) {
      singleOrderTaskFactory.AddField<double>(field.m_name, field.m_key);
    } else if(field.m_type->GetCompatibility(Spire::MoneyType::GetInstance()) ==
        CanvasType::Compatibility::EQUAL) {
      singleOrderTaskFactory.AddField<Money>(field.m_name, field.m_key);
    } else if(field.m_type->GetCompatibility(Spire::TextType::GetInstance()) ==
        CanvasType::Compatibility::EQUAL) {
      singleOrderTaskFactory.AddField<string>(field.m_name, field.m_key);
    }
  }
  SingleRedisplayableOrderTaskFactory singleRedisplayableOrderTaskFactory(
    singleOrderTaskFactory);
  vector<ReactorProperty> properties;
  std::shared_ptr<Reactor<Quantity>> quantityReactor;
  for(const auto& child : node.GetChildren()) {
    auto reactor = boost::get<std::shared_ptr<BaseReactor>>(
      InternalTranslation(child));
    if(child.GetName() == BaseSingleOrderTaskFactory::QUANTITY) {
      quantityReactor = std::static_pointer_cast<Reactor<Quantity>>(reactor);
    }
    string propertyName;
    if(child.GetName() == SingleOrderTaskNode::VOLUME_PROPERTY) {
      propertyName = SingleRedisplayableOrderTaskFactory::DISPLAY;
      reactor = MakeFunctionReactor(MinOperation(),
        std::static_pointer_cast<Reactor<Quantity>>(reactor),
        std::static_pointer_cast<Reactor<Quantity>>(quantityReactor));
    } else {
      propertyName = child.GetName();
    }
    if(child.GetName() != SingleOrderTaskNode::DISPLAY_PROPERTY) {
      auto property = Instantiate<PropertyBuilder>(reactor->GetType())(
        propertyName, reactor);
      properties.push_back(property);
    }
  }
  TaskTranslation taskTranslation;
  taskTranslation.m_publisher = orderExecutionPublisher;
  taskTranslation.m_factory = OrderExecutionPublisherTaskFactory(
    ReactorTaskFactory(singleRedisplayableOrderTaskFactory,
    std::move(properties), Ref(m_context->GetReactorMonitor())),
    orderExecutionPublisher);
  m_translation = taskTranslation;
}

void CanvasNodeTranslationVisitor::Visit(const SpawnNode& node) {
  auto trigger = boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren().front()));
  CanvasNodeTaskFactory taskFactory(Ref(*m_context),
    Ref(node.GetChildren().back()));
  TaskTranslation taskTranslation;
  taskTranslation.m_factory = SpawnTaskFactory(taskFactory, trigger,
    Ref(m_context->GetReactorMonitor()));
  taskTranslation.m_publisher = taskFactory.GetOrderExecutionPublisher();
  m_translation = taskTranslation;
}

void CanvasNodeTranslationVisitor::Visit(const StaticNode& node) {
  auto reactor = boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren().front()));
  m_translation = Instantiate<StaticTranslator>(
    static_cast<const NativeType&>(node.GetType()).GetNativeType())(reactor);
}

void CanvasNodeTranslationVisitor::Visit(const SubtractionNode& node) {
  m_translation = TranslateFunction<SubtractionTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const TaskStateMonitorNode& node) {
  auto translation = boost::get<TaskTranslation>(
    InternalTranslation(node.GetChildren().front()));
  auto task =
    translation.m_factory.DynamicCast<IndirectTaskFactory>()->GetTask();
  auto publisher = MakePublisherReactor(task->GetPublisher(),
    Ref(m_context->GetReactorMonitor().GetTrigger()));
  m_translation = publisher;
}

void CanvasNodeTranslationVisitor::Visit(const TaskStateNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const TextNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const TimeAndSaleQueryNode& node) {
  auto security = std::static_pointer_cast<Reactor<Security>>(
    boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren()[0])));
  auto range = std::static_pointer_cast<Reactor<Range>>(
    boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren()[1])));
  auto marketDataClient = &m_context->GetUserProfile().GetServiceClients().
    GetMarketDataClient();
  auto reactorMonitor = &m_context->GetReactorMonitor();
  auto timeAndSalePublisher = MakeFunctionReactor(
    [=] (const Security& security, const Range& range) {
      SecurityMarketDataQuery query;
      query.SetIndex(security);
      query.SetRange(range);
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto queue = std::make_shared<Queue<SequencedTimeAndSale>>();
      marketDataClient->QueryTimeAndSales(query, queue);
      return MakeQueueReactor(queue, Ref(reactorMonitor->GetTrigger()));
    }, std::move(security), std::move(range));
  auto query = MakeSwitchReactor(timeAndSalePublisher);
  m_translation = MakeFunctionReactor(TimeAndSaleToRecordConverter{}, query);
}

void CanvasNodeTranslationVisitor::Visit(const TimeInForceNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const TimeNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const TimeRangeNode& node) {
  m_translation = MakeConstantReactor(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const TimeRangeParameterNode& node) {
  m_translation = MakeConstantReactor(node.GetTimeRangeQuery(
    m_context->GetUserProfile().GetServiceClients().GetTimeClient()));
}

void CanvasNodeTranslationVisitor::Visit(const TimerNode& node) {
  auto period = std::static_pointer_cast<Reactor<time_duration>>(
    boost::get<std::shared_ptr<BaseReactor>>(InternalTranslation(
    node.GetChildren().front())));
  auto timerThreadPool = &m_context->GetUserProfile().GetTimerThreadPool();
  auto timerFactory = [=] (time_duration interval) {
    return make_unique<LiveTimer>(interval, Ref(*timerThreadPool));
  };
  auto reactor = MakeTimerReactor<Quantity>(timerFactory, period,
    Ref(m_context->GetReactorMonitor().GetTrigger()));
  m_translation = reactor;
}

void CanvasNodeTranslationVisitor::Visit(const UnequalNode& node) {
  m_translation = TranslateFunction<UnequalTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const UntilNode& node) {
  CanvasNodeTaskFactory taskFactory(Ref(*m_context),
    Ref(node.GetChildren().back()));
  auto condition = std::static_pointer_cast<Reactor<bool>>(
    boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren().front())));
  m_context->GetReactorMonitor().Add(condition);
  TaskTranslation taskTranslation;
  taskTranslation.m_factory = UntilTaskFactory(taskFactory, condition,
    Ref(m_context->GetReactorMonitor()));
  taskTranslation.m_publisher = taskFactory.GetOrderExecutionPublisher();
  m_translation = taskTranslation;
}

void CanvasNodeTranslationVisitor::Visit(const WhenNode& node) {
  CanvasNodeTaskFactory taskFactory(Ref(*m_context),
    Ref(node.GetChildren().back()));
  auto condition = std::static_pointer_cast<Reactor<bool>>(
    boost::get<std::shared_ptr<BaseReactor>>(
    InternalTranslation(node.GetChildren().front())));
  m_context->GetReactorMonitor().Add(condition);
  TaskTranslation taskTranslation;
  taskTranslation.m_factory = WhenTaskFactory(taskFactory, condition,
    Ref(m_context->GetReactorMonitor()));
  taskTranslation.m_publisher = taskFactory.GetOrderExecutionPublisher();
  m_translation = taskTranslation;
}

Translation CanvasNodeTranslationVisitor::InternalTranslation(
    const CanvasNode& node) {
  auto existingTranslation = m_context->FindTranslation(node);
  auto isRootTask = false;
  if(existingTranslation.is_initialized()) {
    m_translation = *existingTranslation;
    m_context->Add(Ref(node), m_translation);
  } else if(dynamic_cast<const ReferenceNode*>(&node) == nullptr &&
      node.GetType().GetCompatibility(TaskType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL) {
    auto publisher = MakeAggregateOrderExecutionPublisher();
    TaskTranslation taskTranslation;
    taskTranslation.m_publisher = publisher;
    taskTranslation.m_factory = IndirectTaskFactory(
      Ref(m_context->GetReactorMonitor()));
    m_context->Add(Ref(node), taskTranslation);
    node.Apply(*this);
    auto subtaskTranslation = get<TaskTranslation>(m_translation);
    taskTranslation.m_factory.StaticCast<IndirectTaskFactory>().SetTaskFactory(
      OrderExecutionPublisherTaskFactory(*subtaskTranslation.m_factory,
      publisher));
    publisher->Add(*subtaskTranslation.m_publisher);
    m_translation = taskTranslation;
  } else {
    node.Apply(*this);
    m_context->Add(Ref(node), m_translation);
  }
  return m_translation;
}

template<typename Translator>
Translation CanvasNodeTranslationVisitor::TranslateFunction(
    const CanvasNode& node) {
  vector<std::shared_ptr<BaseReactor>> parameters;
  for(const auto& child : node.GetChildren()) {
    auto parameter = boost::get<std::shared_ptr<BaseReactor>>(
      InternalTranslation(child));
    parameters.push_back(parameter);
  }
  const auto& result = static_cast<const NativeType&>(
    node.GetType()).GetNativeType();
  const auto PARAMETERS = ParameterCount<Translator>::value;
  return FunctionTranslator<Translator, PARAMETERS>()(parameters, result,
    *m_context);
}
