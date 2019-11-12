
#include <FluidCLIWrapper.hpp>
#include <clients/rt/NoveltySliceClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NRTNoveltySliceClient>::run(argc, argv);
}
