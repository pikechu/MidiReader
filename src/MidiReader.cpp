#include "MidiReader.h"
#include "error_def.h"

MidiReader::MidiReader()
{
    file_size = 0;
    current_pos = 0;
    fs.clear();
    buff.clear();
};

MidiReader::MidiReader(std::string file_path)
{
    current_pos = 0;
    fs.open(file_path);
    if (!is_file_opened())
    {
        PRINT_ERROR(ERROR_READ_FILE)
    }
    fs.seekg(0, fs.end);
    file_size = fs.tellg();
    fs.seekg(0);
    buff.reserve(file_size);
    buff.clear();
};

MidiReader::~MidiReader()
{
    fs.close();
}

bool MidiReader::open_file(std::string file_path)
{
    fs.open(file_path);
    if (!is_file_opened())
    {
        PRINT_ERROR(ERROR_READ_FILE)
        return false;
    }
    fs.seekg(0, fs.end);
    file_size = fs.tellg();
    fs.seekg(0);
    buff.reserve(file_size);
    buff.clear();
    return true;
}

bool MidiReader::read(int byte_num)
{
    if (current_pos + byte_num > file_size)
    {
        PRINT_ERROR(ERROR_OVER_RANGE)
        return false;
    }
    buff.clear();
    fs.read(buff.data(), byte_num);
    current_pos += byte_num;
    return true;
}

bool MidiReader::read_header()
{
    int header_size = MidiHeader::get_struct_size();
    if (!read(header_size))
    {
        PRINT_ERROR(ERROR_READ_HEADER)
        return false;
    }
    MidiHeader* header = static_cast<MidiHeader*>(static_cast<void*>(buff.data()));
    // test
    std::cout << header->m_magic << std::endl;
    return true;
}

