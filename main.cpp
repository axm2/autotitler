#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

// for convenience
using json = nlohmann::json;

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main (){
	
    std::string app_api_key = "";
    if (const char* env_p = std::getenv("ACOUSTID_KEY"))
        //std::cout << env_p << std::endl;
        app_api_key = std::string (env_p);
    else{
        std::cout << "NO API KEY FOUND" << std::endl;
        return 1;
    }
        //app_api_key = "ZKnVrQZ0du";
    char* cmd = "fpcalc fjj.flac -json";
	std::string fpcalc_output = exec(cmd);
	
    auto j = json::parse(fpcalc_output);
    //std::cout << j["fingerprint"] << std::endl;
    std::string url = "https://api.acoustid.org/v2/lookup";
    std::string clienturlss = "?client=" + app_api_key;
    std::string durationss = "&duration=" + std::to_string(j["duration"].get<int>());
    std::string fingerprintss = "&fingerprint=" + j["fingerprint"].get<std::string>();
    std::string metass = "&meta=recordings+releasegroups+compress";
    std::string final_url = url + clienturlss + metass + durationss + fingerprintss;
    

    CURL *curl;
	CURLcode res;
	std::string readBuffer;

  	curl = curl_easy_init();
  	if(curl) {
        // We have to combine, then convert to cstr then pass to CURL
        curl_easy_setopt(curl,CURLOPT_URL,final_url.c_str());
    	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    	res = curl_easy_perform(curl);
    	curl_easy_cleanup(curl);

        auto j_res = json::parse(readBuffer);
        std::cout << j_res["results"][0]["recordings"][0]["title"] << std::endl;
    	//std::cout << readBuffer << std::endl;
  	}
	return 0;
}
