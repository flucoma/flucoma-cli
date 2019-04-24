
#include <FluidCLIWrapper.hpp>
#include <clients/rt/PitchClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTPitchClient>::run(argc, argv);
}
