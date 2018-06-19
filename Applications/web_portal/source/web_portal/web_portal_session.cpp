#include "web_portal/web_portal/web_portal_session.hpp"

using namespace Beam;
using namespace Beam::WebServices;
using namespace Nexus;
using namespace Nexus::ClientWebPortal;
using namespace std;

ClientWebPortalSession::ClientWebPortalSession(string id)
    : AuthenticatedSession{std::move(id)} {}
