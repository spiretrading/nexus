#include "Spire/Canvas/Operations/CanvasNodeTranslator.hpp"
#include <any>
#include <unordered_map>
#include <Aspen/Aspen.hpp>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/Parsers/ParserPublisher.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/PublisherReactor.hpp>
#include <Beam/Queues/QueueReactor.hpp>
#include <Beam/TimeService/AlarmReactor.hpp>
#include <Beam/TimeService/CurrentTimeReactor.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/TimeService/TimerReactor.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"
#include "Nexus/OrderExecutionService/OrderCancellationReactor.hpp"
#include "Nexus/OrderExecutionService/OrderReactor.hpp"
#include "Nexus/OrderExecutionService/OrderWrapperReactor.hpp"
#include "Nexus/Parsers/CountryParser.hpp"
#include "Nexus/Parsers/CurrencyParser.hpp"
#include "Nexus/Parsers/MoneyParser.hpp"
#include "Nexus/Parsers/OrderStatusParser.hpp"
#include "Nexus/Parsers/OrderTypeParser.hpp"
#include "Nexus/Parsers/QuantityParser.hpp"
#include "Nexus/Parsers/SecurityParser.hpp"
#include "Nexus/Parsers/SideParser.hpp"
#include "Nexus/Parsers/VenueParser.hpp"
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
#include "Spire/Canvas/MarketDataNodes/BboQuoteQueryNode.hpp"
#include "Spire/Canvas/MarketDataNodes/OrderImbalanceQueryNode.hpp"
#include "Spire/Canvas/MarketDataNodes/TimeAndSaleQueryNode.hpp"
#include "Spire/Canvas/Operations/Translation.hpp"
#include "Spire/Canvas/OrderExecutionNodes/DefaultCurrencyNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/ExecutionReportMonitorNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderPublisherReactor.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderWrapperTaskNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SecurityPortfolioNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/Records/QueryNode.hpp"
#include "Spire/Canvas/Records/RecordNode.hpp"
#include "Spire/Canvas/Records/RecordParser.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/StandardNodes/AbsNode.hpp"
#include "Spire/Canvas/StandardNodes/AdditionNode.hpp"
#include "Spire/Canvas/StandardNodes/AlarmNode.hpp"
#include "Spire/Canvas/StandardNodes/CeilNode.hpp"
#include "Spire/Canvas/StandardNodes/ComparisonSignatures.hpp"
#include "Spire/Canvas/StandardNodes/CountNode.hpp"
#include "Spire/Canvas/StandardNodes/CurrentDateNode.hpp"
#include "Spire/Canvas/StandardNodes/CurrentDateTimeNode.hpp"
#include "Spire/Canvas/StandardNodes/CurrentTimeNode.hpp"
#include "Spire/Canvas/StandardNodes/DistinctNode.hpp"
#include "Spire/Canvas/StandardNodes/DivisionNode.hpp"
#include "Spire/Canvas/StandardNodes/EqualsNode.hpp"
#include "Spire/Canvas/StandardNodes/FilterNode.hpp"
#include "Spire/Canvas/StandardNodes/FirstNode.hpp"
#include "Spire/Canvas/StandardNodes/FloorNode.hpp"
#include "Spire/Canvas/StandardNodes/FoldNode.hpp"
#include "Spire/Canvas/StandardNodes/FoldOperandNode.hpp"
#include "Spire/Canvas/StandardNodes/GreaterNode.hpp"
#include "Spire/Canvas/StandardNodes/GreaterOrEqualsNode.hpp"
#include "Spire/Canvas/StandardNodes/IfNode.hpp"
#include "Spire/Canvas/StandardNodes/LastNode.hpp"
#include "Spire/Canvas/StandardNodes/LesserNode.hpp"
#include "Spire/Canvas/StandardNodes/LesserOrEqualsNode.hpp"
#include "Spire/Canvas/StandardNodes/MathSignatures.hpp"
#include "Spire/Canvas/StandardNodes/MaxNode.hpp"
#include "Spire/Canvas/StandardNodes/MinNode.hpp"
#include "Spire/Canvas/StandardNodes/MultiplicationNode.hpp"
#include "Spire/Canvas/StandardNodes/NotNode.hpp"
#include "Spire/Canvas/StandardNodes/PreviousNode.hpp"
#include "Spire/Canvas/StandardNodes/RangeNode.hpp"
#include "Spire/Canvas/StandardNodes/RoundNode.hpp"
#include "Spire/Canvas/StandardNodes/SubtractionNode.hpp"
#include "Spire/Canvas/StandardNodes/TimeRangeParameterNode.hpp"
#include "Spire/Canvas/StandardNodes/TimerNode.hpp"
#include "Spire/Canvas/StandardNodes/UnequalNode.hpp"
#include "Spire/Canvas/SystemNodes/BlotterTaskMonitorNode.hpp"
#include "Spire/Canvas/SystemNodes/InteractionsNode.hpp"
#include "Spire/Canvas/Types/ExecutionReportRecordType.hpp"
#include "Spire/Canvas/Types/OrderImbalanceRecordType.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/Canvas/Types/QuoteRecordType.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/TimeAndSaleRecordType.hpp"
#include "Spire/Canvas/ValueNodes/BooleanNode.hpp"
#include "Spire/Canvas/ValueNodes/CurrencyNode.hpp"
#include "Spire/Canvas/ValueNodes/DateTimeNode.hpp"
#include "Spire/Canvas/ValueNodes/DecimalNode.hpp"
#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/ValueNodes/DurationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderStatusNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderTypeNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TextNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeRangeNode.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"
#include "Spire/Canvas/ValueNodes/VenueNode.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::mp11;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  class CanvasNodeTranslationVisitor final : private CanvasNodeVisitor {
    public:
      CanvasNodeTranslationVisitor(
        Ref<CanvasNodeTranslationContext> context, Ref<const CanvasNode> node);

      Translation Translate();
      void Visit(const AbsNode& node) override;
      void Visit(const AdditionNode& node) override;
      void Visit(const AggregateNode& node) override;
      void Visit(const AlarmNode& node) override;
      void Visit(const BboQuoteQueryNode& node) override;
      void Visit(const BlotterTaskMonitorNode& node) override;
      void Visit(const BooleanNode& node) override;
      void Visit(const CanvasNode& node) override;
      void Visit(const CeilNode& node) override;
      void Visit(const ChainNode& node) override;
      void Visit(const CountNode& node) override;
      void Visit(const CurrencyNode& node) override;
      void Visit(const CurrentDateNode& node) override;
      void Visit(const CurrentDateTimeNode& node) override;
      void Visit(const CurrentTimeNode& node) override;
      void Visit(const CustomNode& node) override;
      void Visit(const DateTimeNode& node) override;
      void Visit(const DecimalNode& node) override;
      void Visit(const DefaultCurrencyNode& node) override;
      void Visit(const DestinationNode& node) override;
      void Visit(const DistinctNode& node) override;
      void Visit(const DivisionNode& node) override;
      void Visit(const DurationNode& node) override;
      void Visit(const EqualsNode& node) override;
      void Visit(const ExecutionReportMonitorNode& node) override;
      void Visit(const FilePathNode& node) override;
      void Visit(const FileReaderNode& node) override;
      void Visit(const FilterNode& node) override;
      void Visit(const FirstNode& node) override;
      void Visit(const FloorNode& node) override;
      void Visit(const FoldNode& node) override;
      void Visit(const FoldOperandNode& node) override;
      void Visit(const GreaterNode& node) override;
      void Visit(const GreaterOrEqualsNode& node) override;
      void Visit(const IfNode& node) override;
      void Visit(const IntegerNode& node) override;
      void Visit(const LastNode& node) override;
      void Visit(const LesserNode& node) override;
      void Visit(const LesserOrEqualsNode& node) override;
      void Visit(const LuaScriptNode& node) override;
      void Visit(const MaxFloorNode& node) override;
      void Visit(const MaxNode& node) override;
      void Visit(const MinNode& node) override;
      void Visit(const MoneyNode& node) override;
      void Visit(const MultiplicationNode& node) override;
      void Visit(const NoneNode& node) override;
      void Visit(const NotNode& node) override;
      void Visit(const OptionalPriceNode& node) override;
      void Visit(const OrderImbalanceQueryNode& node) override;
      void Visit(const OrderStatusNode& node) override;
      void Visit(const OrderTypeNode& node) override;
      void Visit(const OrderWrapperTaskNode& node) override;
      void Visit(const PreviousNode& node) override;
      void Visit(const QueryNode& node) override;
      void Visit(const RangeNode& node) override;
      void Visit(const ReferenceNode& node) override;
      void Visit(const RoundNode& node) override;
      void Visit(const SecurityNode& node) override;
      void Visit(const SideNode& node) override;
      void Visit(const SingleOrderTaskNode& node) override;
      void Visit(const SpawnNode& node) override;
      void Visit(const SubtractionNode& node) override;
      void Visit(const TextNode& node) override;
      void Visit(const TimeAndSaleQueryNode& node) override;
      void Visit(const TimeInForceNode& node) override;
      void Visit(const TimeNode& node) override;
      void Visit(const TimeRangeNode& node) override;
      void Visit(const TimeRangeParameterNode& node) override;
      void Visit(const TimerNode& node) override;
      void Visit(const UnequalNode& node) override;
      void Visit(const UntilNode& node) override;
      void Visit(const VenueNode& node) override;
      void Visit(const WhenNode& node) override;

    private:
      CanvasNodeTranslationContext* m_context;
      const CanvasNode* m_node;
      std::optional<Translation> m_translation;
      std::unordered_map<const CanvasNode*, std::any> m_proxies;

      Translation InternalTranslation(const CanvasNode& node);
      template<typename Translator>
      Translation TranslateFunction(const CanvasNode& node);
  };

  template<typename Translator>
  struct ParameterCount {
    static const auto value =
      mp_size<mp_front<typename Translator::type>>::value - 1;
  };

  template<typename Translator, std::size_t N>
  struct FunctionTranslator {};

  template<typename Translator>
  struct FunctionTranslator<Translator, 1> {
    Translation operator ()(
        const std::vector<Translation>& arguments, const std::type_info& result,
        CanvasNodeTranslationContext& context) const {
      return instantiate<Translator>(arguments[0].GetTypeInfo(), result)(
        arguments[0], context);
    }
  };

  template<typename Translator>
  struct FunctionTranslator<Translator, 2> {
    Translation operator ()(
        const std::vector<Translation>& arguments, const std::type_info& result,
        CanvasNodeTranslationContext& context) const {
      return instantiate<Translator>(
        arguments[0].GetTypeInfo(), arguments[1].GetTypeInfo(), result)(
          arguments[0], arguments[1], context);
    }
  };

  template<typename Translator>
  struct FunctionTranslator<Translator, 3> {
    Translation operator ()(
        const std::vector<Translation>& arguments, const std::type_info& result,
        CanvasNodeTranslationContext& context) const {
      return instantiate<Translator>(arguments[0].GetTypeInfo(),
        arguments[1].GetTypeInfo(), arguments[2].GetTypeInfo(), result)(
        arguments[0], arguments[1], arguments[2], context);
    }
  };

  struct AbsTranslator {
    using type = AbsNodeSignatures::type;

    template<typename T, typename R>
    struct Operation {
      R operator()(const T& arg) const {
        return std::abs(arg);
      }
    };

    template<>
    struct Operation<Nexus::Quantity, Nexus::Quantity> {
      Nexus::Quantity operator ()(const Nexus::Quantity& arg) const {
        return Nexus::abs(arg);
      }
    };

    template<>
    struct Operation<Nexus::Money, Nexus::Money> {
      Nexus::Money operator ()(const Nexus::Money& arg) const {
        return Nexus::abs(arg);
      }
    };

    template<typename T, typename R>
    Translation operator ()(const Translation& value,
        CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T, R>(), value.Extract<Aspen::Box<T>>());
    }
  };

  struct AggregateTranslator {
    using type = NativeTypes;

    template<typename T>
    Translation operator ()(const std::vector<Translation>& children) const {
      auto queue = Aspen::Queue<Aspen::SharedBox<T>>();
      for(auto& child : children) {
        queue.push(Aspen::Shared(child.Extract<Aspen::Box<T>>()));
      }
      queue.set_complete();
      return Aspen::concur(std::move(queue));
    }
  };

  struct AdditionTranslator {
    using type = AdditionNodeSignatures::type;

    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left + right;
      }
    };

    template<>
    struct Operation<double, Quantity, double> {
      double operator()(const double& left, const Quantity& right) const {
        return static_cast<double>(left + right);
      }
    };

    template<>
    struct Operation<Quantity, double, double> {
      double operator()(const Quantity& left, const double& right) const {
        return static_cast<double>(left + right);
      }
    };

    template<typename T0, typename T1, typename R>
    Translation operator ()(const Translation& left,
        const Translation& right, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, R>(), left.Extract<Aspen::Box<T0>>(),
        right.Extract<Aspen::Box<T1>>());
    }
  };

  struct CeilToTranslator {
    using type = RoundingNodeSignatures::type;

    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator ()(const T0& value, const T1& multiple) const {
        return ceil_to(value, multiple);
      }
    };

    template<>
    struct Operation<double, double, double> {
      double operator ()(const double& value, const double& multiple) const {
        if(multiple == 0) {
          return 0;
        }
        return std::ceil(value / multiple) * multiple;
      }
    };

    template<typename T0, typename T1, typename R>
    Translation operator ()(const Translation& left,
        const Translation& right, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, R>(), left.Extract<Aspen::Box<T0>>(),
        right.Extract<Aspen::Box<T1>>());
    }
  };

  struct ChainTranslator {
    using type = NativeTypes;

    template<typename T>
    Translation operator ()(
        const std::vector<Translation>& translations) const {
      if(translations.empty()) {
        return Aspen::box(Aspen::none<T>());
      } else if(translations.size() == 1) {
        return translations.front().Extract<Aspen::Box<T>>();
      } else {
        auto reactor = translations.back().Extract<Aspen::Box<T>>();
        for(auto i = std::size_t(translations.size() - 1); i-- > 0;) {
          reactor = Aspen::box(Aspen::chain(
            translations[i].Extract<Aspen::Box<T>>(), std::move(reactor)));
        }
        return Translation(std::move(reactor));
      }
    }
  };

  struct DistinctTranslator {
    using type = NativeTypes;

    template<typename T>
    Translation operator ()(const Translation& source) const {
      return Aspen::distinct(source.Extract<Aspen::Box<T>>());
    }
  };

  struct DivisionTranslator {
    using type = DivisionNodeSignatures::type;

    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator ()(const T0& left, const T1& right) const {
        if(right == 0) {
          BOOST_THROW_EXCEPTION(std::runtime_error("Division by 0."));
        }
        return left / right;
      }
    };

    template<>
    struct Operation<Nexus::Money, Nexus::Money, double> {
      double operator ()(
          const Nexus::Money& left, const Nexus::Money& right) const {
        if(right == Nexus::Money::ZERO) {
          BOOST_THROW_EXCEPTION(std::runtime_error("Division by 0."));
        }
        return left / right;
      }
    };

    template<>
    struct Operation<double, Quantity, double> {
      double operator()(const double& left, const Quantity& right) const {
        return static_cast<double>(left / right);
      }
    };

    template<>
    struct Operation<Quantity, double, double> {
      double operator()(const Quantity& left, const double& right) const {
        return static_cast<double>(left / right);
      }
    };

    template<>
    struct Operation<time_duration, Quantity, time_duration> {
      time_duration operator()(
          const time_duration& left, const Quantity& right) const {
        return left / static_cast<int>(right);
      }
    };

    template<>
    struct Operation<time_duration, time_duration, double> {
      double operator ()(
          const time_duration& left, const time_duration& right) const {
        if(right.ticks() == 0) {
          BOOST_THROW_EXCEPTION(std::runtime_error("Division by 0."));
        }
        return static_cast<double>(left.ticks()) / right.ticks();
      }
    };

    template<typename T0, typename T1, typename R>
    Translation operator ()(const Translation& left,
        const Translation& right, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, R>(), left.Extract<Aspen::Box<T0>>(),
        right.Extract<Aspen::Box<T1>>());
    }
  };

  struct EqualsTranslator {
    using type = EqualitySignatures::type;

    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left == right;
      }
    };

    template<typename T0, typename T1, typename R>
    Translation operator ()(const Translation& left,
        const Translation& right, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, R>(), left.Extract<Aspen::Box<T0>>(),
        right.Extract<Aspen::Box<T1>>());
    }
  };

  struct FileReaderTranslator {
    using type = ValueTypes;

    template<typename Parser>
    static auto MakeParser(const Parser& parser) {
      return tokenize(parser, ('\n' | eps_p));
    }

    template<typename T>
    Translation operator ()(const NativeType& nativeType,
        Ref<UserProfile> userProfile, ParserErrorPolicy errorPolicy,
        const std::string& path) const {

      // TODO
      if constexpr(std::is_same_v<T, Beam::Sequence> ||
          std::is_same_v<T, Beam::Range> ||
          std::is_same_v<T, TimeInForce>) {
        return Aspen::none<T>();
      } else {
        auto parser = MakeParser(default_parser<T>);
        using Parser = decltype(parser);
        auto publisher = std::make_shared<ParserPublisher<BasicIStreamReader<
          std::ifstream>, Parser>>(path, parser, errorPolicy);
        return publisher_reactor(std::move(publisher));
      }
    }

    template<>
    Translation operator ()<CurrencyId>(const NativeType& nativeType,
        Ref<UserProfile> userProfile, ParserErrorPolicy errorPolicy,
        const std::string& path) const {
      auto parser =
        MakeParser(currency_parser(userProfile->GetCurrencyDatabase()));
      using Parser = decltype(parser);
      auto publisher = std::make_shared<ParserPublisher<BasicIStreamReader<
        std::ifstream>, Parser>>(path, parser, errorPolicy);
      return publisher_reactor(std::move(publisher));
    }

    template<>
    Translation operator ()<Record>(const NativeType& nativeType,
        Ref<UserProfile> userProfile, ParserErrorPolicy errorPolicy,
        const std::string& path) const {
      auto parser = MakeParser(RecordParser(
        static_cast<const RecordType&>(nativeType), Ref(userProfile)));
      using Parser = decltype(parser);
      auto publisher = std::make_shared<ParserPublisher<BasicIStreamReader<
        std::ifstream>, Parser>>(path, parser, errorPolicy);
      return publisher_reactor(std::move(publisher));
    }

    template<>
    Translation operator ()<Security>(const NativeType& nativeType,
        Ref<UserProfile> userProfile, ParserErrorPolicy errorPolicy,
        const std::string& path) const {
      auto parser =
        MakeParser(SecurityParser(userProfile->GetVenueDatabase()));
      using Parser = decltype(parser);
      auto publisher = std::make_shared<ParserPublisher<BasicIStreamReader<
        std::ifstream>, Parser>>(path, parser, errorPolicy);
      return publisher_reactor(std::move(publisher));
    }

    template<>
    Translation operator ()<Venue>(const NativeType& nativeType,
        Ref<UserProfile> userProfile, ParserErrorPolicy errorPolicy,
        const std::string& path) const {
      auto parser = MakeParser(venue_parser(userProfile->GetVenueDatabase()));
      using Parser = decltype(parser);
      auto publisher = std::make_shared<ParserPublisher<BasicIStreamReader<
        std::ifstream>, Parser>>(path, parser, errorPolicy);
      return publisher_reactor(std::move(publisher));
    }
  };

  struct FilterTranslator {
    using type = ValueTypes;

    template<typename T>
    Translation operator ()(
        const Translation& filter, const Translation& source) const {
      return Aspen::discard(
        filter.Extract<Aspen::Box<bool>>(), source.Extract<Aspen::Box<T>>());
    }
  };

  struct FirstTranslator {
    using type = NativeTypes;

    template<typename T>
    Translation operator ()(const Translation& source) const {
      return Aspen::first(source.Extract<Aspen::Box<T>>());
    }
  };

  struct FloorToTranslator {
    using type = RoundingNodeSignatures::type;

    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator ()(const T0& value, const T1& multiple) const {
        return floor_to(value, multiple);
      }
    };

    template<>
    struct Operation<double, double, double> {
      double operator ()(const double& value, const double& multiple) const {
        if(multiple == 0) {
          return 0;
        }
        return std::floor(value / multiple) * multiple;
      }
    };

    template<typename T0, typename T1, typename R>
    Translation operator ()(const Translation& left,
        const Translation& right, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, R>(), left.Extract<Aspen::Box<T0>>(),
        right.Extract<Aspen::Box<T1>>());
    }
  };

  struct FoldParameterTranslator {
    using type = ValueTypes;

    template<typename T>
    Translation operator ()() const {
      return Aspen::make_fold_argument<T>();
    }
  };

  struct FoldTranslator {
    using type = FoldSignatures::type;

    template<typename CombinerType, typename SourceType, typename Unused>
    Translation operator ()(const Translation& combiner,
        const Translation& leftTrigger, const Translation& rightTrigger,
        const Translation& source) const {
      return Aspen::fold(combiner.Extract<Aspen::Box<CombinerType>>(),
        leftTrigger.Extract<Aspen::Shared<Aspen::FoldArgument<CombinerType>>>(),
        rightTrigger.Extract<Aspen::Shared<Aspen::FoldArgument<SourceType>>>(),
        source.Extract<Aspen::Box<SourceType>>());
    }
  };

  struct GreaterTranslator {
    using type = ComparisonSignatures::type;

    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left > right;
      }
    };

    template<typename T0, typename T1, typename R>
    Translation operator ()(const Translation& left,
        const Translation& right, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, R>(), left.Extract<Aspen::Box<T0>>(),
        right.Extract<Aspen::Box<T1>>());
    }
  };

  struct GreaterOrEqualsTranslator {
    using type = ComparisonSignatures::type;

    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left >= right;
      }
    };

    template<typename T0, typename T1, typename R>
    Translation operator ()(const Translation& left,
        const Translation& right, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, R>(), left.Extract<Aspen::Box<T0>>(),
        right.Extract<Aspen::Box<T1>>());
    }
  };

  struct IfTranslator {
    using type = IfNodeSignatures::type;

    template<typename T0, typename T1, typename T2, typename R>
    struct Operation {
      R operator()(
          const T0& condition, const T1& consequent, const T2& other) const {
        return condition ? consequent : other;
      }
    };

    template<typename T0, typename T1, typename T2, typename R>
    Translation operator ()(const Translation& condition,
        const Translation& consequent, const Translation& other,
        CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, T2, R>(),
        condition.Extract<Aspen::Box<T0>>(),
        consequent.Extract<Aspen::Box<T1>>(),
        other.Extract<Aspen::Box<T2>>());
    }
  };

  struct LastTranslator {
    using type = NativeTypes;

    template<typename T>
    Translation operator ()(const Translation& source) const {
      return Aspen::last(source.Extract<Aspen::Box<T>>());
    }
  };

  struct LesserTranslator {
    using type = ComparisonSignatures::type;

    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left < right;
      }
    };

    template<typename T0, typename T1, typename R>
    Translation operator ()(const Translation& left,
        const Translation& right, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, R>(), left.Extract<Aspen::Box<T0>>(),
        right.Extract<Aspen::Box<T1>>());
    }
  };

  struct LesserOrEqualsTranslator {
    using type = ComparisonSignatures::type;

    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left <= right;
      }
    };

    template<typename T0, typename T1, typename R>
    Translation operator ()(const Translation& left,
        const Translation& right, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, R>(), left.Extract<Aspen::Box<T0>>(),
        right.Extract<Aspen::Box<T1>>());
    }
  };

  struct LuaParameterTranslator {
    using type = ValueTypes;

    template<typename T>
    Aspen::Unique<LuaReactorParameter> operator ()(
        const Translation& reactor, const CanvasType& type) const {
      return Aspen::Unique<LuaReactorParameter>(
        new NativeLuaReactorParameter(reactor.Extract<Aspen::Box<T>>()));
    }

    template<>
    Aspen::Unique<LuaReactorParameter> operator ()<Record>(
        const Translation& reactor, const CanvasType& type) const {
      return Aspen::Unique<LuaReactorParameter>(new RecordLuaReactorParameter(
        reactor.Extract<Aspen::Box<Record>>(),
        static_cast<const RecordType&>(type)));
    }
  };

  struct LuaScriptTranslator {
    using type = ValueTypes;

    template<typename T>
    Translation operator ()(std::string name,
        std::vector<Aspen::Unique<LuaReactorParameter>> parameters,
        lua_State& luaState) const {
      return LuaReactor<T>(std::move(name), std::move(parameters), luaState);
    }
  };

  struct MaxTranslator {
    using type = ExtremaNodeSignatures::type;

    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return std::max(left, right);
      }
    };

    template<typename T0, typename T1, typename R>
    Translation operator ()(const Translation& left,
        const Translation& right, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, R>(), left.Extract<Aspen::Box<T0>>(),
        right.Extract<Aspen::Box<T1>>());
    }
  };

  struct MinTranslator {
    using type = ExtremaNodeSignatures::type;

    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return std::min(left, right);
      }
    };

    template<typename T0, typename T1, typename R>
    Translation operator ()(const Translation& left,
        const Translation& right, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, R>(), left.Extract<Aspen::Box<T0>>(),
        right.Extract<Aspen::Box<T1>>());
    }
  };

  struct MultiplicationTranslator {
    using type = MultiplicationNodeSignatures::type;

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
    struct Operation<Quantity, Money, Money> {
      Money operator()(const Quantity& left, const Money& right) const {
        return left * right;
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
      time_duration operator()(
          const Quantity& left, const time_duration& right) const {
        return right * static_cast<int>(left);
      }
    };

    template<>
    struct Operation<time_duration, Quantity, time_duration> {
      time_duration operator()(
          const time_duration& left, const Quantity& right) const {
        return left * static_cast<int>(right);
      }
    };

    template<>
    struct Operation<double, Quantity, double> {
      double operator()(const double& left, const Quantity& right) const {
        return static_cast<double>(left * right);
      }
    };

    template<>
    struct Operation<Quantity, double, double> {
      double operator()(const Quantity& left, const double& right) const {
        return static_cast<double>(right * left);
      }
    };

    template<typename T0, typename T1, typename R>
    Translation operator ()(const Translation& left,
        const Translation& right, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, R>(), left.Extract<Aspen::Box<T0>>(),
        right.Extract<Aspen::Box<T1>>());
    }
  };

  struct NoneTranslator {
    using type = NativeTypes;

    template<typename T>
    Translation operator ()() const {
      return Aspen::none<T>();
    }
  };

  struct NotTranslator {
    using type = NotNodeSignatures::type;

    template<typename T, typename R>
    struct Operation {
      R operator()(const T& arg) const {
        return !arg;
      }
    };

    template<>
    struct Operation<Side, Side> {
      Side operator()(const Side& arg) const {
        return get_opposite(arg);
      }
    };

    template<typename T0, typename R>
    Translation operator ()(
        const Translation& value, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, R>(), value.Extract<Aspen::Box<T0>>());
    }
  };

  struct PreviousTranslator {
    using type = NativeTypes;

    template<typename T>
    Translation operator ()(const Translation& source) const {
      return Aspen::previous(source.Extract<Aspen::Box<T>>());
    }
  };

  struct ProxyBuilder {
    using type = NativeTypes;

    template<typename T>
    std::any operator ()() const {
      return Aspen::Shared(Aspen::Proxy<Aspen::Box<T>>());
    }
  };

  struct ProxyFinalizer {
    using type = NativeTypes;

    template<typename T>
    void operator ()(
        const std::any& proxy, const Translation& translation) const {
      auto reactor =
        std::any_cast<Aspen::Shared<Aspen::Proxy<Aspen::Box<T>>>>(proxy);
      reactor->set_reactor(translation.Extract<Aspen::Box<T>>());
    }
  };

  struct ProxyTranslator {
    using type = NativeTypes;

    template<typename T>
    Translation operator ()(const std::any& proxy) const {
      return std::any_cast<Aspen::Shared<Aspen::Proxy<Aspen::Box<T>>>>(proxy);
    }
  };

  struct QueryTranslator {
    using type = ValueTypes;

    template<typename T>
    struct Operation {
      int m_index;

      Operation(int index)
        : m_index(index) {}

      T operator ()(const Record& record) const {
        return get<T>(record.GetFields()[m_index]);
      }
    };

    template<typename T>
    Translation operator ()(Aspen::Box<Record> record, int index) const {
      return Aspen::lift(Operation<T>(index), std::move(record));
    }
  };

  struct ReferenceTranslator {
    using type = NativeTypes;

    template<typename T>
    Translation operator ()(const Translation& source) const {
      return source.Extract<Aspen::Box<T>>();
    }
  };

  struct RoundToTranslator {
    using type = RoundingNodeSignatures::type;

    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& value, const T1& multiple) const {
        return round_to(value, multiple);
      }
    };

    template<>
    struct Operation<double, double, double> {
      double operator ()(const double& value, const double& multiple) const {
        if(multiple == 0) {
          return 0;
        }
        return std::round(value / multiple) * multiple;
      }
    };

    template<typename T0, typename T1, typename R>
    Translation operator ()(const Translation& left,
        const Translation& right, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, R>(), left.Extract<Aspen::Box<T0>>(),
        right.Extract<Aspen::Box<T1>>());
    }
  };

  struct SpawnTranslator {
    using type = NativeTypes;

    template<typename T>
    struct SpawnReactor {
      using Type = Aspen::Box<T>::Type;
      using Result = Aspen::Box<T>::Result;
      CanvasNodeTranslationContext m_context;
      Aspen::Box<T> m_reactor;

      explicit SpawnReactor(
        CanvasNodeTranslationContext& context, const CanvasNode& series)
        : m_context(Ref(context)),
          m_reactor(Translate(m_context, series).Extract<Aspen::Box<T>>()) {}

      Aspen::State commit(int sequence) noexcept {
        return m_reactor.commit(sequence);
      }

      Result eval() const {
        return m_reactor.eval();
      }
    };

    template<typename T>
    Translation operator ()(CanvasNodeTranslationContext& context,
        Aspen::Box<void> trigger, const CanvasNode& series) const {
      return Aspen::concur(Aspen::lift([&] (const Aspen::Maybe<void>& value) {
        return Aspen::Shared<SpawnReactor<T>>(context, series);
      }, std::move(trigger)));
    }
  };

  struct SubtractionTranslator {
    using type = SubtractionNodeSignatures::type;

    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left - right;
      }
    };

    template<>
    struct Operation<double, Quantity, double> {
      double operator()(const double& left, const Quantity& right) const {
        return static_cast<double>(left - right);
      }
    };

    template<>
    struct Operation<Quantity, double, double> {
      double operator()(const Quantity& left, const double& right) const {
        return static_cast<double>(left - right);
      }
    };

    template<typename T0, typename T1, typename R>
    Translation operator ()(const Translation& left,
        const Translation& right, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, R>(), left.Extract<Aspen::Box<T0>>(),
        right.Extract<Aspen::Box<T1>>());
    }
  };

  struct ThrowTranslator {
    using type = NativeTypes;

    template<typename T>
    Translation operator ()(std::exception_ptr exception) const {
      return Aspen::throws<T>(std::move(exception));
    }
  };

  struct UnequalTranslator {
    using type = EqualitySignatures::type;

    template<typename T0, typename T1, typename R>
    struct Operation {
      R operator()(const T0& left, const T1& right) const {
        return left != right;
      }
    };

    template<typename T0, typename T1, typename R>
    Translation operator ()(const Translation& left,
        const Translation& right, CanvasNodeTranslationContext& context) const {
      return Aspen::lift(Operation<T0, T1, R>(), left.Extract<Aspen::Box<T0>>(),
        right.Extract<Aspen::Box<T1>>());
    }
  };

  struct UntilTranslator {
    using type = NativeTypes;

    template<typename T>
    Translation operator ()(Aspen::Box<bool> condition,
        const Translation& series, CanvasNodeTranslationContext& context) const {
      return Aspen::until(
        std::move(condition), series.Extract<Aspen::Box<T>>());
    }

    template<>
    Translation operator ()<std::shared_ptr<Order>>(
        Aspen::Box<bool> condition, const Translation& series,
        CanvasNodeTranslationContext& context) const {
      return OrderCancellationReactor(
        context.GetUserProfile().GetClients().get_order_execution_client(),
        Aspen::until(std::move(condition),
          series.Extract<Aspen::Box<std::shared_ptr<Order>>>()));
    }
  };

  struct WhenTranslator {
    using type = NativeTypes;

    template<typename T>
    Translation operator ()(Aspen::Box<bool> condition,
        const Translation& series) const {
      return Aspen::when(std::move(condition), series.Extract<Aspen::Box<T>>());
    }
  };
}

