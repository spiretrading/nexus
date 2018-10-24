#include "web_portal/web_portal/web_portal_session.hpp"

using namespace Beam;
using namespace Beam::WebServices;
using namespace Nexus;
using namespace Nexus::WebPortal;
using namespace std;

WebPortalSession::WebPortalSession(string id)
    : AuthenticatedSession{std::move(id)} {}
