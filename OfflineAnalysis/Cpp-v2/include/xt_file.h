#ifndef XT_FILE_H
#define XT_FILE_H

#include <string>
#include <vector>
#include "xt_data.h"

using namespace std;

const string XT_FILE_EXT    = ".txt";
// const string XT_FILE_PATH   = "/Users/MChen/Workspace/XTaint/OfflineAnalysis/Cpp-v2/test-file/";
const string XT_FILE_PATH   = "/home/mchen/Workspace/XTaint/OfflineAnalysis/Cpp-v2/test-file/";
// const string XT_RESULT_PATH = "/Users/MChen/Workspace/XTaint/OfflineAnalysis/Cpp-v2/test-result/";
const string XT_RESULT_PATH = "/home/mchen/Workspace/XTaint/OfflineAnalysis/Cpp-v2/test-result/";

const string XT_FILE_FAKE_DATA  = "test-aes-128-1B-all-identify-in-out-buffer-fake-data";
const string XT_FILE_AES        = "test-aes-128-1B-all-marks";
const string FILE_REFINE        = "test-aes-128-oneblock-sizemark-refine";

const string XT_PREPROCESS      = "-preprocess";
const string XT_ADD_SIZE_INFO   = "-add-size-info";
const string XT_ALIVE_BUF       = "-alive-buf";
const string CONT_BUF           = "-cont-buf";
const string ALL_PROPAGATE_RES  = "-all-propagate-res";

class XT_File
{
private:
    std::string path_r;
public:
    XT_File(std::string);

    std::vector<std::string> read();
    void write(std::string, std::vector<std::string> &);
    void write_continue_buffer(string, vector<Func_Call_Cont_Buf_t> &);
    void write_all_propagate_result(string path, vector<NodePropagate> &allPropagateRes);
}; 
#endif
