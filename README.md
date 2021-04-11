# C-RestAPI

This is a C++ REST API , that recovers info from sites sucha as  itunes and tvmaze, and returns it in a json format

## How to build

1. Install  C++ REST SDK is a Microsoft project.

With apt-get on Debian/Ubuntu

        $ sudo apt-get install libcpprest-dev
          
2. Clone the repository.

3. Execute the below command: 

        $ g++ -std=c++11 main.cpp -o main.run -lboost_system -lcrypto -lssl -lcpprest
4. Run
        $ ./main.run
          
4. To test the rest API you can execute
        $ curl --header "Content-Type: application/json" --request POST --data '{"keyword":"wu tang"}' http://localhost:9000/api/search
          
