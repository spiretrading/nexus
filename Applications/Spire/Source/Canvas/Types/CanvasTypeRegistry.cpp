#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"
#include <boost/range/algorithm/find_if.hpp>
#include "Spire/Canvas/Types/BooleanType.hpp"
#include "Spire/Canvas/Types/CanvasTypeRedefinitionException.hpp"
#include "Spire/Canvas/Types/CurrencyType.hpp"
#include "Spire/Canvas/Types/DateTimeType.hpp"
#include "Spire/Canvas/Types/DecimalType.hpp"
#include "Spire/Canvas/Types/DestinationType.hpp"
#include "Spire/Canvas/Types/DurationType.hpp"
#include "Spire/Canvas/Types/ExecutionReportRecordType.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/Types/OrderFieldsRecordType.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/Canvas/Types/OrderStatusType.hpp"
#include "Spire/Canvas/Types/OrderTypeType.hpp"
#include "Spire/Canvas/Types/PortfolioEntryRecordType.hpp"
#include "Spire/Canvas/Types/QuoteRecordType.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/SequenceType.hpp"
#include "Spire/Canvas/Types/SideType.hpp"
#include "Spire/Canvas/Types/TextType.hpp"
#include "Spire/Canvas/Types/TickerType.hpp"
#include "Spire/Canvas/Types/TimeInForceType.hpp"
#include "Spire/Canvas/Types/TimeRangeType.hpp"
#include "Spire/Canvas/Types/VenueType.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace std;

CanvasTypeRegistry::CanvasTypeRegistry() {
  Register(BooleanType::GetInstance());
  Register(CurrencyType::GetInstance());
  Register(DateTimeType::GetInstance());
  Register(DecimalType::GetInstance());
  Register(DestinationType::GetInstance());
  Register(DurationType::GetInstance());
  Register(GetExecutionReportRecordType());
  Register(RecordType::GetEmptyRecordType());
  Register(IntegerType::GetInstance());
  Register(MoneyType::GetInstance());
  Register(GetOrderFieldsRecordType());
  Register(OrderReferenceType::GetInstance());
  Register(OrderStatusType::GetInstance());
  Register(OrderTypeType::GetInstance());
  Register(GetPortfolioEntryRecordType());
  Register(GetQuoteRecordType());
  Register(SequenceType::GetInstance());
  Register(SideType::GetInstance());
  Register(TextType::GetInstance());
  Register(TickerType::GetInstance());
  Register(TimeInForceType::GetInstance());
  Register(TimeRangeType::GetInstance());
  Register(VenueType::GetInstance());
}

boost::optional<const CanvasType&> CanvasTypeRegistry::Find(
    const string& name) const {
  auto type = boost::find_if(m_types,
    [&] (const std::shared_ptr<CanvasType>& type) {
      return type->GetName() == name;
    });
  if(type == m_types.end()) {
    return none;
  }
  return **type;
}

boost::optional<const NativeType&> CanvasTypeRegistry::Find(
    const type_info& type) const {
  auto typeIterator = boost::find_if(m_nativeTypes,
    [&] (const std::shared_ptr<NativeType>& nativeType) {
      return nativeType->GetNativeType() == type;
    });
  if(typeIterator == m_nativeTypes.end()) {
    return none;
  }
  return **typeIterator;
}

void CanvasTypeRegistry::Register(const CanvasType& type) {
  auto existingType = Find(type.GetName());
  if(existingType.is_initialized()) {
    BOOST_THROW_EXCEPTION(CanvasTypeRedefinitionException());
  }
  m_types.push_back(type);
  auto nativeType = std::dynamic_pointer_cast<NativeType>(m_types.back());
  if(nativeType != nullptr) {
    m_nativeTypes.push_back(nativeType);
  }
}

View<CanvasType> CanvasTypeRegistry::GetTypes() const {
  return make_dereference_view(m_types);
}

View<NativeType> CanvasTypeRegistry::GetNativeTypes() const {
  return make_dereference_view(m_nativeTypes);
}
