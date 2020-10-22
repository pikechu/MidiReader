#include <fstream>
#include <iostream>
#include <assert.h>

using namespace std;

void test_fstream()
{
    const string file_name = "D:\\MidiReader\\src\\test.txt";
    fstream fs(file_name);
    assert(fs.is_open(), "file not open");
    char first[4] = {};
    fs.read(first, 4);
    cout << first;
    system("pause");
}

int main()
{
    test_fstream();
    return 0;
}