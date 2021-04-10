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
 * 
 */
#define BASEITUNESURL "http://itunes.apple.com/search?term="
#define BASETVMAZEURL "http://api.tvmaze.com/singlesearch/shows?q="


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

void processResults(const Value& array, Value& resultArray, rapidjson::Document& resultDocument, string dataType, string source){
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
            object.AddMember("id", i, allocator);
            Value s;
            const char * ctype = dataType.c_str();
            s.SetString(StringRef(ctype));
            object.AddMember("type", s, allocator);
            const char * csource = source.c_str();
            s.SetString(StringRef(csource));
            object.AddMember("source", s, allocator);
            object.AddMember("data", document, allocator);
            resultArray.PushBack(object,allocator);
        }
    }
}


int main(int argc, char *argv[]) {
    Document resultDocument;
    resultDocument.SetObject();
    Document::AllocatorType& allocator = resultDocument.GetAllocator();

    Value resultArray(Type::kArrayType);

    string url = BASEITUNESURL;
    string requestResult = "";
    int exitStatus = request("GET", url + "jack", &requestResult);
    if (exitStatus > -1){
        // 1. Parse string request into json
        const char* json = requestResult.c_str();
        Document document;
        document.Parse(json);
        // 2. Optain request results.
        const Value& results = document["results"];
        assert(results.IsArray());
        // 
        processResults(results, resultArray, resultDocument, "track", "itunes");
        //
    } 

    resultDocument.AddMember("result", resultArray, allocator);
        /*
        for (SizeType i = 0; i < results.Size(); i++) // Uses SizeType instead of size_t
            const Value& resultElement = results[i]; 
            assert(resultElement.HasMember("trackName"));
            assert(resultElement["trackName"].IsString());
            cout << i << resultElement["trackName"] << endl  ;
        */       

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    resultDocument.Accept(writer);
    std::cout << buffer.GetString() << std::endl;
    return 0;
}