
#include <FluidCLIWrapper.hpp>
#include <clients/rt/SinesClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTSines>::run(argc, argv);
}
