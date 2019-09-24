
#include <FluidCLIWrapper.hpp>
#include <clients/rt/MelBandsClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTMelBandsClient>::run(argc, argv);
}
