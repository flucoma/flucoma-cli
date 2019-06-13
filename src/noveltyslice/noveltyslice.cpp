
#include <FluidCLIWrapper.hpp>
#include <clients/rt/NoveltySlice.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTNoveltySlice>::run(argc, argv);
}
