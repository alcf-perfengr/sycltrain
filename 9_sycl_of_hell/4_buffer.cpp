#include "cxxopts.hpp"
#include <CL/sycl.hpp>
#include <vector>

namespace sycl = cl::sycl;

int main(int argc, char **argv) {

  //  _                ___
  // |_) _. ._ _  _     |  ._  ._     _|_
  // |  (_| | _> (/_   _|_ | | |_) |_| |_
  //

  cxxopts::Options options("4_buffer", " How to use 'nd_range' ");

  options.add_options()("h,help", "Print help")(
      "g,grange", "Global Range", cxxopts::value<int>()->default_value("1"));

  auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help({"", "Group"}) << std::endl;
    exit(0);
  }

  const auto global_range = result["grange"].as<int>();
  //  _       _   _
  // |_)    _|_ _|_ _  ._
  // |_) |_| |   | (/_ |
  //

  // Crrate array
  std::vector<int> A(global_range);

  // Selectors determine which device kernels will be dispatched to.
  sycl::default_selector selector;
  // Create your own or use `{cpu,gpu,accelerator}_selector`
  {
    // Create sycl buffer.
    // Trivia: What happend if we create the buffer in the outer scope?
    sycl::buffer<sycl::cl_int, 1> bufferA(A.data(), A.size());

    sycl::queue myQueue(selector);
    std::cout << "Running on "
              << myQueue.get_device().get_info<sycl::info::device::name>()
              << "\n";

    // Create a command_group to issue command to the group
    myQueue.submit([&](sycl::handler &cgh) {
      // Create an accesor for the sycl buffer. Trust me, use auto.
      auto accessorA = bufferA.get_access<sycl::access::mode::discard_write>(cgh);
      // Submit the kernel
      cgh.parallel_for<class hello_world>(
          sycl::range<1>(global_range), 
          [=](sycl::id<1> idx) {
            // Use the accesor
            // id have some 'usefull' overwrite
            accessorA[idx] = idx[0];
          }); // End of the kernel function
    });       // End of the queue commands
  }           // End of scope, wait for the queued work to stop.

  for (size_t i = 0; i < global_range; i++)
    std::cout << "A[ " << i << " ] = " << A[i] << std::endl;
  return 0;
}