Translation Spire::Translate(
    CanvasNodeTranslationContext& context, const CanvasNode& node) {
  auto visitor = CanvasNodeTranslationVisitor(Ref(context), Ref(node));
  return visitor.Translate();
}

CanvasNodeTranslationVisitor::CanvasNodeTranslationVisitor(
  Ref<CanvasNodeTranslationContext> context, Ref<const CanvasNode> node)
  : m_context(context.get()),
    m_node(node.get()) {}

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
  auto children = std::vector<Translation>();
  for(auto& child : node.GetChildren()) {
    if(!dynamic_cast<const NoneNode*>(&child)) {
      children.push_back(InternalTranslation(child));
    }
  }
  auto& nativeType = static_cast<const NativeType&>(node.GetType());
  m_translation =
    instantiate<AggregateTranslator>(nativeType.GetNativeType())(children);
}

void CanvasNodeTranslationVisitor::Visit(const AlarmNode& node) {
  auto timerFactory = [] (time_duration interval) {
    return std::make_unique<LiveTimer>(interval);
  };
  auto expiry = InternalTranslation(
    node.GetChildren().front()).Extract<Aspen::Box<ptime>>();
  m_translation = alarm_reactor(
    &m_context->GetUserProfile().GetClients().get_time_client(),
    timerFactory, std::move(expiry));
}

