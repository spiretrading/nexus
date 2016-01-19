mkdir -p ./../../Include/OasisOrderExecutionServer
printf "#define OASIS_ORDER_EXECUTION_SERVER_VERSION \""> ./../../Include/OasisOrderExecutionServer/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/OasisOrderExecutionServer/Version.hpp
printf \" >> ./../../Include/OasisOrderExecutionServer/Version.hpp
