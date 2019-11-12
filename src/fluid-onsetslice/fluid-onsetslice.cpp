
#include <FluidCLIWrapper.hpp>
#include <clients/rt/OnsetSliceClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTOnsetSliceClient>::run(argc, argv);
}
