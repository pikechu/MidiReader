#include "MidiReader.h"
#include "error_def.h"

void bzero(void* ptr, size_t sz)
{
    memset(ptr, 0, sz);
}

// ´óÐ¡¶Ë×ª»»
void EndianSwap(char *pData, int length)
{
    if (length <= 1) return;
    if (length == 2)
    {
        std::swap(pData[0], pData[1]);
    }
    if (length == 4)
    {
        std::swap(pData[0], pData[3]);
        std::swap(pData[1], pData[2]);
    }
    if (length == 8)
    {
        std::swap(pData[0], pData[7]);
        std::swap(pData[1], pData[6]);
        std::swap(pData[2], pData[5]);
        std::swap(pData[3], pData[4]);
    }
}

void MidiReader::buff_clean()
{
    bzero(static_cast<void*>(buff.get()), file_size);
}

void MidiReader::init()
{
    fs.seekg(0, fs.end);
    if (file_size < fs.tellg())
    {
        file_size = (int)fs.tellg();
        buff.reset(new char[file_size]);
    }
    fs.seekg(0);
}

MidiReader::MidiReader()
{
    file_size = 0;
    current_pos = 0;
    fs.clear();
    buff_clean();
};

MidiReader::MidiReader(std::string file_path)
{
    current_pos = 0;
    fs.open(file_path);
    if (!is_file_opened())
    {
        PRINT_ERROR(ERROR_READ_FILE)
    }
    init();
    is_read_header_ok = false;
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
    init();
    return true;
}

bool MidiReader::read(int byte_num)
{
    if (current_pos + byte_num > file_size)
    {
        PRINT_ERROR(ERROR_OVER_RANGE)
        return false;
    }
    buff_clean();
    fs.read(buff.get(), byte_num);
    current_pos += byte_num;
    return true;
}

template<typename T>
bool MidiReader::read_type(const T &t, void* addr, size_t len)
{
    size_t sz = sizeof(t);
    if (read(sz))
    {
        memcpy_s(addr, sz, buff.get(), sz);
        EndianSwap((char *)&t, len ? len : sz);   
        return true;
    }
    else
    {
        PRINT_ERROR(ERROR_READ_HEADER)
            return false;
    }
    RaiseException()
}

bool MidiReader::read_header()
{
    if (read_type(midi_file.header.m_magic, &midi_file.header.m_magic, 1) &&
        read_type(midi_file.header.m_seclen, &midi_file.header.m_seclen) &&
        read_type(midi_file.header.m_format, &midi_file.header.m_format) &&
        read_type(midi_file.header.m_ntracks, &midi_file.header.m_ntracks) &&
        read_type(midi_file.header.m_tickdiv, &midi_file.header.m_tickdiv))
    {
        is_read_header_ok = true;
        return true;
    }
    return false;
}

bool MidiReader::read_file()
{
    read_header();
    if (is_read_header_ok && read_track())
    {
        return true;      
    }
    return false;
}

