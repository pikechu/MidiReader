#include <fstream>
#include <iostream>
#include <assert.h>
#include "MidiReader.h"

using namespace std;

void test_fstream()
{
    const string file_name = "D:\\MidiReader\\src\\test.txt";
    fstream fs(file_name);
    assert(fs.is_open());
    char first[4] = {};
    fs.read(first, 4);
    cout << first;
    system("pause");
}

void test_read()
{
    const char * file_name = "D:\\midi-files\\YS01.MID";
    MidiReader reader;
    reader.open_file(file_name);
    MidiFile file;
    reader.read_file(file);

    reader.print_file(file);
}

void test_app()
{
    const char * file_name = "D:\\midi-files\\YS01.MID";
    ifstream ifs;
    ifs.open(file_name, ios_base::in | ios_base::binary);
    vector<char> vec;
    vec.resize(50000);
    char * buf = new char(50000);
    int count = 0;
    while (!ifs.eof())
    {
        ifs.read(vec.data(), 50000);
        cout << vec.data();
        cout << ifs.gcount();
        count++;
    }
    int a = 1;
}

void test_new()
{
    char * c = new char;
    for (int i = 0; i < 1000; ++i)
    {
        c[i] = 1;
    }
    int a = 10;
}

int main()
{
    //test_app();
    test_read();
    //test_new();
    return 0;
}