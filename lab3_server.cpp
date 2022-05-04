#include <windows.h> 
#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <sstream>
#include <map>
#include <string>
 
#define CONNECTING_STATE 0 
#define READING_STATE 1 
#define WRITING_STATE 2 
#define INSTANCES 4 
#define PIPE_TIMEOUT 5000
#define BUFSIZE 128

using namespace std;

BOOL wait_for_connection_client(HANDLE);

int main(){

	DWORD  dwWait, cbRet, dwErr; 
	BOOL fSuccess; 
	string lpszPipename;
	cout << R"("Enter pipe name (without `.\\pipe\\`): )";
	cin >> lpszPipename;


	auto lpszPipename_full = ("\\\\.\\pipe\\" + lpszPipename).c_str();

	HANDLE hPipe= CreateNamedPipe(lpszPipename_full,
                                   PIPE_ACCESS_DUPLEX,
                                   PIPE_TYPE_MESSAGE,
                                   PIPE_UNLIMITED_INSTANCES,
                                   BUFSIZE,   // output buffer size 
         							BUFSIZE,
                                   0,
                                   NULL);                // default security attributes 

	if (hPipe == INVALID_HANDLE_VALUE) 
      {
         printf("CreateNamedPipe failed with %d.\n", GetLastError());
         return 0;
      }


    while(true){
		string keyword;
		map<string,string> storage;
		cout << "Waiting for a client...";
		if(wait_for_connection_client(hPipe)){
			cout << "Connected.\n\n";
		}


		bool quit = false; 
		while(!quit){
			printf("Waiting for the command...");
			string command(64, '\0');
			ReadFile(hPipe, &command[0], command.size(), NULL, NULL);
			istringstream parser{command};
			cout << "\nCommand: " << command;
			parser >> keyword;
			string response;

			if (keyword == "set"){
				string name;
				string value;
				parser >> name >> value;
				storage[name] = value;
				response = "acknowledged";
				cout << "\nResponse: " << response << "\n\n";
				WriteFile(hPipe, response.c_str(), response.size(), NULL, NULL);
			}

			else if(keyword == "get"){
				string key;
				parser >> key;
				if (storage.find(key.c_str()) != storage.end()){
					string value = storage[key.c_str()];
					response = "found: " + value; 
					WriteFile(hPipe, response.c_str(), response.size(), NULL, NULL);
					cout << "\nResponse: " << response << "\n\n";
				}
				else{
					response = "missing";
					WriteFile(hPipe, response.c_str(), response.size(), NULL, NULL);
					cout << "\nResponse: " << response << "\n\n";
				}
			}

			else if(keyword == "delete"){
				string key;
				parser >> key;
				if (storage.find(key.c_str()) != storage.end()){
					storage.erase(key.c_str());
					response = "deleted";
					WriteFile(hPipe, response.c_str(), response.size(), NULL, NULL);
					cout << "\nResponse: " << response << "\n\n";
				}
				else{
					response = "missing";
					WriteFile(hPipe, response.c_str(), response.size(), NULL, NULL);
					cout << "\nResponse: " << response << "\n\n";
				}
			}

			else if(keyword == "list"){
				string response;
				for (auto item : storage) {
					response = response + " " + item.first;
				}
				WriteFile(hPipe, response.c_str(), response.size(), NULL, NULL);
				cout << "\nResponse: " << response << "\n\n";
				
			}

			else if(keyword == "quit"){
				DisconnectNamedPipe(hPipe);
				quit = true;
			}

		}
		break;
	}
	CloseHandle(hPipe);
    return 0;	

}

BOOL wait_for_connection_client(HANDLE hPipe){

	bool fConnected = ConnectNamedPipe(hPipe, NULL);

	if (!fConnected){
		switch(GetLastError())
	    {
	     	case ERROR_NO_DATA:
	        printf("ConnectNamedPipe: ERROR_NO_DATA");
	        getch();
	        CloseHandle(hPipe);
	        return 0;
	     break;

	      case ERROR_PIPE_CONNECTED:
	        printf("ConnectNamedPipe: ERROR_PIPE_CONNECTED");
	        getch();
	        CloseHandle(hPipe);
	        return 0;
	     break;

	      case ERROR_PIPE_LISTENING:
	        printf("ConnectNamedPipe: ERROR_PIPE_LISTENING");
	        getch();
	        CloseHandle(hPipe);
	        return 0;
	    break;

	      case ERROR_CALL_NOT_IMPLEMENTED:
	        printf("ConnectNamedPipe: ERROR_CALL_NOT_IMPLEMENTED");
	        getch();
	        CloseHandle(hPipe);
	        return 0;
	    break;

	      default:
	        printf("ConnectNamedPipe: Error %ld\n", GetLastError());
	        getch();
	        CloseHandle(hPipe);
	        return 0;
	    break;
		}

		CloseHandle(hPipe);
    	getch();
    	return 0;
	}

	return fConnected;
}
