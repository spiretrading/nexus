#!/bin/bash
DIRECTORY="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd -P)"
exec "$DIRECTORY/../../Nexus/version.sh" "OTC_LINK_MARKET_DATA_FEED_CLIENT"
