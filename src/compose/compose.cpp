
#include <FluidCLIWrapper.hpp>
#include <clients/nrt/BufferComposeNRT.hpp>

int main(int argc, const char* argv[])
{
  using namespace fluid::client;
  return CLIWrapper<BufferComposeClient>::run(argc, argv);
}
