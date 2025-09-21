#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>

using namespace std;

// Write binary data callback
size_t WriteAudio(void* contents, size_t size, size_t nmemb, void* userp) {
    ofstream* outFile = static_cast<ofstream*>(userp);
    outFile->write((char*)contents, size * nmemb);
    return size * nmemb;
}

int main() {
    string hf_token = "$HF_TOKEN"; // replace with your Hugging Face token
    string model = "hexgrad/Kokoro-82M";
    string text = "The answer to the universe is 42";

    // Build JSON payload
    string data = R"({"inputs":")" + text + R"("})";

    CURL* curl = curl_easy_init();
    if (curl) {
        ofstream outFile("output.wav", ios::binary);

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + hf_token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        string url = "https://api-inference.huggingface.co/models/" + model;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteAudio);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        } else {
            cout << "Saved TTS audio to output.wav" << endl;
        }

        outFile.close();
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    return 0;
}

