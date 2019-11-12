
#include <FluidCLIWrapper.hpp>
#include <clients/rt/HPSSClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTHPSSClient>::run(argc, argv);
}
