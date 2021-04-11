/* File:     
 *     main.cpp
 *
 * Compile:  
 *    g++ -std=c++11 main.cpp -o main.run -lboost_system -lcrypto -lssl -lcpprest
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
/*


curl --header "Content-Type: application/json" \
  --request POST \
  --data '{"username":"xyz","password":"xyz"}' \
  http://localhost:9000/api/search
*/

//Functions
#include <string>
#include <iostream>
#include "include/HTTPRequest.hpp"
//Rapidjson
#include "include/rapidjson/document.h"
#include "include/rapidjson/writer.h"
#include "include/rapidjson/stringbuffer.h"

//Rest microserver
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#pragma comment(lib, "cpprest110_1_1")
#include <iostream>
#include <map>
#include <set>
#include <string>

using namespace rapidjson;
using namespace std;

#define TRACE(msg)            wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"

#define URL "http://localhost:9000/api/search"
#define BASEITUNESURL "http://itunes.apple.com/search?term="
#define BASETVMAZEURL "http://api.tvmaze.com/search/shows?q="


using namespace std;


struct requestInfo
{
    string url, posturl, type, source, resultLocation;
}; 

const struct requestInfo itunesMusic = {BASEITUNESURL, "&entity=song", "song", "itunes", "results"} ;
const struct requestInfo itunesMovies = {BASEITUNESURL, "&entity=song", "movie", "itunes", "results"} ;
const struct requestInfo tvmazeSeries = {BASETVMAZEURL, "", "serie", "tvmaze", ""} ;
const vector<const requestInfo*> searchConfig = {&itunesMusic, &itunesMovies, &tvmazeSeries};
/*---------------------------------------------------------------------
 * Function:      request
 * Purpose:       Make a get request
 * In arg:        type : the type of the request
 *                url : the url to where the request is going to be made
 *                resultAdress : where is the result going to be saved
 * Return val:    -1 erro    0 success
 */
int request(string type, string url, string* resultAdress){
    try
    {
        http::Request request(url);
        // send a get request
        const http::Response response = request.send(type);
        *resultAdress = string(response.body.begin(), response.body.end());
        return 0;
    }
    catch (const exception& e)
    {
        cout << "Request failed, error: " << e.what() << '\n';
        return -1;
    }
}

/*---------------------------------------------------------------------
 * Function:      processResults
 * Purpose:       Format a request result array , and add it to the result json
 * In arg:        array: The array that is going to be process 
 *                resultArray : The array, where the result is store
 *                resultDocument : The document (json dom), where the resultArray is store
 *                source: The request info
 * Return val:    
 */
void processResults(const Value& array, Value& resultArray, rapidjson::Document& resultDocument, const requestInfo* source){
   //Get the Dom allocator
    Document::AllocatorType& allocator = resultDocument.GetAllocator();
    //For each resutl in the array
    for (int i = 0; i < array.Size(); i++){
        if ( array[i].IsObject() ) {
            //Optain object as a string
            Value object(kObjectType);
            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> writer( sb );
            array[0].Accept( writer );
            string obj = sb.GetString();
            //Parse object and create sub Dom
            Document document(&resultDocument.GetAllocator());;
            document.Parse(obj.c_str());
            //add extra fields
            Value s;
            const char * ctype = source->type.c_str();
            s.SetString(StringRef(ctype));
            object.AddMember("type", s, allocator);
            const char * csource = source->source.c_str();
            s.SetString(StringRef(csource));
            object.AddMember("source", s, allocator);
            object.AddMember("data", document, allocator);
            //add to result array
            resultArray.PushBack(object,allocator);
        }
    }
}

/*---------------------------------------------------------------------
 * Function:      obtainInfo
 * Purpose:       Optain search result from different sources
 * In arg:        resultArray : The array, where the result is store
 *                resultDocument : The document (json dom), where the resultArray is store
 *                term : the term that is going to be search
 *                configs : request configurations
 *                source: The request info
 * Return val:    
 */
void obtainInfo(Value& resultArray, rapidjson::Document& resultDocument, string term, const vector<const requestInfo*> configs){
    for (int i = 0; i < configs.size(); i++){
        struct requestInfo config = *configs[i];
        string requestResult = "";
        int exitStatus = request("GET", config.url + term + config.posturl, &requestResult);
        if (exitStatus > -1){
           if (requestResult.size() >0) {
               // 1. Parse string request into json
               const char* json = requestResult.c_str();
               Document document;
               document.Parse(json);
               // 2. Optain request results.
               if (document.IsObject()){
                  Value& results = document;
                  if (config.resultLocation.size() > 0){
                     results = document["results"];
                  } 
                  processResults(results, resultArray, resultDocument, configs[i]);
               } 
            }
        }
    }

}


/*---------------------------------------------------------------------
 * Function:      search
 * Purpose:       Generate the result from searching a term in the designed sources
 * In arg:        term : the term that is going to be search
 * Return val:    result
 */
string search(string term){
   try {
      cout << term;
      //Initialize result dom
      Document resultDocument;
      resultDocument.SetObject();
      Document::AllocatorType& allocator = resultDocument.GetAllocator();
      //Initialize array
      Value resultArray(Type::kArrayType);
      string url, posturl, type, source;
      //Fill result array
      obtainInfo(resultArray, resultDocument, term, searchConfig);
      //Insert into the json the result
      resultDocument.AddMember("result", resultArray, allocator);
      //Dump json into a string
      StringBuffer buffer;
      Writer<StringBuffer> writer(buffer);
      resultDocument.Accept(writer);
      return buffer.GetString();
   } catch (exception const & e) {
      throw std::invalid_argument("invalid term");
   }
}

/*---------------------------------------------------------------------
 * Function:      handle_post
 * Purpose:       Handle post request
 * In arg:        request: The received request
 * Return val:    
 */
void handle_post(web::http::http_request request)
{
   TRACE("handle POST\n");
   try {
      //Obtain the request data
      string receivedObject =  request.extract_string().get();
      const char* json = receivedObject.c_str();
      Document document;
      document.Parse(json);
      string term = "";
      
      //Check that the Dom generated from the request data, is correct to search
      if (document.IsObject()){       
         if (document.HasMember("keyword")) {
            if (document["keyword"].IsString()){
               term = document["keyword"].GetString();
               //Remove blank spaces
               replace(term.begin(), term.end(), ' ', '+');
            } else {
               request.reply(web::http::status_codes::BadRequest, "Request field has wrong type");
               throw std::invalid_argument("Request field has wrong type");
            }
         } else {
            request.reply(web::http::status_codes::BadRequest, "Request doesn't contain necessary field");
            throw std::invalid_argument( "Request doesn't contain necessary field" );
         }
      }else {
         request.reply(web::http::status_codes::BadRequest, "Bad request");
         throw std::invalid_argument("Request has no object");
      } 
      TRACE("Obtaining information ...\n");
      //return result
      request.reply(web::http::status_codes::OK, search(term));
      TRACE("Response Send\n");
   } catch (exception const & e) {
      wcout << e.what() << endl;
      request.reply(web::http::status_codes::InternalError, "Internar error");
   }
}

int main()
{
   //Create listener
   web::http::experimental::listener::http_listener listener(URL);
   listener.support(web::http::methods::POST, handle_post);
   try
   {
      listener
         .open()
         .then([&listener](){TRACE(L"\nstarting to listen\n");})
         .wait();

      while (true);
   }
   catch (exception const & e)
   {
      wcout << e.what() << endl;
   }
   return 0;
}