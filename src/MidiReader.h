#ifndef MIDI_READER_h
#define MIDI_READER_h


#include <vector>
#include <stdint.h>
#include <fstream>
#include <string>
#include <iostream>
#include <memory>


enum Format : int16_t
{
    MIDI_SINGLE = 0,
    MIDI_MULTIPLE = 1,
    MIDI_PATTERN = 2
};

enum Type : char
{
    META_SEQUENCE_NUM = 0,
    META_TEXT = 1,
    META_COPYRIGHT = 2,
    META_SEQUENCE_NAME = 3,
    META_INSTRUMENT_NAME = 4,
    META_LYRIC = 5,
    META_MARKER = 6,
    META_CUE_POINT = 7,
    META_PROGRAM_NAME = 8,
    META_DEVICE_NAME = 9,
    META_MIDI_CHANNEL_PREFIX = 0x20,
    META_MIDI_PORT = 0x21,
    META_END_OF_TRACK = 0x2f,
    META_TEMPO = 0x51,
    META_SMPTE_OFFSET = 0x54,
    META_TIME_SIGNATURE = 0x58,
    META_KEY_SIGNATURE = 0x59,
    META_SEQUENCER_EVENT = 0x7f
};

struct MidiHeader
{
    char m_magic[4];
    int32_t m_seclen;
    Format m_format;
    int16_t m_ntracks;
    int16_t m_tickdiv;


    // 防止内存对齐导致读取错误
    static int get_header_size(){ return sizeof(char) * 4 + sizeof(uint32_t) + sizeof(Format) + sizeof(short) + sizeof(short); }
    MidiHeader(){};
};

//#define TEST
#ifndef TEST
struct DeltaTime
{
    uint32_t total = 0;
    char t0;
    char t1;
    char t2;
    char t3;

    DeltaTime(){};

    void init(uint32_t tot, char t_0, char t_1, char t_2, char t_3)
    {
        total = tot;
        t0 = t_0;
        t1 = t_1;
        t2 = t_2;
        t3 = t_3;

        total += t0 & 0x7f;
        if (!(t0 & 0x80)) return;
        total <<= 7;
        total += t1 & 0x7f;
        if (!(t1 & 0x80)) return;
        total <<= 7;
        total += t2 & 0x7f;
        if (!(t2 & 0x80)) return;
        total <<= 7;
        total += t3 & 0x7f;
        if (!(t3 & 0x80)) return;
    }
};

struct MetaEvent
{
    Type m_type;
    DeltaTime m_length;
    
    //可选
    short m_seqNum;
    std::string m_text;
    std::string m_copyright;
    std::string m_name;
    std::string m_lyric;
    std::string m_marker;
    std::string m_cuePoint;
    std::string m_programName;
    std::string m_deviceName;
    char m_channelPrefix;
    char m_port;
    uint32_t m_usecPerQuarterNote : 24; //位域24
    uint32_t m_bpm = 60000000 / m_usecPerQuarterNote;
    char m_hours;
    char m_mins;
    char m_secs;
    char m_fps;
    char m_fracFrames;
    char m_numerator;
    char m_denominator;
    char m_clocksPerClick;
    char m_32ndPer4th;
    char m_flatsSharps;
    char m_majorMinor;
    std::string m_data;

