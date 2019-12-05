#include "Spire/CanvasTests/TranslationTester.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslator.hpp"
#include "Spire/Canvas/Tasks/Executor.hpp"

using namespace Spire;
using namespace Spire::Tests;

void TranslationTester::TestTranslatingConstant() {
  auto value = IntegerNode(100);
  auto executor = Executor();
  auto context = CanvasNodeTranslationContext(
  auto translation = Translate(value);
}
