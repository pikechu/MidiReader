#include <fstream>
#include <iostream>
//#include <assert.h>
//#include "MidiReader.h"
//
//using namespace std;
//
//void test_fstream()
//{
//    const string file_name = "D:\\MidiReader\\src\\test.txt";
//    fstream fs(file_name);
//    assert(fs.is_open());
//    char first[4] = {};
//    fs.read(first, 4);
//    cout << first;
//    system("pause");
//}
//
//void test_read()
//{
//    const char * file_name = "D:\\midi-files\\YS01.MID";
//    MidiReader reader;
//    reader.open_file(file_name);
//    reader.read_header();
//}


int main()
{
    //test_read();
    std::string str;
    str.resize(10);
    char *c = new char[10];
    for (int i = 'a'; i < 'a' + 10; ++i)
    {
        *(c + i - 'a') = i;
    }
    memcpy_s(&str[0], 10, c, 10);
    return 0;
}