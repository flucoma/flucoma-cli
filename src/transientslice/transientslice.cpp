
#include <FluidCLIWrapper.hpp>
#include <clients/rt/TransientSlice.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTTransientSlice>::run(argc, argv);
}
