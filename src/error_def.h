#ifndef ERROR_DEF_h
#define ERROR_DEF_h

#define PRINT_ERROR(error_code) std::cout << get_error_msg(error_code) << std::endl;

enum ERROR_CODE
{
    // 成功
    OK = 1,
    // 读取文件错误
    ERROR_READ_FILE = -1,
    // 写入文件错误
    ERROR_WRITE_FILE = -2,
    // 读取位置超出文件大小
    ERROR_OVER_RANGE = -3,
    // 读取文件头错误
    ERROR_READ_HEADER = -4,
};


const char* get_error_msg(ERROR_CODE error_code)
{
    switch (error_code)
    {
    case OK:
        return "operation success";
        break;
    case ERROR_READ_FILE:
        return "read failed";
        break;
    case ERROR_WRITE_FILE:
        return "write failed";
        break;
    case ERROR_OVER_RANGE:
        return "over range to read file";
        break;
    case ERROR_READ_HEADER:
        return "read header failed";
        break;
    default:
        return "";
        break;
    }
}

#endif