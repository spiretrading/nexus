mkdir -p ./../../Include/OasisOrderExecutionServer
printf "#define OASIS_ORDER_EXECUTION_SERVER_VERSION \""> ./../../Include/OasisOrderExecutionServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/OasisOrderExecutionServer/Version.hpp
printf \""\n" >> ./../../Include/OasisOrderExecutionServer/Version.hpp
