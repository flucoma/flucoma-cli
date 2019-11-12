
#include <FluidCLIWrapper.hpp>
#include <clients/rt/TransientSliceClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTTransientSliceClient>::run(argc, argv);
}
