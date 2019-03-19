
#include <FluidCLIWrapper.hpp>
#include <clients/nrt/NoveltyClient.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<NoveltyClient>::run(argc, argv);
}