    void init()
    {
        if (m_type == META_SEQUENCE_NUM)
        {
            short m_seqNum;
        }
        else if (m_type == META_TEXT)
        {
            char m_text[m_length.total];
        }
        else if (m_type == META_COPYRIGHT)
        {
            char m_copyright[m_length.total];
        }
        else if (m_type == META_SEQUENCE_NAME)
        {
            char m_name[m_length.total];
        }
        else if (m_type == META_INSTRUMENT_NAME)
        {
            char m_name[m_length.total];
        }
        else if (m_type == META_LYRIC)
        {
            char m_lyric[m_length.total];
        }
        else if (m_type == META_MARKER)
        {
            char m_marker[m_length.total];
        }
        else if (m_type == META_CUE_POINT)
        {
            char m_cuePoint[m_length.total];
        }
        else if (m_type == META_PROGRAM_NAME)
        {
            char m_programName[m_length.total];
        }
        else if (m_type == META_DEVICE_NAME)
        {
            char m_deviceName[m_length.total];
        }
        else if (m_type == META_MIDI_CHANNEL_PREFIX)
        {
            char m_channelPrefix;
        }
        else if (m_type == META_MIDI_PORT)
        {
            char m_port;
        }
        else if (m_type == META_END_OF_TRACK)
        {
        }
        else if (m_type == META_TEMPO)
        {
            uint32_t m_usecPerQuarterNote : 24; //位域
            uint32_t m_bpm = 60000000 / m_usecPerQuarterNote;
            FSeek(FTell() - 1);
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
};

// message inner structs
struct NoteOffEvent
{
    char m_note;
    char m_velocity;

    NoteOffEvent(){};
};

struct NoteOnEvent
{
    char m_note;
    char m_velocity;

    NoteOnEvent(){};
};

struct NotePressureEvent
{
    char m_note;
    char m_pressure;

    NotePressureEvent(){};
};

struct ControllerEvent
{
    char m_controller;
    char m_value;

    ControllerEvent(){};
};

struct ProgramEvent
{
    char m_program;

    ProgramEvent(){};
};

struct ChannelPressureEvent
{
    char m_pressure;

    ChannelPressureEvent(){};
};

struct PitchBendEvent
{
    char m_lsb;
    char m_msb;

    PitchBendEvent(){};
};

struct SysexEvent
{
    DeltaTime m_length;
    std::vector<char> m_message;

    SysexEvent(){};
};

struct MidiMessage
{
    DeltaTime m_dtime;
    char m_status;
    char lastStatus = 0;

    // 可选
    char m_channel;
    NoteOffEvent note_off_event;
    NoteOnEvent note_on_event;
    NotePressureEvent note_pressure_event;
    ControllerEvent controller_event;
    ProgramEvent program_event;
    ChannelPressureEvent channel_pressure_event;
    PitchBendEvent pitch_bend_event;
    MetaEvent meta_event;


    void init()
    {
        if (m_status & 0x80)
            lastStatus = m_status;
        else
            FSeek(FTell() - 1);

        char m_channel = lastStatus & 0xf;
        if ((lastStatus & 0xf0) == 0x80)
        {
            NoteOffEvent note_off_event;
        }
        else if ((lastStatus & 0xf0) == 0x90)
        {
            NoteOnEvent note_on_event;
        }
        else if ((lastStatus & 0xf0) == 0xA0)
        {
            NotePressureEvent note_pressure_event;
        }
        else if ((lastStatus & 0xf0) == 0xB0)
        {
            ControllerEvent controller_event;
        }
        else if ((lastStatus & 0xf0) == 0xC0)
        {
            ProgramEvent program_event;
        }
        else if ((lastStatus & 0xf0) == 0xD0)
        {
            ChannelPressureEvent channel_pressure_event;
        }
        else if ((lastStatus & 0xf0) == 0xE0)
        {
            PitchBendEvent pitch_bend_event;
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
    MidiMessage(){};
};

struct MidiTrack
{
    char m_magic[4];
    uint32_t m_seclen;
    std::vector<MidiMessage> m_midi_messages;

    MidiTrack(){};
};

struct MidiFile
{
    MidiHeader header;
    MidiTrack tracks;// [header.m_ntracks];

    MidiFile(){};
};
#endif

class MidiReader
{
public:
    bool open_file(std::string file_path);
    bool read_file();
    template<typename T> bool read_type(const T &t, void* addr, size_t len = 0);
    bool read_header();
    bool read_tracks();
    // track inner
    bool read_messages();

    MidiReader();
    MidiReader(std::string file_path);
    ~MidiReader();

private:
    // 文件总大小
    int file_size;
    // 当前读到的位置
    int current_pos;
   
    bool is_read_header_ok;
    std::fstream fs;

    std::shared_ptr<char> buff;

    MidiFile midi_file;

    bool is_file_opened(){ return fs.is_open(); }
    bool read(int byte_num);
    void buff_clean();
    void init();

};


#endif