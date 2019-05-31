
#include <FluidCLIWrapper.hpp>
#include <clients/rt/RTNoveltySlice.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTRTNoveltySlice>::run(argc, argv);
}
