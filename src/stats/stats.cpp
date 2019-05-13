
#include <FluidCLIWrapper.hpp>
#include <clients/nrt/BufferStats.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<BufferStats>::run(argc, argv);
}
