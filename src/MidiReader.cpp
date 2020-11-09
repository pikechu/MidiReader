#include "MidiReader.h"
#include "error_def.h"

void bzero(void* ptr, size_t sz)
{
    memset(ptr, 0, sz);
}

// 大小端转换
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
bool MidiReader::read_var(const T &t, void* addr, size_t len)
{
    size_t sz = sizeof(t);
    if (read(sz))
    {
        memcpy_s(addr, sz, buff.get(), sz);
        // 文件读出来默认是大端 要转换到小端
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


bool MidiReader::read_str(std::string &str, size_t len)
{
    str.resize(len);
    if (read(len))
    {
        memcpy_s(&str[0], len, buff.get(), len);
        return true;
    }
    PRINT_ERROR(ERROR_READ_STRING)
    return false;
}


bool MidiReader::read_file()
{
    read_header();
    if (is_read_header_ok && read_tracks())
    {
        return true;
    }
    return false;
}

bool MidiReader::read_header()
{
    if (read_var(midi_file.header.m_magic, &midi_file.header.m_magic, 1) &&
        read_var(midi_file.header.m_seclen, &midi_file.header.m_seclen) &&
        read_var(midi_file.header.m_format, &midi_file.header.m_format) &&
        read_var(midi_file.header.m_ntracks, &midi_file.header.m_ntracks) &&
        read_var(midi_file.header.m_tickdiv, &midi_file.header.m_tickdiv))
    {
        is_read_header_ok = true;
        return true;
    }
    return false;
}

bool MidiReader::read_tracks()
{
    if (read_var(midi_file.tracks.m_magic, &midi_file.tracks.m_magic, 1) &&
        read_var(midi_file.tracks.m_seclen, &midi_file.tracks.m_seclen))
    {
        uint32_t remaining = midi_file.tracks.m_seclen;
        while (remaining) {
            midi_file.tracks.m_midi_messages.push_back(MidiMessage());
            if (!read_messages(midi_file.tracks.m_midi_messages.back())) return false;
            remaining -= sizeof(midi_file.tracks.m_midi_messages.back());
        }
        return true;
    }
    return false;
}


bool MidiReader::read_messages(MidiMessage &message)
{
    read_delta_time(message.m_dtime);
    read_var(message.m_status, &message.m_status);
    char &m_status = message.m_status;
    char lastStatus = 0;


    if (m_status & 0x80)
        lastStatus = m_status;
    else
        fs.seekg(-1, fs.tellg());  // FSeek(FTell() - 1);

    char m_channel = lastStatus & 0xf;
    if ((lastStatus & 0xf0) == 0x80)
    {
        read_var(message.note_off_event, &message.note_off_event, 1);
    }
    else if ((lastStatus & 0xf0) == 0x90)
    {
        read_var(message.note_on_event, &message.note_on_event, 1);
    }
    else if ((lastStatus & 0xf0) == 0xA0)
    {
        read_var(message.note_pressure_event, &message.note_pressure_event, 1);
    }
    else if ((lastStatus & 0xf0) == 0xB0)
    {
        read_var(message.controller_event, &message.controller_event, 1);
    }
    else if ((lastStatus & 0xf0) == 0xC0)
    {
        read_var(message.program_event, &message.program_event, 1);
    }
    else if ((lastStatus & 0xf0) == 0xD0)
    {
        read_var(message.channel_pressure_event, &message.channel_pressure_event, 1);
    }
    else if ((lastStatus & 0xf0) == 0xE0)
    {
        read_var(message.pitch_bend_event, &message.pitch_bend_event, 1);
    }
    else if (lastStatus == -1)
    {
        MetaEvent meta_event;
    }
    else if ((lastStatus & 0xf0) == 0xF0)
    {
        SysexEvent sysex_event;
    }
}

bool MidiReader::read_delta_time(DeltaTime &dt)
{
    uint32_t total = 0;
    char t0;
    char t1;
    char t2;
    char t3;
    if (read_var(total, &total) &&
        read_var(t0, &t0) &&
        read_var(t1, &t1) &&
        read_var(t2, &t2) &&
        read_var(t3, &t3))
    {
        dt.init(total, t0, t1, t2, t3);
        return true;
    }
    return false;
}

bool MidiReader::read_meta_event(MetaEvent &me)
{
    read_var(me.m_type, &me.m_type, 1);
    read_delta_time(me.m_length);
    Type &m_type = me.m_type;
    DeltaTime &m_length = me.m_length;
    if (m_type == META_SEQUENCE_NUM)
    {
        read_var(me.m_seqNum, &me.m_seqNum);
    }
    else if (m_type == META_TEXT)
    {
        read_str(me.m_text, m_length.total);
    }
    else if (m_type == META_COPYRIGHT)
    {
        read_str(me.m_copyright, m_length.total);
    }
    else if (m_type == META_SEQUENCE_NAME)
    {
        read_str(me.m_name, m_length.total);
    }
    else if (m_type == META_INSTRUMENT_NAME)
    {
        read_str(me.m_name, m_length.total);
    }
    else if (m_type == META_LYRIC)
    {
        read_str(me.m_lyric, m_length.total);
    }
    else if (m_type == META_MARKER)
    {
        read_str(me.m_marker, m_length.total);
    }
    else if (m_type == META_CUE_POINT)
    {
        read_str(me.m_cuePoint, m_length.total);
    }
    else if (m_type == META_PROGRAM_NAME)
    {
        read_str(me.m_programName, m_length.total);
    }
    else if (m_type == META_DEVICE_NAME)
    {
        read_str(me.m_deviceName, m_length.total);
    }
    else if (m_type == META_MIDI_CHANNEL_PREFIX)
    {
        read_var(me.m_channelPrefix, &me.m_channelPrefix);
    }
    else if (m_type == META_MIDI_PORT)
    {
        read_var(me.m_port, &me.m_port);
    }
    else if (m_type == META_END_OF_TRACK)
    {
    }
    else if (m_type == META_TEMPO)
    {
        read_var(me.m_usecPerQuarterNote, &me.m_usecPerQuarterNote);
        read_var(me.m_bpm, &me.m_bpm);
        uint32_t m_usecPerQuarterNote : 24; //位域
        uint32_t m_bpm = 60000000 / m_usecPerQuarterNote;
        fs.seekg(-1, fs.tellg());
    }
    else if (m_type == META_SMPTE_OFFSET)
    {
        char m_hours;
        char m_mins;
        char m_secs;
        char m_fps;
        char m_fracFrames;
    }
    else if (m_type == META_TIME_SIGNATURE)
    {
        char m_numerator;
        char m_denominator;
        char m_clocksPerClick;
        char m_32ndPer4th;
    }
    else if (m_type == META_KEY_SIGNATURE)
    {
        char m_flatsSharps;
        char m_majorMinor;
    }
    else
    {
        char m_data[m_length.total];
    }
}


bool MidiReader::read_sysex_event(SysexEvent &se)
{

}

