#include "Spire/Canvas/Types/CanvasTypeRedefinitionException.hpp"

using namespace boost;
using namespace Spire;
using namespace std;

CanvasTypeRedefinitionException::CanvasTypeRedefinitionException()
    : CanvasTypeRedefinitionException(
      "A type with the specified name already exists.") {}

CanvasTypeRedefinitionException::CanvasTypeRedefinitionException(string message)
    : runtime_error(std::move(message)) {}

CanvasTypeRedefinitionException::~CanvasTypeRedefinitionException() {}
