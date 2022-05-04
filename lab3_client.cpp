#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <locale.h>

using namespace std;

int main(){

   HANDLE hPipe;
	string lpszPipename;

   while(1){

      cout << R"(Enter pipe name (without `.\\pipe\\`): )";
      cin >> lpszPipename;
      auto lpszPipename_full = ("\\\\.\\pipe\\" + lpszPipename).c_str();

      hPipe = CreateFileA( 
         lpszPipename_full,   // pipe name 
         GENERIC_READ |  // read and write access 
         GENERIC_WRITE, 
         0,              // no sharing 
         NULL,           // default security attributes
         OPEN_EXISTING,  // opens existing pipe 
         0,              // default attributes 
         NULL);          // no template file 

      if(hPipe == INVALID_HANDLE_VALUE){
         fprintf(stdout,"CreateFile: Error %ld\n", 
         GetLastError());
      }
      else break;
   }  


   cout << "Connected\n\n";

   cin.ignore();
   while(1){
      printf(">> ");
      string command;
      string response(1024, '\0');
      
      getline(cin, command);

      WriteFile(hPipe, command.c_str(), command.size(),NULL, NULL);


      ReadFile(hPipe, &response[0], response.size(), NULL, NULL);
      printf("%s \n\n", response.c_str());


   }

   CloseHandle(hPipe);
	return 0;
} 