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
    friend std::ostream &operator << (std::ostream &os, const DeltaTime &dt)
    {
        os << dt.total;
        return os;
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

    MetaEvent(){};
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
    std::string m_message;

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
    SysexEvent sysex_event;

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
    // len指定读取的结构大小 is_str如果为true则不进行大小端转换
    template<typename T> bool read_var(const T &t, void* addr, size_t len = 0, bool is_translate = false);
    bool read_str(std::string &str, size_t len);
    bool read_header();
    bool read_tracks();
    // print
    void print_header();
    void print_tracks();
    void print_file();

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
    // track inner
    bool read_messages(MidiMessage &message);
    bool read_delta_time(DeltaTime &dt);
    bool read_meta_event(MetaEvent &me);
    bool read_sysex_event(SysexEvent &se);

};


#endif