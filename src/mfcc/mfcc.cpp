
#include <FluidCLIWrapper.hpp>
#include <clients/rt/MFCCClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTMFCCClient>::run(argc, argv);
}
