#include "oatpp/network/Server.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"

#include "opentelemetry/exporters/ostream/span_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_http_exporter_factory.h"
#include "opentelemetry/sdk/trace/exporter.h"
#include "opentelemetry/sdk/trace/processor.h"
#include "opentelemetry/sdk/trace/simple_processor_factory.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/trace/provider.h"

#include <mysql/jdbc.h>

#include <cstdlib>
#include <ctime>
#include <string>

using namespace std;
namespace trace_api = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace trace_exporter = opentelemetry::exporter::trace;

namespace {
void InitTracer() {

#ifdef DEBUG
  auto exporter = trace_exporter::OStreamSpanExporterFactory::Create();
#else
  auto exporter = opentelemetry::exporter::otlp::OtlpHttpExporterFactory::Create();
#endif

  auto processor =
      trace_sdk::SimpleSpanProcessorFactory::Create(std::move(exporter));

  std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
      trace_sdk::TracerProviderFactory::Create(std::move(processor));

  // set the global trace provider
  trace_api::Provider::SetTracerProvider(provider);
}
void CleanupTracer() {
  std::shared_ptr<opentelemetry::trace::TracerProvider> none;
  trace_api::Provider::SetTracerProvider(none);
}

} // namespace

static const char *url = "tcp://127.0.0.1:13000";
static const char *user = "root";
static const char *pass = "";
static const char *database = "test";

void RecordDice(int dice) {
  try {
    sql::Driver *driver = sql::mysql::get_driver_instance();

    std::unique_ptr<sql::Connection> con{driver->connect(url, user, pass)};

    con->setSchema(database);

    std::unique_ptr<sql::Statement> stmt{con->createStatement()};

#if 0
    std::unique_ptr<sql::ResultSet> res{
        stmt->executeQuery("SELECT 'Welcome to Connector/C++' AS _message")};
#endif

    std::unique_ptr<sql::PreparedStatement> prep_stmt{
        con->prepareStatement("INSERT INTO test.roll(dice) VALUES (?)")};

    prep_stmt->setInt(1, dice);
    prep_stmt->execute();

  } catch (sql::SQLException &e) {
    cout << "# ERR: SQLException in " << __FILE__;
    cout << "("
         << "EXAMPLE_FUNCTION"
         << ") on line " << __LINE__ << endl;

    cout << "# ERR: " << e.what();
    cout << " (MySQL error code: " << e.getErrorCode();
    cout << ", SQLState: " << e.getSQLState() << " )" << endl;
  }
}

class Handler : public oatpp::web::server::HttpRequestHandler {
public:
  shared_ptr<OutgoingResponse>
  handle(const shared_ptr<IncomingRequest> &request) override {
    auto tracer =
        opentelemetry::trace::Provider::GetTracerProvider()->GetTracer(
            "my-app-tracer");
    string response;

    {
      auto scoped_span =
          opentelemetry::trace::Scope(tracer->StartSpan("RollDiceServer"));

      int low = 1;
      int high = 7;
      int random = rand() % (high - low) + low;
      response = to_string(random);
      RecordDice(random);
    }

    return ResponseFactory::createResponse(Status::CODE_200, response.c_str());
  }
};

void run() {
  auto router = oatpp::web::server::HttpRouter::createShared();

  router->route("GET", "/rolldice", std::make_shared<Handler>());

  auto connectionHandler =
      oatpp::web::server::HttpConnectionHandler::createShared(router);

  auto connectionProvider =
      oatpp::network::tcp::server::ConnectionProvider::createShared(
          {"localhost", 8080, oatpp::network::Address::IP_4});

  oatpp::network::Server server(connectionProvider, connectionHandler);

  OATPP_LOGI("Dice Server", "Server running on port %s",
             connectionProvider->getProperty("port").getData());

  server.run();
}

int main() {
  oatpp::base::Environment::init();
  InitTracer();
  srand((int)time(0));
  run();
  oatpp::base::Environment::destroy();
  CleanupTracer();
  return 0;
}
