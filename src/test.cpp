#include "H5Cpp.h"
#include "h5md_io.h"
#include <thread> 
#include <chrono> 
#include <iostream>
#include <vector>

int main()
{
    std::string filename = "output.h5md";
    std::string group_name = "/h5md";

    // Create an H5MD file and write data
    H5MDWriter h5_writer(filename, "w", false);
    // H5MD_IO::initialize_step(file);

    const int nparticles = 2;
    std::vector<int> species = {1, 2}; // Example species data
    std::vector<std::vector<double>> step1(nparticles, std::vector<double>(3));
    std::vector<std::vector<double>> step2(nparticles, std::vector<double>(3));
    for (int i = 0; i < nparticles; ++i)
    {
        step1[i] = {1.1 * i, 0.0, 0.0};
        step2[i] = {1.2 * i, 0.0, 0.0};
    }
    printf("<== Writing first step... ==>\n");
    h5_writer.append_one_step(step1, 0.0, 0, species);  // First frame
    h5_writer.flush();

    // std::this_thread::sleep_for(std::chrono::minutes(20)); 
    
    // file.openFile(filename, H5F_ACC_RDWR);
    printf("<== Writing second step... ==>\n");
    h5_writer.append_one_step(step2, 0.01, 10, species);  // Second frame
    // delete file; // Clean up the file handle
    return 0;
}