void CanvasNodeTranslationVisitor::Visit(const BboQuoteQueryNode& node) {
  auto security =
    InternalTranslation(node.GetChildren()[0]).Extract<Aspen::Box<Security>>();
  auto side =
    InternalTranslation(node.GetChildren()[1]).Extract<Aspen::Box<Side>>();
  auto range = InternalTranslation(
    node.GetChildren()[2]).Extract<Aspen::Box<Beam::Range>>();
  auto marketDataClient =
    &m_context->GetUserProfile().GetClients().get_market_data_client();
  m_translation = Aspen::lift(QuoteToRecordConverter{},
    Aspen::lift(
      [] (Side side, const SequencedBboQuote& quote) {
        return pick(side, quote->m_ask, quote->m_bid);
      }, std::move(side), Aspen::override(Aspen::lift(
      [=] (const Security& security, const Beam::Range& range) {
        auto query = SecurityMarketDataQuery();
        query.set_index(security);
        query.set_range(range);
        query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
        auto queue = std::make_shared<Queue<SequencedBboQuote>>();
        marketDataClient->query(query, queue);
        return Aspen::Shared(QueueReactor(queue));
      }, std::move(security), std::move(range)))));
}

void CanvasNodeTranslationVisitor::Visit(const BlotterTaskMonitorNode& node) {
  m_translation = InternalTranslation(node.GetChildren().front());
}

