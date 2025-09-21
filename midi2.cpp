#include "MidiFile.h"
#include "Options.h"
#include <iostream>
#include <iomanip>


using namespace smf;

#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>
#include<set>
#include<map>
#include<fstream>
#include<algorithm>
#include<cstdio>
int rmfile(const char* fn);

// 显示帮助信息
static void showHelp(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n"
        << "\n"
        << "Coptions:\n"
        << "  -h, --help            show this help message and exit\n"
        << "  -i, --input INPUT     input midi file | 输入的 MIDI 文件路径\n"
        << "  -w, --wavfile WAVFILE wav file to be used as reference | 作为参考使用的 WAV 文件（用于音色或采样）\n"
        << "  -o, --output OUTPUT   output directory | 输出文件的保存目录\n"
        << "  -t, --midiTrack MIDITRACK\n"
        << "                        the track number of the midi file to be used (-1 means the last track) | 要使用的 MIDI 文件中的轨道编号（-1 表示最后一轨）\n"
        << "  -s, --sampleRate SAMPLERATE\n"
        << "                        sample rate to be used for output | 输出音频所使用的采样率\n"
        << "  -B, --baseNote BASENOTE\n"
        << "                        the base note to be used for the output (60 -> C4) | 输出音频所使用的基准音符（60 对应中央 C，即 C4）\n"
        //<< "  -wv, --withVideo WITHVIDEO\n"
        //<<"                        create video file (if enabled ,wavfile will be from video file(ffmepg -i <--withVideo>"
        <<"input0.wav)) | 附加视频文件"
        << std::endl;
}
static bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

// 检查字符串是否为某个选项（支持 -x 和 --xxx）
static bool isOption(const std::string& arg, const std::string& shortOpt, const std::string& longOpt) {
    return arg == shortOpt || arg == longOpt;
}

