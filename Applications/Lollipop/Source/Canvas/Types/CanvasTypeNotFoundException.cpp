#include "Spire/Canvas/Types/CanvasTypeNotFoundException.hpp"

using namespace boost;
using namespace Spire;
using namespace std;

CanvasTypeNotFoundException::CanvasTypeNotFoundException()
    : CanvasTypeNotFoundException("The specified type was not found.") {}

CanvasTypeNotFoundException::CanvasTypeNotFoundException(string message)
    : runtime_error(std::move(message)) {}

CanvasTypeNotFoundException::~CanvasTypeNotFoundException() {}