void CanvasNodeTranslationVisitor::Visit(const BooleanNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const CanvasNode& node) {
  m_translation = instantiate<NoneTranslator>(
    static_cast<const NativeType&>(node.GetType()).GetNativeType())();
}

void CanvasNodeTranslationVisitor::Visit(const CeilNode& node) {
  m_translation = TranslateFunction<CeilToTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const ChainNode& node) {
  auto translations = std::vector<Translation>();
  for(auto& child : node.GetChildren()) {
    if(!dynamic_cast<const NoneNode*>(&child)) {
      translations.push_back(InternalTranslation(child));
    }
  }
  auto& nativeType = static_cast<const NativeType&>(node.GetType());
  m_translation =
    instantiate<ChainTranslator>(nativeType.GetNativeType())(translations);
}

void CanvasNodeTranslationVisitor::Visit(const CountNode& node) {
  auto source = InternalTranslation(node.GetChildren().front());
  m_translation = Aspen::static_reactor_cast<Quantity>(
    Aspen::count(source.Extract<Aspen::Box<void>>()));
}

void CanvasNodeTranslationVisitor::Visit(const CurrencyNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const CurrentDateNode& node) {
  m_translation = Aspen::lift(
    [] (ptime time) {
      return ptime(time.date(), seconds(0));
    }, current_time_reactor(
    &m_context->GetUserProfile().GetClients().get_time_client()));
}

