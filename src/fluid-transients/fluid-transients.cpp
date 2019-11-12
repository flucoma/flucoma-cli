
#include <FluidCLIWrapper.hpp>
#include <clients/rt/TransientClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTTransientsClient>::run(argc, argv);
}
