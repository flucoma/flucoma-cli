
#include <FluidCLIWrapper.hpp>
#include <clients/rt/AmpSlice.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTAmpSlice>::run(argc, argv);
}
