
#include <FluidCLIWrapper.hpp>
#include <clients/rt/SinesClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTSinesClient>::run(argc, argv);
}
