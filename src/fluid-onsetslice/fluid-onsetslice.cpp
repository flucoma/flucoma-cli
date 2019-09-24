
#include <FluidCLIWrapper.hpp>
#include <clients/rt/OnsetSlice.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTOnsetSlice>::run(argc, argv);
}
