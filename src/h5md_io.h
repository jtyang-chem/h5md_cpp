// #ifndef __H5MD_IO_H__
// #define __H5MD_IO_H__

// #include <H5Cpp.h>
// #include <iostream>
// #include <vector>

// namespace H5MD_IO
// {
//     using namespace H5;
//     H5File createH5mdFile(const H5std_string &fileName, const H5std_string &group_name);
//     H5File createH5File(const H5std_string &fileName);

//     void initialize_step(H5File &file);

//     void append_one_step(H5File &file, const std::vector<std::vector<double>> &step_data, double time_val, int step_val);

// } // namespace H5MD_IO

// #endif // H5MD_IO_H_

#ifndef __H5MD_IO_H__
#define __H5MD_IO_H__

#include <H5Cpp.h>
#include <vector>
#include <string>
using namespace H5;
class H5MDWriter {
    
public:
    H5MDWriter(const std::string &filename, const std::string &method = "w", const bool &isPbc = false);
    ~H5MDWriter();  // 负责关闭和清理资源

    // H5File createH5mdFile(const H5std_string &fileName, const H5std_string &group_name);
    H5File createH5mdFile(const std::string &fileName);
    void append_one_step(const std::vector<std::vector<double>> &step_data, double time_val, int step_val, std::vector<int> &species);
    // void append_one_step(const std::vector<std::vector<double>> &step_data, double time_val, int step_val);

    void flush();  // 可选手动 flush

private:
    H5::H5File file;\
    std::string filename;  // 文件名
    std::string method;   // 'w' for write, 'r' for read
    bool isPbc;  // 是否使用 PBC
};

#endif // __H5MD_IO_H__
