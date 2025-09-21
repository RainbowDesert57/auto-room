#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <cstdlib>
#include <vector>
#include <alsa/asoundlib.h>
#include <regex>
#include <curl/curl.h>
#include <nlohmann/json.hpp>


using namespace std;
using json = nlohmann::json;

// Callback to store CURL response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


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

string escape_json(const string& s) {
    string escaped;
    for (char c : s) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            default: escaped += c;
        }
    }
    return escaped;
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
                string hf_token = "YOUR_API_KEY"; // replace with your token
                string model = "deepseek-ai/DeepSeek-R1:novita";

                // JSON payload
                prompt.erase(prompt.find_last_not_of(" \n\r\t")+1);
                prompt.erase(0, prompt.find_first_not_of(" \n\r\t"));

                string safePrompt = escape_json(prompt);
                string data = R"({"model": ")" + model + R"(", "messages":[{"role":"user","content":")" + safePrompt + R"("}]})";


    CURL* curl = curl_easy_init();
    if(curl) {
        string response;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + hf_token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, "https://router.huggingface.co/v1/chat/completions");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        } else {
            try {
                auto j = json::parse(response);
                string answer = j["choices"][0]["message"]["content"];

                // Remove <think>...</think> blocks
                answer = regex_replace(answer, regex("<think>[\\s\\S]*?</think>", regex::icase), "");

                cout << "Assistant answer:\n" << answer << endl;
            } catch (const std::exception& e) {
                cerr << "JSON parsing error: " << e.what() << endl;
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

        }
    }

    return 0;
}

