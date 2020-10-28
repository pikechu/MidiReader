#include "MidiReader.h"
#include "error_def.h"

void bzero(void* ptr, size_t sz)
{
    memset(ptr, 0, sz);
}

// ´óÐ¡¶Ë×ª»»
void EndianSwap(char *pData, int startIndex, int length)
{
    int i, start;
    start = startIndex;
    uint8_t tmp;
    for (i = 0; i < length; i += 2)
    {
        tmp = pData[start + i];
        pData[start + i] = pData[start + i + 1];
        pData[start + i + 1] = tmp;
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
        file_size = fs.tellg();
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

bool MidiReader::read_header()
{

    std::cout << sizeof(MidiHeader) << " " << MidiHeader::get_header_size() << std::endl;
    int header_size = MidiHeader::get_header_size();
    if (!read(header_size))
    {
        PRINT_ERROR(ERROR_READ_HEADER)
        return false;
    }
    MidiHeader midi_header;
    memcpy_s(&midi_header, header_size, buff.get(), header_size);
    EndianSwap((char *)&(midi_header.m_magic) + 4, 0, header_size - sizeof(midi_header.m_magic));
    // test
    std::cout << midi_header.m_magic << "add=" << &midi_header.m_magic << std::endl;
    std::cout << midi_header.m_seclen << "add=" << &midi_header.m_seclen << std::endl;
    std::cout << midi_header.m_format << "add=" << &midi_header.m_format << std::endl;
    std::cout << midi_header.m_ntracks << "add=" << &midi_header.m_ntracks << std::endl;
    std::cout << midi_header.m_tickdiv << "add=" << &midi_header.m_tickdiv << std::endl;

    return true;
}

