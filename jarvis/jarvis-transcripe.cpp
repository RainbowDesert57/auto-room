#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstdlib>
#include <vector>
#include <alsa/asoundlib.h>

using namespace std;

// ALSA parameters
const char* device = "default";
const unsigned int sampleRate = 16000;
const snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
const unsigned int channels = 1;
const snd_pcm_uframes_t frames = 1024;

// Silence detection
const double silenceThreshold = 500; // adjust after testing
const double silenceDuration = 1.25; // seconds

// Temp file for recording
const string tempWav = "temp.wav";

// Simple WAV header writer
void writeWavHeader(ofstream &outFile, int totalAudioLen) {
    int totalDataLen = totalAudioLen + 36;
    int byteRate = sampleRate * channels * 2;
    char header[44] = {0};

    header[0]='R'; header[1]='I'; header[2]='F'; header[3]='F';
    *(int*)(header+4)=totalDataLen;
    header[8]='W'; header[9]='A'; header[10]='V'; header[11]='E';
    header[12]='f'; header[13]='m'; header[14]='t'; header[15]=' ';
    *(int*)(header+16)=16; // PCM chunk size
    *(short*)(header+20)=1; // PCM format
    *(short*)(header+22)=channels;
    *(int*)(header+24)=sampleRate;
    *(int*)(header+28)=byteRate;
    *(short*)(header+32)=channels * 2;
    *(short*)(header+34)=16;
    header[36]='d'; header[37]='a'; header[38]='t'; header[39]='a';
    *(int*)(header+40)=totalAudioLen;

    outFile.write(header, 44);
}

// Record audio until silenceDuration of quiet
void recordUntilSilence(const string &filename) {
    snd_pcm_t *capture_handle;
    snd_pcm_open(&capture_handle, device, SND_PCM_STREAM_CAPTURE, 0);
    snd_pcm_set_params(capture_handle, format, SND_PCM_ACCESS_RW_INTERLEAVED,
                       channels, sampleRate, 1, 500000);

    vector<short> buffer(frames * channels);
    vector<short> recorded;

    auto lastVoiceTime = chrono::steady_clock::now();

    ofstream outFile(filename, ios::binary);
    outFile.seekp(44); // leave space for WAV header

    while (true) {
        snd_pcm_readi(capture_handle, buffer.data(), frames);

        bool hasVoice = false;
        for (auto sample : buffer) {
            if (abs(sample) > silenceThreshold) {
                hasVoice = true;
                lastVoiceTime = chrono::steady_clock::now();
                break;
            }
        }

        recorded.insert(recorded.end(), buffer.begin(), buffer.end());

        auto now = chrono::steady_clock::now();
        double elapsed = chrono::duration<double>(now - lastVoiceTime).count();
        if (elapsed > silenceDuration && hasVoice==false) break;
    }

    // write recorded samples to WAV
    int totalAudioLen = recorded.size() * sizeof(short);
    outFile.write(reinterpret_cast<char*>(recorded.data()), totalAudioLen);
    outFile.seekp(0);
    writeWavHeader(outFile, totalAudioLen);
    outFile.close();

    snd_pcm_close(capture_handle);
}

// Call whisper-cli to transcribe temp.wav
string transcribe() {
    system(("./whisper.cpp/build/bin/whisper-cli -m ./whisper.cpp/models/for-tests-ggml-small.bin -f " + tempWav + " > temp.txt").c_str());

    ifstream inFile("temp.txt");
    string line, result;
    while (getline(inFile, line)) result += line + "\n";
    return result;
}

int main() {
    cout << "Assistant running. Say 'hey homie' or 'hey jarvis' to wake.\n";

    while (true) {
        // Listen continuously
        recordUntilSilence(tempWav); // record small clip

        string snippet = transcribe();

        // Convert to lowercase for simple keyword detection
        string lowerSnippet = snippet;
        for (auto &c : lowerSnippet) c = tolower(c);

        if (lowerSnippet.find("hey homie") != string::npos ||
            lowerSnippet.find("hey jarvis") != string::npos) {

            cout << "Wake word detected! Listening for prompt...\n";
            // Record the actual prompt until 1.25s silence
            recordUntilSilence(tempWav);
            string prompt = transcribe();

            cout << "Prompt captured:\n" << prompt << endl;

            // Here you can send 'prompt' to GPT API and get a response
            // ...
        }
    }

    return 0;
}