void CanvasNodeTranslationVisitor::Visit(const CurrentDateTimeNode& node) {
  m_translation = current_time_reactor(
    &m_context->GetUserProfile().GetClients().get_time_client());
}

void CanvasNodeTranslationVisitor::Visit(const CurrentTimeNode& node) {
  m_translation = Aspen::lift(
    [] (ptime time) {
      return time.time_of_day();
    }, current_time_reactor(
    &m_context->GetUserProfile().GetClients().get_time_client()));
}

void CanvasNodeTranslationVisitor::Visit(const CustomNode& node) {
  m_translation = InternalTranslation(node.GetChildren().front());
}

void CanvasNodeTranslationVisitor::Visit(const DateTimeNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const DecimalNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const DefaultCurrencyNode& node) {
  auto source = InternalTranslation(
    node.GetChildren().front()).Extract<Aspen::Box<Security>>();
  m_translation = Aspen::lift(
    [userProfile = &m_context->GetUserProfile()] (const Security& security) {
      return userProfile->GetVenueDatabase().from(
        security.get_venue()).m_currency;
    }, std::move(source));
}

void CanvasNodeTranslationVisitor::Visit(const DestinationNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const DistinctNode& node) {
  m_translation = instantiate<DistinctTranslator>(
    static_cast<const NativeType&>(node.GetType()).GetNativeType())(
      InternalTranslation(node.GetChildren().front()));
}

