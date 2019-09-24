
#include <FluidCLIWrapper.hpp>
#include <clients/nrt/NMFClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NMFClient>::run(argc, argv);
}
