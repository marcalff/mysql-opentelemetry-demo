
# Introduction

This tutorial is based on the opentelemetry C++ Getting Started application,
available at https://opentelemetry.io/docs/languages/cpp/getting-started/

The basic tutorial is expended further, with the following changes:

* the roll-dice server sends telemetry using the OTLP HTTP protocol,
  instead of using the OStream exporter to the console
* traces are sent to an opentelemetry-collector backend
* the opentelemetry-collector forwards traces to Jaeger
* the opentelemetry-collector forwards traces to Zipkin

And finally, to show how MySQL and opentelemetry work together:

* the roll-dice server connects to a MySQL database, to record rolled dices.

# Dependencies

The amount of dependencies to build the full roll-dice server is
significant, mostly due to all the dependencies needed by opentelemetry-cpp
itself, when using the OTLP HTTP exporter.

For convenience, git submodules are added under the top level directory,
which is henceforth referred to as ${DEMO_TOPDIR}

Version of each submodule used:

* oatpp, tag 1.3.0-latest
* json, tag v3.11.3
* protobuf, tag v3.19.4
* opentelemetry-cpp, tag v1.15.0

```bash
export DEMO_TOPDIR=<the path where this repository is cloned>
```

This tutorial installs all components under ${DEMO_TOPDIR}/sandbox,
so that:

* the machine environment is preserved
* sudo privileges are not required


```bash
cd ${DEMO_TOPDIR}

git submodule init

git submodule update
```

## Oatpp

```bash
mkdir ${DEMO_TOPDIR}/build-oatpp

cd ${DEMO_TOPDIR}/build-oatpp

cmake \
  -DCMAKE_INSTALL_PREFIX=${DEMO_TOPDIR}/sandbox \
  ../third_party/oatpp

make

make install
```

## nlohmann_json

```bash
mkdir ${DEMO_TOPDIR}/build-json

cd ${DEMO_TOPDIR}/build-json

cmake \
  -DCMAKE_INSTALL_PREFIX=${DEMO_TOPDIR}/sandbox \
  -DJSON_BuildTests=OFF \
  ../third_party/json

make

make install
```

## protobuf

```bash
mkdir ${DEMO_TOPDIR}/build-protobuf

cd ${DEMO_TOPDIR}/build-protobuf

cmake \
  -DCMAKE_INSTALL_PREFIX=${DEMO_TOPDIR}/sandbox \
  -DCMAKE_BUILD_TYPE=Release \
  -Dprotobuf_BUILD_SHARED_LIBS=ON \
  -Dprotobuf_BUILD_TESTS=OFF \
  ../third_party/protobuf/cmake

make

make install
```

## opentelemetry-cpp

For convenience, the opentelemetry-cpp repository is added under
third_party/opentelemetry-cpp, as a git submodule.

opentelemetry-cpp needs a lot of dependencies,
for detailed instructions, see
https://github.com/open-telemetry/opentelemetry-cpp/blob/main/INSTALL.md

Assuming all the required dependencies are available:

* curl

the build should look like this:

```bash
mkdir ${DEMO_TOPDIR}/build-opentelemetry-cpp

cd ${DEMO_TOPDIR}/build-opentelemetry-cpp

cmake \
  -DCMAKE_INSTALL_PREFIX=${DEMO_TOPDIR}/sandbox \
  -DWITH_OTLP_HTTP=ON \
  -DBUILD_TESTING=OFF \
  ../third_party/opentelemetry-cpp

make

make install
```

## MySQL C++ connector

Download the connector from https://dev.mysql.com/downloads/connector/cpp/

Building from source is also possible,
the git repository is https://github.com/mysql/mysql-connector-cpp

The documentation is available at
https://dev.mysql.com/doc/connector-cpp/8.3/en/connector-cpp-opentelemetry.html

Assuming the installation method used is to download,
for example using
mysql-connector-c++-8.3.0-linux-glibc2.28-x86-64bit.tar.gz,

```bash
tar xvf mysql-connector-c++-8.3.0-linux-glibc2.28-x86-64bit.tar.gz \
  --strip-components=1 \
  -C ${DEMO_TOPDIR}/sandbox
```

## MySQL server

Install a server from your operating system distribution.

Alternately, download the server from https://dev.mysql.com/downloads/mysql/

Building from source is also possible,
the git repository is https://github.com/mysql/mysql-server

The documentation is available at
https://dev.mysql.com/doc/refman/8.3/en/

Assuming the installation method used is to download,
for example using
mysql-8.3.0-linux-glibc2.28-x86_64.tar.xz

```bash
tar xvf mysql-8.3.0-linux-glibc2.28-x86_64.tar.xz \
  --strip-components=1 \
  -C ${DEMO_TOPDIR}/sandbox
```

# Roll-dice server

## Build

```bash
mkdir ${DEMO_TOPDIR}/cpp-client/otel-cpp-starter/build-roll-dice

cd ${DEMO_TOPDIR}/cpp-client/otel-cpp-starter/build-roll-dice

cmake \
  -DCMAKE_INSTALL_PREFIX=${DEMO_TOPDIR}/sandbox \
  -DCMAKE_PREFIX_PATH="${DEMO_TOPDIR}/sandbox;" \
  ../roll-dice

make
```

## Run

Make sure to start:

* The MySQL server
* The opentelemetry endpoint

to get the full demo working.

```bash
cd ${DEMO_TOPDIR}/cpp-client/otel-cpp-starter/build-roll-dice

./dice-server
```

The server should print a message like:

```
[malff@malff-desktop build-roll-dice]$ ./dice-server
 I |2024-04-24 17:39:15 1713973155902753| Dice Server:Server running on port 8080
```

Open a browser on URL `http://localhost:8080/rolldice`,
it will print the dice rolled.
Hit refresh to roll more dices.

When the MySQL server is not running, the roll-dice server will show errors:

```
# ERR: SQLException in /data/malff/CODE/MARC_GITHUB/mysql-opentelemetry-demo/cpp-client/otel-cpp-starter/roll-dice/main.cpp(EXAMPLE_FUNCTION) on line 74
# ERR: Unable to connect to localhost (MySQL error code: 2003, SQLState: HY000 )
```

When the opentelemetry endpoint is not running, the roll-dice server will
show errors:

```
[Error] File: /data/malff/CODE/MARC_GITHUB/mysql-opentelemetry-demo/third_party/opentelemetry-cpp/exporters/otlp/src/otlp_http_client.cc:204 [OTLP HTTP Client] Session state: connection failed.Failed to connect to localhost port 4318: Connection refused
[Error] File: /data/malff/CODE/MARC_GITHUB/mysql-opentelemetry-demo/third_party/opentelemetry-cpp/exporters/otlp/src/otlp_http_exporter.cc:130 [OTLP TRACE HTTP Exporter] ERROR: Export 1 trace span(s) error: 1
```

To stop the roll-dice server, hit `CTRL-C`.

