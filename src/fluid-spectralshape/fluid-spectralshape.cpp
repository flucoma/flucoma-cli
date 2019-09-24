
#include <FluidCLIWrapper.hpp>
#include <clients/rt/SpectralShapeClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTSpectralShapeClient>::run(argc, argv);
}
