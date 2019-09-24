
#include <FluidCLIWrapper.hpp>
#include <clients/rt/LoudnessClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTLoudnessClient>::run(argc, argv);
}
