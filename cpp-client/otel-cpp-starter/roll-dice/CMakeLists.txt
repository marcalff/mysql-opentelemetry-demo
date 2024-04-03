cmake_minimum_required(VERSION 3.1)

project(RollDiceServer)

# Set C++ standard (e.g., C++17)
set(CMAKE_CXX_STANDARD 17)

set(project_name roll-dice-server)

find_package(Threads)
find_package(oatpp REQUIRED)
find_package(CURL REQUIRED)
find_package(Protobuf REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(opentelemetry-cpp REQUIRED)
find_package(mysql-concpp REQUIRED)

# find_package(MySQL REQUIRED)
SET (MYSQL_LIBRARY /opt/mysql/lib/libmysqlclient.so)

# Define your project's source files
set(SOURCES
    main.cpp  # Add your source files here
)

# Create an executable target
add_executable(dice-server ${SOURCES})

target_link_libraries(dice-server PRIVATE
  oatpp::oatpp
  mysql::concpp-jdbc
  opentelemetry-cpp::otlp_http_exporter
  ${MYSQL_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT})