void CanvasNodeTranslationVisitor::Visit(const DivisionNode& node) {
  m_translation = TranslateFunction<DivisionTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const DurationNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const EqualsNode& node) {
  m_translation = TranslateFunction<EqualsTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(
    const ExecutionReportMonitorNode& node) {
  auto source = InternalTranslation(node.GetChildren().front());
  m_translation = Aspen::lift(ExecutionReportToRecordConverter(),
    Aspen::concur(Aspen::lift(
    [] (const std::shared_ptr<Order>& order) {
      return Aspen::shared_box(publisher_reactor(order->get_publisher()));
    }, source.Extract<Aspen::Box<std::shared_ptr<Order>>>())));
}

void CanvasNodeTranslationVisitor::Visit(const FilePathNode& node) {
  m_translation = Aspen::constant(node.GetPath());
}

void CanvasNodeTranslationVisitor::Visit(const FileReaderNode& node) {
  auto path = InternalTranslation(
    node.GetChildren().front()).Extract<Aspen::Box<std::string>>();
  auto& nativeType = static_cast<const NativeType&>(node.GetType());
  m_translation = instantiate<FileReaderTranslator>(nativeType.GetNativeType())(
    nativeType, Ref(m_context->GetUserProfile()), node.GetErrorPolicy(),
    path.eval());
}

void CanvasNodeTranslationVisitor::Visit(const FilterNode& node) {
  auto filter = InternalTranslation(node.GetChildren().front());
  auto source = InternalTranslation(node.GetChildren().back());
  m_translation = instantiate<FilterTranslator>(
    static_cast<const NativeType&>(node.GetType()).GetNativeType())(filter,
    source);
}

void CanvasNodeTranslationVisitor::Visit(const FirstNode& node) {
  auto reactor = InternalTranslation(node.GetChildren().front());
  m_translation = instantiate<FirstTranslator>(
    static_cast<const NativeType&>(node.GetType()).GetNativeType())(reactor);
}

void CanvasNodeTranslationVisitor::Visit(const FloorNode& node) {
  m_translation = TranslateFunction<FloorToTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const FoldNode& node) {
  auto combiner = InternalTranslation(node.GetChildren().front());
  auto leftTrigger = InternalTranslation(*node.FindLeftOperand());
  auto rightTrigger = InternalTranslation(*node.FindRightOperand());
  auto source = InternalTranslation(node.GetChildren().back());
  m_translation = instantiate<FoldTranslator>(combiner.GetTypeInfo(),
    source.GetTypeInfo(), source.GetTypeInfo())(combiner, leftTrigger,
    rightTrigger, source);
}

void CanvasNodeTranslationVisitor::Visit(const FoldOperandNode& node) {
  m_translation = instantiate<FoldParameterTranslator>(
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
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const LastNode& node) {
  m_translation = instantiate<LastTranslator>(static_cast<const NativeType&>(
    node.GetType()).GetNativeType())(InternalTranslation(
      node.GetChildren().front()));
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
    m_translation = instantiate<ThrowTranslator>(nativeType.GetNativeType())(
      std::make_exception_ptr(std::runtime_error(lua_tostring(luaState, -1))));
    lua_close(luaState);
    return;
  }
  auto parameters = std::vector<Aspen::Unique<LuaReactorParameter>>();
  for(auto& child : node.GetChildren()) {
    auto& childNativeType = static_cast<const NativeType&>(child.GetType());
    auto parameter = instantiate<LuaParameterTranslator>(
      childNativeType.GetNativeType())(InternalTranslation(child),
      child.GetType());
    parameters.push_back(std::move(parameter));
  }
  m_translation = instantiate<LuaScriptTranslator>(nativeType.GetNativeType())(
    node.GetName(), std::move(parameters), *luaState);
}

void CanvasNodeTranslationVisitor::Visit(const MaxFloorNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const MaxNode& node) {
  m_translation = TranslateFunction<MaxTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const MinNode& node) {
  m_translation = TranslateFunction<MinTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const MoneyNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const MultiplicationNode& node) {
  m_translation = TranslateFunction<MultiplicationTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const NoneNode& node) {
  m_translation = instantiate<NoneTranslator>(
    static_cast<const NativeType&>(node.GetType()).GetNativeType())();
}

void CanvasNodeTranslationVisitor::Visit(const NotNode& node) {
  m_translation = TranslateFunction<NotTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const OptionalPriceNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const OrderImbalanceQueryNode& node) {
  auto market = InternalTranslation(
    node.GetChildren()[0]).Extract<Aspen::Box<Venue>>();
  auto range = InternalTranslation(
    node.GetChildren()[1]).Extract<Aspen::Box<Beam::Range>>();
  auto marketDataClient =
    &m_context->GetUserProfile().GetClients().get_market_data_client();
  m_translation = Aspen::lift(OrderImbalanceToRecordConverter{},
    Aspen::override(Aspen::lift(
    [=] (Venue venue, const Beam::Range& range) {
      auto query = VenueMarketDataQuery();
      query.set_index(venue);
      query.set_range(range);
      query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto queue = std::make_shared<Queue<SequencedOrderImbalance>>();
      marketDataClient->query(query, queue);
      return Aspen::Shared(QueueReactor(queue));
    }, std::move(market), std::move(range))));
}

void CanvasNodeTranslationVisitor::Visit(const OrderStatusNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const OrderTypeNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const OrderWrapperTaskNode& node) {
  m_context->Add(Ref(*node.FindChild(SingleOrderTaskNode::SECURITY_PROPERTY)),
    Aspen::constant(node.GetOrder()->get_info().m_fields.m_security));
  m_context->Add(Ref(*node.FindChild(SingleOrderTaskNode::ORDER_TYPE_PROPERTY)),
    Aspen::constant(node.GetOrder()->get_info().m_fields.m_type));
  m_context->Add(Ref(*node.FindChild(SingleOrderTaskNode::SIDE_PROPERTY)),
    Aspen::constant(node.GetOrder()->get_info().m_fields.m_side));
  m_context->Add(Ref(*node.FindChild(
    SingleOrderTaskNode::DESTINATION_PROPERTY)),
    Aspen::constant(node.GetOrder()->get_info().m_fields.m_destination));
  m_context->Add(Ref(*node.FindChild(SingleOrderTaskNode::PRICE_PROPERTY)),
    Aspen::constant(node.GetOrder()->get_info().m_fields.m_price));
  m_context->Add(Ref(*node.FindChild(SingleOrderTaskNode::QUANTITY_PROPERTY)),
    Aspen::constant(node.GetOrder()->get_info().m_fields.m_quantity));
  m_context->Add(Ref(*node.FindChild(SingleOrderTaskNode::CURRENCY_PROPERTY)),
    Aspen::constant(node.GetOrder()->get_info().m_fields.m_currency));
  m_context->Add(Ref(*node.FindChild(
    SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY)),
    Aspen::constant(node.GetOrder()->get_info().m_fields.m_time_in_force));
  m_translation = OrderPublisherReactor(m_context->GetOrderPublisher(),
    OrderWrapperReactor(node.GetOrder()));
}

void CanvasNodeTranslationVisitor::Visit(const PreviousNode& node) {
  m_translation = instantiate<PreviousTranslator>(
    static_cast<const NativeType&>(node.GetType()).GetNativeType())(
      InternalTranslation(node.GetChildren().front()));
}

void CanvasNodeTranslationVisitor::Visit(const QueryNode& node) {
  auto& recordNode = node.GetChildren().front();
  auto recordReactor =
    InternalTranslation(recordNode).Extract<Aspen::Box<Record>>();
  auto& recordType = static_cast<const RecordType&>(recordNode.GetType());
  auto fieldIterator = find_if(recordType.GetFields().begin(),
    recordType.GetFields().end(),
    [&] (const RecordType::Field& field) {
      return field.m_name == node.GetField();
    });
  m_translation = instantiate<QueryTranslator>(
    static_cast<const NativeType&>(node.GetType()).GetNativeType())(
    std::move(recordReactor),
    std::distance(recordType.GetFields().begin(), fieldIterator));
}

void CanvasNodeTranslationVisitor::Visit(const RangeNode& node) {
  auto lower = InternalTranslation(
    node.GetChildren().front()).Extract<Aspen::Box<Quantity>>();
  auto upper = InternalTranslation(
    node.GetChildren().back()).Extract<Aspen::Box<Quantity>>();
  m_translation = Aspen::range(std::move(lower), std::move(upper));
}

void CanvasNodeTranslationVisitor::Visit(const ReferenceNode& node) {
  auto referent = static_cast<const CanvasNode*>(&node);
  while(auto reference = dynamic_cast<const ReferenceNode*>(referent)) {
    referent = &*reference->FindReferent();
    if(referent == reference) {
      m_translation = instantiate<NoneTranslator>(
        static_cast<const NativeType&>(referent->GetType()).GetNativeType())();
      return;
    }
  }
  auto translation = instantiate<ReferenceTranslator>(
    static_cast<const NativeType&>(referent->GetType()).GetNativeType())(
    InternalTranslation(*referent));
  if(dynamic_cast<const SingleOrderTaskNode*>(referent)) {
    m_translation = translation.ToWeak();
  } else {
    m_translation = std::move(translation);
  }
}

void CanvasNodeTranslationVisitor::Visit(const RoundNode& node) {
  m_translation = TranslateFunction<RoundToTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const SecurityNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const SideNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const SingleOrderTaskNode& node) {
  auto orderExecutionPublisher =
    std::make_shared<SequencePublisher<std::shared_ptr<Order>>>();
  auto additionalFields = std::vector<Aspen::Box<Tag>>();
  for(const auto& field : node.GetFields()) {
    auto value = InternalTranslation(*node.FindChild(field.m_name));
    if(field.m_type->GetCompatibility(IntegerType::GetInstance()) ==
        CanvasType::Compatibility::EQUAL) {
      additionalFields.push_back(Aspen::box(Aspen::unconsecutive(Aspen::lift(
        [key = field.m_key] (Quantity q) {
          return Tag(key, q);
        }, value.Extract<Aspen::Box<Quantity>>()))));
    } else if(field.m_type->GetCompatibility(
        Spire::DecimalType::GetInstance()) ==
        CanvasType::Compatibility::EQUAL) {
      additionalFields.push_back(Aspen::box(Aspen::unconsecutive(Aspen::lift(
        [key = field.m_key] (double q) {
          return Tag(key, q);
        }, value.Extract<Aspen::Box<double>>()))));
    } else if(field.m_type->GetCompatibility(Spire::MoneyType::GetInstance()) ==
        CanvasType::Compatibility::EQUAL) {
      additionalFields.push_back(Aspen::box(Aspen::unconsecutive(Aspen::lift(
        [key = field.m_key] (Money q) {
          return Tag(key, q);
        }, value.Extract<Aspen::Box<Money>>()))));
    } else if(field.m_type->GetCompatibility(Spire::TextType::GetInstance()) ==
        CanvasType::Compatibility::EQUAL) {
      additionalFields.push_back(Aspen::box(Aspen::unconsecutive(Aspen::lift(
        [key = field.m_key] (std::string q) {
          return Tag(key, q);
        }, value.Extract<Aspen::Box<std::string>>()))));
    }
  }
  auto& orderExecutionClient =
    m_context->GetUserProfile().GetClients().get_order_execution_client();
  m_translation = OrderPublisherReactor(m_context->GetOrderPublisher(),
    OrderReactor(Ref(orderExecutionClient),
    Aspen::constant(m_context->GetExecutingAccount()),
    Aspen::unconsecutive(InternalTranslation(*node.FindChild(
      SingleOrderTaskNode::SECURITY_PROPERTY)).Extract<Aspen::Box<Security>>()),
    Aspen::unconsecutive(InternalTranslation(*node.FindChild(
      SingleOrderTaskNode::CURRENCY_PROPERTY)).Extract<
      Aspen::Box<CurrencyId>>()),
    Aspen::unconsecutive(InternalTranslation(*node.FindChild(
      SingleOrderTaskNode::ORDER_TYPE_PROPERTY)).Extract<
      Aspen::Box<OrderType>>()),
    Aspen::unconsecutive(InternalTranslation(*node.FindChild(
      SingleOrderTaskNode::SIDE_PROPERTY)).Extract<Aspen::Box<Side>>()),
    Aspen::unconsecutive(InternalTranslation(*node.FindChild(
      SingleOrderTaskNode::DESTINATION_PROPERTY)).Extract<
      Aspen::Box<std::string>>()),
    Aspen::unconsecutive(InternalTranslation(*node.FindChild(
      SingleOrderTaskNode::QUANTITY_PROPERTY)).Extract<Aspen::Box<Quantity>>()),
    Aspen::unconsecutive(InternalTranslation(*node.FindChild(
      SingleOrderTaskNode::PRICE_PROPERTY)).Extract<Aspen::Box<Money>>()),
    Aspen::unconsecutive(InternalTranslation(*node.FindChild(
      SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY)).Extract<
      Aspen::Box<TimeInForce>>()), std::move(additionalFields)));
}

void CanvasNodeTranslationVisitor::Visit(const SpawnNode& node) {
  auto trigger =
    InternalTranslation(node.GetChildren().front()).Extract<Aspen::Box<void>>();
  auto& seriesType =
    static_cast<const NativeType&>(node.GetChildren().back().GetType());
  m_translation = instantiate<SpawnTranslator>(seriesType.GetNativeType())(
    *m_context, std::move(trigger), node.GetChildren().back());
}

void CanvasNodeTranslationVisitor::Visit(const SubtractionNode& node) {
  m_translation = TranslateFunction<SubtractionTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const TextNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const TimeAndSaleQueryNode& node) {
  auto security =
    InternalTranslation(node.GetChildren()[0]).Extract<Aspen::Box<Security>>();
  auto range = InternalTranslation(
    node.GetChildren()[1]).Extract<Aspen::Box<Beam::Range>>();
  auto marketDataClient =
    &m_context->GetUserProfile().GetClients().get_market_data_client();
  m_translation = Aspen::lift(TimeAndSaleToRecordConverter{}, Aspen::override(
    Aspen::lift(
    [=] (const Security& security, const Beam::Range& range) {
      auto query = SecurityMarketDataQuery();
      query.set_index(security);
      query.set_range(range);
      query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto queue = std::make_shared<Queue<SequencedTimeAndSale>>();
      marketDataClient->query(query, queue);
      return Aspen::Shared(QueueReactor(queue));
    }, std::move(security), std::move(range))));
}

void CanvasNodeTranslationVisitor::Visit(const TimeInForceNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const TimeNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const TimeRangeNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const TimeRangeParameterNode& node) {
  m_translation = Aspen::constant(node.GetTimeRangeQuery(
    m_context->GetUserProfile().GetClients().get_time_client()));
}

void CanvasNodeTranslationVisitor::Visit(const TimerNode& node) {
  auto period = InternalTranslation(node.GetChildren().front());
  auto timerFactory = [=] (time_duration interval) {
    return std::make_unique<LiveTimer>(interval);
  };
  m_translation = timer_reactor<Quantity>(timerFactory,
    period.Extract<Aspen::Box<time_duration>>());
}

void CanvasNodeTranslationVisitor::Visit(const UnequalNode& node) {
  m_translation = TranslateFunction<UnequalTranslator>(node);
}

void CanvasNodeTranslationVisitor::Visit(const UntilNode& node) {
  auto condition =
    InternalTranslation(node.GetChildren().front()).Extract<Aspen::Box<bool>>();
  auto series = InternalTranslation(node.GetChildren().back());
  m_translation = instantiate<UntilTranslator>(series.GetTypeInfo())(
    std::move(condition), series, *m_context);
}

void CanvasNodeTranslationVisitor::Visit(const VenueNode& node) {
  m_translation = Aspen::constant(node.GetValue());
}

void CanvasNodeTranslationVisitor::Visit(const WhenNode& node) {
  auto condition =
    InternalTranslation(node.GetChildren().front()).Extract<Aspen::Box<bool>>();
  auto series = InternalTranslation(node.GetChildren().back());
  m_translation = instantiate<WhenTranslator>(series.GetTypeInfo())(
    std::move(condition), series);
}

Translation CanvasNodeTranslationVisitor::InternalTranslation(
    const CanvasNode& node) {
  if(auto existingTranslation = m_context->FindTranslation(node)) {
    m_translation = std::move(*existingTranslation);
  } else if(m_proxies.count(&node) == 1) {
    auto& nativeType =
      static_cast<const NativeType&>(node.GetType()).GetNativeType();
    auto& proxy = m_proxies[&node];
    m_translation = instantiate<ProxyTranslator>(nativeType)(proxy);
  } else {
    if(auto type = dynamic_cast<const NativeType*>(&node.GetType())) {
      auto& nativeType = type->GetNativeType();
      auto proxy = instantiate<ProxyBuilder>(nativeType)();
      m_proxies.insert(std::make_pair(&node, proxy));
      node.Apply(*this);
      instantiate<ProxyFinalizer>(nativeType)(proxy, *m_translation);
      m_proxies.erase(&node);
    } else {
      node.Apply(*this);
    }
    auto type = dynamic_cast<const NativeType*>(&node.GetType());
    if(type && type->GetCompatibility(OrderReferenceType::GetInstance()) ==
        CanvasType::Compatibility::EQUAL) {
      m_context->Add(Ref(node), m_translation->ToWeak());
    } else {
      m_context->Add(Ref(node), *m_translation);
    }
  }
  return *m_translation;
}

template<typename Translator>
Translation CanvasNodeTranslationVisitor::TranslateFunction(
    const CanvasNode& node) {
  auto arguments = std::vector<Translation>();
  for(auto& child : node.GetChildren()) {
    arguments.push_back(InternalTranslation(child));
  }
  auto& result = static_cast<const NativeType&>(node.GetType()).GetNativeType();
  return FunctionTranslator<Translator, ParameterCount<Translator>::value>()(
    arguments, result, *m_context);
}
