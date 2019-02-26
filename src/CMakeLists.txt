file(GLOB SRC_FILES_COMMON common/*)
file(GLOB SRC_FILES_MYSQLCPP mysqlcpp/*)
file(GLOB SRC_FILE_HTTP httplib/*)
file(GLOB SRC_FILE_BATTLE  battlesvr/* )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#library
############################################################
add_library(common  ${SRC_FILES_COMMON})
add_library(mysqlcpp  ${SRC_FILES_MYSQLCPP})
add_library(httplib  ${SRC_FILE_HTTP})

#subproject
############################################################
add_executable(battlesvr ${SRC_FILE_BATTLE})
target_link_libraries(battlesvr common mysqlcpp httplib debug mysqlclientd libprotobufd optimized mysqlclient libprotobuf)
install(TARGETS battlesvr DESTINATION ${BIN_DIR}/)