#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>

using namespace std;

// Replace with your actual OpenAI API key
const string API_KEY = "YOUR_API_KEY_HERE";

string gpt_request(const string &prompt) {
    string cmd = "curl -s https://api.openai.com/v1/responses "
                 "-H \"Content-Type: application/json\" "
                 "-H \"Authorization: Bearer " + API_KEY + "\" "
                 "-d '{\"model\": \"gpt-5\", \"input\": \"" + prompt + "\"}' "
                 " -o response.json";

    system(cmd.c_str());

    ifstream inFile("response.json");
    string line, result;
    while (getline(inFile, line)) result += line;
    inFile.close();

    // For simplicity, just return the raw JSON. You can parse with nlohmann/json if desired.
    return result;
}

void tts_request(const string &text, const string &outFile) {
    string cmd = "curl -s https://api.openai.com/v1/audio/speech "
                 "-H \"Content-Type: application/json\" "
                 "-H \"Authorization: Bearer " + API_KEY + "\" "
                 "-d '{\"model\":\"gpt-voice\",\"voice\":\"alloy\",\"input\":\"" + text + "\"}' "
                 " --output " + outFile;

    system(cmd.c_str());

    cout << "Saved TTS output to " << outFile << endl;
    cout << "Playing audio...\n";
    system(("mpg123 " + outFile).c_str()); // or aplay if wav
}

int main() {
    cout << "Enter your prompt: ";
    string prompt;
    getline(cin, prompt);

    cout << "Sending to GPT-5...\n";
    string gpt_response = gpt_request(prompt);

    cout << "GPT response (raw JSON):\n" << gpt_response << endl;

    cout << "\nConverting to speech...\n";
    tts_request(prompt, "output.mp3");

    return 0;
}

