#include "Spire/Canvas/TaskNodes/TaskStateParser.hpp"
#include <Beam/Collections/EnumIterator.hpp>
#include <boost/lexical_cast.hpp>

using namespace Beam;
using namespace Beam::Parsers;
using namespace boost;
using namespace Spire;

TaskStateParser::TaskStateParser()
  : EnumeratorParser<Task::State>(begin(MakeRange<Task::State>()),
      end(MakeRange<Task::State>()), lexical_cast<std::string, Task::State>) {}
