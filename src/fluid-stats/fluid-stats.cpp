
#include <FluidCLIWrapper.hpp>
#include <clients/nrt/BufStatsClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<BufStatsClient>::run(argc, argv);
}
