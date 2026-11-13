#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include <abb_rws/rws_interface.h>
#include <abb_rws/rws_rapid.h>

using namespace abb::rws;

namespace
{
  using RWSVersion = POCOClient::RWSVersion;

  struct Options
  {
    std::string host = "127.0.0.1";
    unsigned short port = 80;
    std::string protocol = "legacy";
    std::string task = "T_ROB1";
    std::string module = "MainModule";
    std::string variable = "test_var";
  };

  Options parseOptions(const int argc, char **argv)
  {
    Options options;
    if (argc > 1)
    {
      options.host = argv[1];
    }
    if (argc > 2)
    {
      const unsigned long port = std::stoul(argv[2]);
      if (port > 65535)
      {
        throw std::invalid_argument("port must be in the range 1..65535");
      }
      options.port = static_cast<unsigned short>(port);
    }
    if (argc > 3)
    {
      options.protocol = argv[3];
    }
    if (argc > 4)
    {
      options.task = argv[4];
    }
    if (argc > 5)
    {
      options.module = argv[5];
    }
    if (argc > 6)
    {
      options.variable = argv[6];
    }
    return options;
  }

  RWSVersion parseVersion(const std::string &protocol)
  {
    if (protocol == "legacy" || protocol == "http" || protocol == "rws1")
    {
      return RWSVersion::RWS1;
    }
    if (protocol == "modern" || protocol == "https" || protocol == "rws2")
    {
      return RWSVersion::RWS2;
    }
    if (protocol == "auto")
    {
      return RWSVersion::AUTO;
    }
    throw std::invalid_argument("protocol must be legacy/http/rws1, modern/https/rws2, or auto");
  }

  RWSVersion resolvedVersion(const RWSVersion version, const unsigned short port)
  {
    return version == RWSVersion::AUTO && port == 443 ? RWSVersion::RWS2 : version == RWSVersion::AUTO ? RWSVersion::RWS1
                                                                                                       : version;
  }

  std::unique_ptr<RWSInterface> makeClient(const Options &options, const RWSVersion version)
  {
    Poco::Net::Context::Ptr context;
    if (resolvedVersion(version, options.port) == RWSVersion::RWS2)
    {
      // RobotWare virtual controllers normally use a self-signed certificate.
      context = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE,
                                       "",
                                       "",
                                       "",
                                       Poco::Net::Context::VERIFY_NONE);
    }

    return std::make_unique<RWSInterface>(options.host,
                                          options.port,
                                          SystemConstants::General::DEFAULT_USERNAME,
                                          SystemConstants::General::DEFAULT_PASSWORD,
                                          version,
                                          context);
  }

  int runRAPIDVariableTest(RWSInterface &client, const Options &options, const RWSVersion version)
  {
    std::cout << "Reading " << options.task << "/" << options.module << "/" << options.variable << "...\n";

    const std::vector<RWSInterface::RAPIDModuleInfo> modules = client.getRAPIDModulesInfo(options.task);
    std::cout << "Modules reported for " << options.task << ":";
    for (const auto &module : modules)
    {
      std::cout << " " << module.name;
    }
    std::cout << "\n";

    RAPIDNum initial_data;
    if (!client.getRAPIDSymbolData(options.task, options.module, options.variable, &initial_data))
    {
      std::cerr << "[ERROR] Could not read the initial RAPID value.\n"
                << "Last RWS transaction: " << client.getLogTextLatestEvent(true) << "\n";
      return 1;
    }
    std::cout << "Initial value: " << initial_data.value << '\n';

    const float new_value_number = initial_data.value == 10.0f ? 20.0f : 10.0f;
    const RAPIDNum new_value(new_value_number);
    bool mastership = false;

    if (version == RWSVersion::RWS2)
    {
      std::cout << "Requesting RWS 2.0 mastership...\n";
      mastership = client.requestMasterShip();
    }

    const bool write_ok = client.setRAPIDSymbolData(options.task, options.module, options.variable, new_value);
    if (mastership)
    {
      client.releaseMasterShip();
    }
    if (!write_ok)
    {
      std::cerr << "[ERROR] Could not write the RAPID value.\n";
      return 1;
    }

    RAPIDNum verified_data;
    if (!client.getRAPIDSymbolData(options.task, options.module, options.variable, &verified_data))
    {
      std::cerr << "[ERROR] Could not read back the RAPID value.\n";
      return 1;
    }

    std::cout << "Verified value: " << verified_data.value << '\n';
    if (verified_data.value != new_value.value)
    {
      std::cerr << "[ERROR] Read-back value does not match the written value.\n";
      return 1;
    }

    std::cout << "[OK] RWS variable read/write test completed.\n";
    return 0;
  }
}

int main(int argc, char **argv)
{
  try
  {
    const Options options = parseOptions(argc, argv);
    const RWSVersion version = parseVersion(options.protocol);

    std::cout << "=== abb_rws RWS protocol test ===\n"
              << "Controller: " << options.host << ':' << options.port << '\n'
              << "Protocol: " << options.protocol << " ("
              << (resolvedVersion(version, options.port) == RWSVersion::RWS2 ? "HTTPS / RWS 2.0" : "HTTP / RWS 1.0")
              << ")\n";

    std::unique_ptr<RWSInterface> client = makeClient(options, version);
    return runRAPIDVariableTest(*client, options, resolvedVersion(version, options.port));
  }
  catch (const std::exception &error)
  {
    std::cerr << "[EXCEPTION] " << error.what() << '\n';
    return 1;
  }
}
