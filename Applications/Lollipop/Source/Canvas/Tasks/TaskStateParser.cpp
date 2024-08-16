#include "Spire/Canvas/Tasks/TaskStateParser.hpp"
#include <Beam/Collections/EnumIterator.hpp>
#include <boost/lexical_cast.hpp>

using namespace Beam;
using namespace Beam::Parsers;
using namespace boost;
using namespace Spire;

TaskStateParser::TaskStateParser()
  : EnumeratorParser<Task::State>(EnumIterator(Task::State::READY),
      EnumIterator(static_cast<Task::State>(7)),
      lexical_cast<std::string, Task::State>) {}
