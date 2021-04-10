#include <string>
#include <iostream>
#include "include/HTTPRequest.hpp"
// rapidjson/example/simpledom/simpledom.cpp`
#include "include/rapidjson/document.h"
#include "include/rapidjson/writer.h"
#include "include/rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;
/* File:     
 *     main.cpp
 *
 * Compile:  
 *    c++ main.cpp -o main.run
 * 
 * libraries 
 * 
 *  https://github.com/elnormous/HTTPRequest
 *      copiar repo y sacar el archivo
 * 
 *  https://github.com/Tencent/rapidjson
 * 
 *  https://github.com/Microsoft/cpprestsdk
 * 
 * 
 */
#define BASEITUNESURL "http://itunes.apple.com/search?term="
#define BASETVMAZEURL "http://api.tvmaze.com/search/shows?q="


struct requestInfo
{
    string url, posturl, type, source, resultLocation;
}; 

const struct requestInfo itunesMusic = {BASEITUNESURL, "&entity=song", "song", "itunes", "results"} ;
const struct requestInfo itunesMovies = {BASEITUNESURL, "&entity=song", "movie", "itunes", "results"} ;
const struct requestInfo tvmazeSeries = {BASETVMAZEURL, "", "serie", "tvmaze", ""} ;
const vector<const requestInfo*> searchConfig = {&itunesMusic, &itunesMovies, &tvmazeSeries};

int request(string type, string url, string* resultAdress){
    try
    {
        // you can pass http::InternetProtocol::V6 to Request to make an IPv6 request
        http::Request request(url);

        // send a get request
        const http::Response response = request.send(type);
        *resultAdress = string(response.body.begin(), response.body.end()); // print the result
        return 0;
    }
    catch (const exception& e)
    {
        cout << "Request failed, error: " << e.what() << '\n';
        return -1;
    }
}

void processResults(const Value& array, Value& resultArray, rapidjson::Document& resultDocument, const requestInfo* source){
    Document::AllocatorType& allocator = resultDocument.GetAllocator();
    for (int i = 0; i < array.Size(); i++){
        if ( array[i].IsObject() ) {
            Value object(kObjectType);
            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> writer( sb );
            array[0].Accept( writer );
            string obj = sb.GetString();
            Document document(&resultDocument.GetAllocator());;
            document.Parse(obj.c_str());
            Value s;
            const char * ctype = source->type.c_str();
            s.SetString(StringRef(ctype));
            object.AddMember("type", s, allocator);
            const char * csource = source->source.c_str();
            s.SetString(StringRef(csource));
            object.AddMember("source", s, allocator);
            object.AddMember("data", document, allocator);
            resultArray.PushBack(object,allocator);
        }
    }
}

void obtainInfo(Value& resultArray, rapidjson::Document& resultDocument, string term, const vector<const requestInfo*> configs){
    for (int i = 0; i < configs.size(); i++){
        struct requestInfo config = *configs[i];
        string requestResult = "";
        int exitStatus = request("GET", config.url + term + config.posturl, &requestResult);
        if (exitStatus > -1){
            // 1. Parse string request into json
            const char* json = requestResult.c_str();
            Document document;
            document.Parse(json);
            // 2. Optain request results.
            Value& results = document;
            if (config.resultLocation.size() > 0){
                results = document["results"];
            } 
            assert(results.IsArray());
            // 
            processResults(results, resultArray, resultDocument, configs[i]);
            //
        }
    }

}

int main(int argc, char *argv[]) {
    Document resultDocument;
    resultDocument.SetObject();
    Document::AllocatorType& allocator = resultDocument.GetAllocator();
    Value resultArray(Type::kArrayType);
    string url, posturl, type, source;
    obtainInfo(resultArray, resultDocument, "jack", searchConfig);
    resultDocument.AddMember("result", resultArray, allocator);
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    resultDocument.Accept(writer);
    std::cout << buffer.GetString() << std::endl;
    return 0;
}