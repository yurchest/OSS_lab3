#include <iostream>
#include <stdio.h>
#include <windows.h>

#define BUFSIZE 1024


using namespace std;

int main() {

    cout << "Enter name of pipe: ";

    string name;
    cin >> name;

    string path = "\\\\.\\pipe\\" + name;

    HANDLE hPipe = CreateFileA( 
        path.c_str(),   // pipe name 
        GENERIC_READ | GENERIC_WRITE, // read and write access 
        0,              // no sharing 
        NULL,           // default security attributes
        OPEN_EXISTING,  // opens existing pipe 
        FILE_ATTRIBUTE_NORMAL,   //  attributes 
        NULL);          // no template file 
    

    // Break if the pipe handle is valid. 
    if (hPipe == INVALID_HANDLE_VALUE)
    {
        printf("Open Pipe Error Error: %d\n", GetLastError());
        return -1;
    }



    while(1){
        printf(">> ");
        string command;
        getline(cin, command);
        // cin >> command;

        // Send command
        if (!WriteFile(hPipe, command.c_str(), command.size(), NULL, NULL))
        {
            auto Error = GetLastError();
            if (Error != ERROR_IO_PENDING)
            {
                printf("WriteFile Error: %d", Error);
                CloseHandle(hPipe);
                return -2;
            }
        }

        //Read 
        if (command != "quit")
        {   
            string response(1024, '\0');
            if (!ReadFile(hPipe, &response[0], response.size(), NULL, NULL))
            {
                auto Error = GetLastError();
                if (Error != ERROR_IO_PENDING)
                {
                    printf("ReadFile Error: %d\n", Error);
                    CloseHandle(hPipe);
                    return -3;
                }
            }
            
            printf("%s\n", response.c_str());
            
        }
        else
            break;

    }

    CloseHandle(hPipe);

    printf("End.\n");

    return 0;
}