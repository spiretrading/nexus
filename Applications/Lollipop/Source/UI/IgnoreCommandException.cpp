#include "Spire/UI/IgnoreCommandException.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

IgnoreCommandException::IgnoreCommandException()
    : runtime_error("") {}

IgnoreCommandException::IgnoreCommandException(const string& message)
    : runtime_error(message) {}

IgnoreCommandException::~IgnoreCommandException() {}
