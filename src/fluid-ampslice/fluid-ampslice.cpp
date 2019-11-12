
#include <FluidCLIWrapper.hpp>
#include <clients/rt/AmpSliceClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTAmpSliceClient>::run(argc, argv);
}
