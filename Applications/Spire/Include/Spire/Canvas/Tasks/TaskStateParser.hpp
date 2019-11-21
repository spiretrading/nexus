#ifndef SPIRE_TASK_STATE_PARSER_HPP
#define SPIRE_TASK_STATE_PARSER_HPP
#include <Beam/Parsers/EnumeratorParser.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Tasks/Task.hpp"

namespace Spire {

  /** Matches a Task's State. */
  class TaskStateParser : public Beam::Parsers::EnumeratorParser<Task::State> {
    public:

      /** Constructs a TaskStateParser. */
      TaskStateParser();
  };
}

#endif