// 从下一个参数中获取值，若不存在则报错
static std::string getArgValue(int argc, char** argv, int& i, const std::string& optName) {
    if (i + 1 >= argc) {
        std::cerr << "Error: option '" << optName << "' requires an argument.\n";
        showHelp(argv[0]);
        std::exit(1);
    }
    return argv[++i];
}
struct NoteVolumeStartTime
{
    int note;
    int volume;
    float startTime;
};
int main(int argc, char** argv) {
    #ifdef _WIN32
        //system("chcp 65001");
    #endif // _WIN32

    // 参数变量
    std::string inputMidi;
    std::string wavFile;
    std::string outputDir;
    //std::string videoFile;
    int midiTrack = -1;        // 默认最后一轨
    int sampleRate = 44100;    // 默认采样率
    int baseNote = 60;         // 默认 C4
    // 遍历所有参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (isOption(arg, "-h", "--help")) {
            showHelp(argv[0]);
            return 0;
        }
        else if (isOption(arg, "-i", "--input")) {
            inputMidi = getArgValue(argc, argv, i, arg);
        }
        else if (isOption(arg, "-w", "--wavfile")) {
            wavFile = getArgValue(argc, argv, i, arg);
        }
        //else if (isOption(arg, "-wv", "--withVideo")) {
        //    videoFile = getArgValue(argc, argv, i, arg);
        //    wavFile = videoFile.substr(0, videoFile.size() - 4) + ".wav";
        //    std::string cmd0 = "ffmpeg -i -y \"" + videoFile+"\" \""+wavFile+"\"";
        //    std::cout << cmd0<<std::endl;

        //    system(cmd0.c_str());
        //}
        else if (isOption(arg, "-o", "--output")) {
            outputDir = getArgValue(argc, argv, i, arg);
        }
        else if (isOption(arg, "-t", "--midiTrack")) {
            std::string valStr = getArgValue(argc, argv, i, arg);
            char* end;
            long val = std::strtol(valStr.c_str(), &end, 10);
            if (*end != '\0') {
                std::cerr << " midi轨道错误: " << valStr << std::endl;
                return 1;
            }
            midiTrack = static_cast<int>(val);
        }
        else if (isOption(arg, "-s", "--sampleRate")) {
            std::string valStr = getArgValue(argc, argv, i, arg);
            char* end;
            long val = std::strtol(valStr.c_str(), &end, 10);
            if (*end != '\0' || val <= 0) {
                std::cerr << "采样率需要是整数.\n";
                return 1;
            }
            sampleRate = static_cast<int>(val);
        }
        else if (isOption(arg, "-B", "--baseNote")) {
            std::string valStr = getArgValue(argc, argv, i, arg);
            char* end;
            long val = std::strtol(valStr.c_str(), &end, 10);
            if (*end != '\0') {
                std::cerr << "基准应该为正数: " << valStr << std::endl;
                return 1;
            }
            baseNote = static_cast<int>(val);
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            showHelp(argv[0]);
            return 1;
        }
    }

    // 必需参数检查
    if (inputMidi.empty()) {
        std::cerr << "输入的mid文件不能为空";
        showHelp(argv[0]);
        return 1;
    }
    if (wavFile.empty())
    {
        std::cerr << "输入的wav文件不能为空";
        showHelp(argv[0]);
        return 1;
    }
    if (outputDir.empty())
    {
        std::cerr << "输出目录不能为空";
        showHelp(argv[0]);
        return 1;
    }
    if (!fileExists(inputMidi))
    {
        std::cerr << "未找到mid文件\n";
        return 1;
    }
    if (!fileExists(wavFile))
    {
        std::cerr << "未找到wav文件\n";
        return 1;
    }
    // 输出解析结果（用于调试或后续处理）
    std::cout 
        << "  mid文件: " << inputMidi << "\n"
   /*     << "  视频文件: " << (videoFile.empty() ? "(not specified)" : videoFile) << "\n"*/
        << "  WAV文件: " << (wavFile.empty() ? "(not specified)" : wavFile) << "\n"
        << "  输出目录: " << (outputDir.empty() ? "(not specified)" : outputDir) << "\n"
        << "  MIDI 轨道: " << midiTrack << "\n"
        << "  采样率: " << sampleRate << "\n"
        << "  基准音符: " << baseNote << "\n"
    ;



    MidiFile midifile;
    //const bool ok = midifile.read("D:/py/wavableMidi/test.mid");
    const bool ok=midifile.read(inputMidi);
    if (!ok)
    {
        std::cerr << "-1";
        return -1;
    }
    midifile.doTimeAnalysis();
    midifile.linkNotePairs();

    const int tracks = midifile.getTrackCount();
    if(midiTrack<0)
    {
        midiTrack = tracks + midiTrack;
    }
    std::cout << "选择了" << midiTrack << "轨道\n";
    std::set<int> notesUsedInSelectedTrack;
    std::map<int, std::string> notesWithTheirWavFiles;

    //复制为shift0
    std::ifstream source(wavFile, std::ios::binary);
    std::ofstream destination("Shift0.wav", std::ios::binary|std::ios::trunc);
    destination << source.rdbuf();

    notesWithTheirWavFiles[baseNote]= "Shift0.wav";
    source.close(); destination.close();
    std::cout << "Ticks per Quarter Note: " << midifile.getTicksPerQuarterNote() << std::endl;
    if (tracks > 1) std::cout << "TRACKS: " << tracks << std::endl;


        //std::cout << "Seconds\t\tDuration\t\tNote\tVolume" << std::endl;
        std::cout << "Note\t|\tVolume\tStart Time\n";
        const int notesCount = midifile[midiTrack].size();
        NoteVolumeStartTime* NotesVolumesStartTimes = new NoteVolumeStartTime[notesCount];
        memset(NotesVolumesStartTimes, 0, sizeof(NoteVolumeStartTime)* notesCount);
        int notesIndex=0;
        for (int event = 0; event < notesCount; event++)
        {
            
            if (midifile[midiTrack][event].isNoteOn()){
                
                std::cout << std::dec;
                int note = (int)midifile[midiTrack][event][1];
                std::cout << note<< "\t|\t";
                notesUsedInSelectedTrack.insert(note);

                int volume = (int)midifile[midiTrack][event][2];
                float startTime = static_cast<float>(midifile[midiTrack][event].seconds);

                std::cout <<volume << "\t" << startTime;
                NotesVolumesStartTimes[notesIndex++] = {note,volume,startTime};
                std::cout << std::endl;

            }
            
        }

        std::cout << "存在" <<notesCount- notesIndex<<"个不是note_on消息\n";
        for (int note : notesUsedInSelectedTrack)
        {
            std::cout << note << std::endl;
            int noteDelta = note-baseNote;
            if (!noteDelta) //跳过不需要变调的音频
            {
                continue;
            }
            std::string noteDeltaAsString = std::to_string(noteDelta);

            std::string cmd = "pitch -i ";
            std::string wavfileWithoutExt = wavFile.substr(0, wavFile.size()-4);
            cmd +="\""+ wavFile + "\"" +" -o \""+ wavfileWithoutExt +"Shifted"+ noteDeltaAsString +".wav\"";
            cmd += " -a "+ noteDeltaAsString;
            std::cout << cmd << '\n';
            notesWithTheirWavFiles[note] = wavfileWithoutExt + "Shifted" + noteDeltaAsString + ".wav";
            system(cmd.c_str());
            //pitch.exe
            //Change pitch of a wav file.\n
            //-h: show help\n
            //-i <input file>: input wav file\n
            //-o <output file>: output wav file\n
            //-a <halfnote(s)>: half note(s) to change pitch\n
            //-L: log to console\n
            //Usage: %s [-h] -i <input file> -o <output file> -a halfnote(s) -L \n
        }

        // 
        // 
        //wavCompositorExtended <input.txt>[-o output.wav][-s <sample_rate>][-h]


        std::ofstream file;
        std::ofstream fileForVideo;
        file.open("w.txt", std::ios::out | std::ios::trunc);


 
        //);
        for (int i = 0; i < notesIndex; ++i)
        {
            const NoteVolumeStartTime& note = NotesVolumesStartTimes[i];
              //std::cout << a.note << '\t' << a.startTime << '\t' << a.volume << '\n';
            std::cout << notesWithTheirWavFiles[note.note]+" " + std::to_string(note.startTime)+" "+std::to_string(note.volume)+"\n";
            file << notesWithTheirWavFiles[note.note] + " " + std::to_string(note.startTime) + " " + std::to_string(note.volume) + "\n";
            //if (!videoFile.empty())
            //{
            //    std::cout << videoFile + " " + std::to_string(note.startTime)+"\n";
            //    fileForVideo << videoFile + " " + std::to_string(note.startTime) + "\n";

            //}
        
        }
        file.close();
        fileForVideo.close();
        delete[] NotesVolumesStartTimes;
        std::string cmd2 = "wavCompositorExtended \"w.txt\" -s " +std::to_string(sampleRate)+" -o "+outputDir+".wav";
        std::cout << cmd2 << std::endl;
        system(cmd2.c_str());
         std::cout << "清理:\n";
        for (std::map<int,std::string>::iterator it = notesWithTheirWavFiles.begin(); it != notesWithTheirWavFiles.end(); ++it) {

           if(rmfile(it->second.c_str()))
           {
               std::cout<<"删除文件失败:" << it->second << std::endl;
           }
        }
    return 0;
}
int rmfile(const char* fn)
{
    if (remove(fn) != 0)
    {

#ifdef _WIN32
        std::string cmd = "del /f \"";
#else
        std::string cmd = "rm -f \"";
#endif
        cmd += fn;
        cmd += '\"';
        return system(cmd.c_str());
    }
    else
    {
        return 0;
    }
}