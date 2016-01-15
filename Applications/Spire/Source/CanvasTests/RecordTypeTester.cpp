#include "Spire/CanvasTests/RecordTypeTester.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"

using namespace Spire;
using namespace Spire::Tests;
using namespace std;

void RecordTypeTester::TestEmptyRecord() {
  auto& type = RecordType::GetEmptyRecordType();
  CPPUNIT_ASSERT(type.GetFields().empty());
  CPPUNIT_ASSERT(type.GetName() == "Empty Record");
  auto explicitType = MakeRecordType(vector<RecordType::Field>());
  CPPUNIT_ASSERT(explicitType->GetFields().empty());
  CPPUNIT_ASSERT(explicitType->GetName() == "Empty Record");
  auto namedType = MakeRecordType("ABC", vector<RecordType::Field>());
  CPPUNIT_ASSERT(namedType->GetFields().empty());
  CPPUNIT_ASSERT(namedType->GetName() == "ABC Record");
}
