#include <iostream>
#include <vector>
#include <CL/cl.hpp>
#include <iomanip>
#include <chrono>
#include <thread>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifndef PROJECT_VERSION
#define PROJECT_VERSION TOSTRING(DEVELOP)
#endif
void listDevices() {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if (platforms.empty()) {
        std::cerr << "No OpenCL platforms found." << std::endl;
    } else {
        for (const auto& platform : platforms) {
            std::vector<cl::Device> devices;
            platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
            std::cout << "Platform :" << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
            std::cout << "  Version :" << platform.getInfo<CL_PLATFORM_VERSION>() << std::endl;
            std::cout << "  Profile :" << platform.getInfo<CL_PLATFORM_PROFILE>() << std::endl;
            std::cout << "  Vendor :" << platform.getInfo<CL_PLATFORM_VENDOR>() << std::endl;
            std::cout << "  Devices:" << std::endl;
            for (size_t i = 0; i < devices.size(); ++i) {
                std::cout << "\tDevice " << i << ": " << devices[i].getInfo<CL_DEVICE_NAME>() << std::endl;
                std::cout << "\t  Vendor: " << devices[i].getInfo<CL_DEVICE_VENDOR>() << std::endl;
                std::cout << "\t  Version: " << devices[i].getInfo<CL_DEVICE_VERSION>() << std::endl;
                std::cout << "\t  Driver Version: " << devices[i].getInfo<CL_DRIVER_VERSION>() << std::endl;
                std::cout << "\t  Global Memory Size: " << devices[i].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() << " bytes" << std::endl;
                std::cout << "\t  Local Memory Size: " << devices[i].getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() << " bytes" << std::endl;
                std::cout << "\t  Max Compute Units: " << devices[i].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;
                std::cout << "\t  Max Work Group Size: " << devices[i].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << std::endl;
                std::cout << "\t  Max Clock Frequency: " << devices[i].getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() << " MHz" << std::endl;
            }
            std::cout << "  Extentions:" << platform.getInfo<CL_PLATFORM_EXTENSIONS>() << std::endl;
            std::cout << std::endl;
            std::cout << "Checking only one platform.  Feel free to implement rest:) Exiting..." << std::endl;
            break;

        }
    }

}

void stressTest(int deviceIndex, int duration) {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty()) {
        std::cerr << "No OpenCL platforms found." << std::endl;
        return;
    }

    std::vector<cl::Device> devices;
    platforms[0].getDevices(CL_DEVICE_TYPE_ALL, &devices);

    if (deviceIndex >= devices.size()) {
        std::cerr << "Invalid device index." << std::endl;
        return;
    }

    cl::Device device = devices[deviceIndex];
    std::cout << "Running stress test on device: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < duration) {
        cl::Context context(device);
        cl::CommandQueue queue(context, device);

        const char* kernelSource = R"(
       __kernel void stressKernel(__global int8* data) {
           int id = get_global_id(0);
           for (int i = 0; i < 1000; ++i) {
               data[id] = data[id] * data[id] + data[id];
           }
       })";

        cl::Program::Sources sources;
        sources.push_back({kernelSource, strlen(kernelSource)});

        cl::Program program(context, sources);
        program.build({device});

        cl::Kernel kernel(program, "stressKernel");

        size_t globalMemSize = device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
        size_t localMemSize = device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();
        size_t maxComputeUnits = device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
        size_t maxWorkGroupSize = device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();

        size_t dataSize = localMemSize / sizeof(int8_t);
        std::vector<int8_t> data(dataSize, 1);
        cl::Buffer buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int8_t) * data.size(), data.data());

        kernel.setArg(0, buffer);

        cl::NDRange globalSize(data.size());
        cl::NDRange localSize(maxWorkGroupSize);

        while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < duration) {
            queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalSize, localSize);
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
            int minutes = (elapsed / 1000) / 60;
            int seconds = (elapsed / 1000) % 60;
            int milliseconds = elapsed % 1000;
            std::cout << "["
                      << std::setfill('0') << std::setw(2) << minutes << "."
                      << std::setfill('0') << std::setw(2) << seconds << "."
                      << std::setfill('0') << std::setw(3) << milliseconds
                      << "] Stress test running..." << std::endl;
        }
        queue.finish();
    }

    std::cout << "Stress test completed." << std::endl;
}

void printHelp() {
    std::cout << "Usage: stress-cl [options]\n"
              << "Options:\n"
              << "  -l          List OpenCL devices\n"
              << "  -d <index>  Run stress test on device with given index\n"
              << "  -t <time>   Duration of the stress test in seconds\n"
              << "  -h          Print this help message\n"
              << "  -v   Print version information\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }

    std::string option = argv[1];
    if (option == "-l") {
        listDevices();
    } else if (option == "-d" && argc >= 5) {
        try {
            int deviceIndex = std::stoi(argv[2]);
            option = argv[3];
            if (option != "-t") {
                printHelp();
                return 1;
            }
            int duration = std::stoi(argv[4]);
            stressTest(deviceIndex, duration);
        } catch (std::exception& e) {
            std::cerr << "Something Happened..." << std::endl;
            std::cerr << "\t" << e.what() << std::endl;
            return 1;
        }
    } else if (option == "-h") {
        printHelp();
    } else if (option == "-v") {
        std::cout << "stress-cl v" << PROJECT_VERSION << std::endl;
    } else {
        printHelp();
        return 1;
    }

    return 0;
}