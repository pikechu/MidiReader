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
    for (int i = 0; i <= length / 2 - 1; ++i)
    {
        std::swap(pData[i], pData[length - i - 1]);
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
bool MidiReader::read_var(const T &t, void* addr, size_t len, bool is_translate)
{
    size_t sz = len ? len : sizeof(t);
    if (read(sz))
    {
        memcpy_s(addr, sz, buff.get(), sz);
        // 文件读出来默认是大端 要转换到小端
        if (!is_translate)EndianSwap((char *)&t, len ? len : sz);
        return true;
    }
    else
    {
        PRINT_ERROR(ERROR_READ_HEADER)
            return false;
    }
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


bool MidiReader::read_file(MidiFile &file)
{
    read_header(file.header);
    if (is_read_header_ok && read_tracks(file.tracks))
    {
        return true;
    }
    return false;
}

bool MidiReader::read_header(MidiHeader &header)
{
    if (read_var(header.m_magic, &header.m_magic, 4, true) &&
        read_var(header.m_seclen, &header.m_seclen) &&
        read_var(header.m_format, &header.m_format) &&
        read_var(header.m_ntracks, &header.m_ntracks) &&
        read_var(header.m_tickdiv, &header.m_tickdiv))
    {
        is_read_header_ok = true;
        return true;
    }
    return false;
}

bool MidiReader::read_tracks(MidiTrack &tracks)
{
    if (read_var(tracks.m_magic, &tracks.m_magic, 4, true) &&
        read_var(tracks.m_seclen, &tracks.m_seclen))
    {
        int remaining = tracks.m_seclen;
        while (remaining > 0) {
            tracks.m_midi_messages.push_back(MidiMessage());
            MidiMessage &message = tracks.m_midi_messages.back();
            if (!read_messages(message)) return false;
            remaining -= sizeof(message);
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
        //fs.seekg(-1, fs.tellg());  // FSeek(FTell() - 1);

    char m_channel = lastStatus & 0xf;
    if ((lastStatus & 0xf0) == 0x80)
    {
        read_var(message.note_off_event, &message.note_off_event, 2, true);
    }
    else if ((lastStatus & 0xf0) == 0x90)
    {
        read_var(message.note_on_event, &message.note_on_event, 2, true);
    }
    else if ((lastStatus & 0xf0) == 0xA0)
    {
        read_var(message.note_pressure_event, &message.note_pressure_event, 2, true);
    }
    else if ((lastStatus & 0xf0) == 0xB0)
    {
        read_var(message.controller_event, &message.controller_event, 2, true);
    }
    else if ((lastStatus & 0xf0) == 0xC0)
    {
        read_var(message.program_event, &message.program_event, 1, true);
    }
    else if ((lastStatus & 0xf0) == 0xD0)
    {
        read_var(message.channel_pressure_event, &message.channel_pressure_event, 1, true);
    }
    else if ((lastStatus & 0xf0) == 0xE0)
    {
        read_var(message.pitch_bend_event, &message.pitch_bend_event, 2, true);
    }
    else if (lastStatus == -1)
    {
        read_meta_event(message.meta_event);
    }
    else if ((lastStatus & 0xf0) == 0xF0)
    {
        read_sysex_event(message.sysex_event);
    }
    return true;
}

bool MidiReader::read_delta_time(DeltaTime &dt)
{
    uint32_t &total = dt.total;

    read_var(dt.t0, &dt.t0);
    total += dt.t0 & 0x7f;
    if (!(dt.t0 & 0x80)) return true;
    read_var(dt.t1, &dt.t1);
    total <<= 7;
    total += dt.t1 & 0x7f;
    if (!(dt.t1 & 0x80)) return true;
    read_var(dt.t2, &dt.t2);
    total <<= 7;
    total += dt.t2 & 0x7f;
    if (!(dt.t2 & 0x80)) return true;
    read_var(dt.t3, &dt.t3);
    total <<= 7;
    total += dt.t3 & 0x7f;
    if (!(dt.t3 & 0x80)) return true;
    return false;
}

bool MidiReader::read_meta_event(MetaEvent &me)
{
    read_var(me.m_type, &me.m_type, true);
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
        uint32_t m_usecPerQuarterNote;
        read_var(m_usecPerQuarterNote, &m_usecPerQuarterNote, 3);
        me.m_usecPerQuarterNote = m_usecPerQuarterNote;
        me.m_bpm = 60000000 / m_usecPerQuarterNote;
        //fs.seekg(-1, fs.tellg());
    }
    else if (m_type == META_SMPTE_OFFSET)
    {
        read_var(me.m_hours, &me.m_hours);
        read_var(me.m_mins, &me.m_mins);
        read_var(me.m_secs, &me.m_secs);
        read_var(me.m_fps, &me.m_fps);
        read_var(me.m_fracFrames, &me.m_fracFrames);
    }
    else if (m_type == META_TIME_SIGNATURE)
    {
        read_var(me.m_numerator, &me.m_numerator);
        read_var(me.m_denominator, &me.m_denominator);
        read_var(me.m_clocksPerClick, &me.m_clocksPerClick);
        read_var(me.m_32ndPer4th, &me.m_32ndPer4th);
    }
    else if (m_type == META_KEY_SIGNATURE)
    {
        read_var(me.m_flatsSharps, &me.m_flatsSharps);
        read_var(me.m_majorMinor, &me.m_majorMinor);
    }
    else
    {
        read_str(me.m_data, m_length.total);
    }
    return true;
}


bool MidiReader::read_sysex_event(SysexEvent &se)
{
    read_delta_time(se.m_length);
    read_str(se.m_message, se.m_length.total);
    return true;
}

void MidiReader::print_header(const MidiHeader &header)
{
    std::cout << "header : \n" <<
        "m_magic = " << header.m_magic << "\t" <<
        "m_seclen = " << header.m_seclen << "\t" <<
        "m_format = " << header.m_format << "\t" <<
        "m_ntracks = " << header.m_ntracks << "\t" <<
        "m_tickdiv = " << header.m_tickdiv << "\n";
}

void MidiReader::print_tracks(const MidiTrack &tracks)
{
    std::cout << "track : \n" <<
        "m_magic = " << tracks.m_magic << "\t" <<
        "m_seclen = " << tracks.m_seclen << "\t";
    int count = 1;
    for (const auto &msg : tracks.m_midi_messages)
    {
        std::cout << "msg" << count++ << " : \n" <<
            "m_dtime = " << msg.m_dtime << "\t" <<
            "m_status = " << msg.m_status << "\t" <<
            "lastStatus = " << msg.lastStatus << "\n";
    }
}

void MidiReader::print_file(const MidiFile &file)
{
    print_header(file.header);
    print_tracks(file.tracks);
}



