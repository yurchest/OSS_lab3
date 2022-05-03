#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <sstream>
#include <map>
#include <string.h>

#define BUF_SIZE2 64

using namespace std;

int func_set(HANDLE pipe, map<string, string> &data, istringstream &parser);
int func_get (HANDLE pipe, map<string, string> &data, istringstream &parser);
int func_list(HANDLE pipe, map<string, string> &data);
int func_delete(HANDLE pipe, map<string, string> &data, istringstream &parser);

int main()
{
    bool connected = false;
    map<string, string> data;
    cout << "Enter name of pipe: ";
    string name;
    cin >> name;

    string path = "\\\\.\\pipe\\" + name;

    HANDLE pipe = CreateNamedPipeA(path.c_str(),
                                   PIPE_ACCESS_DUPLEX,
                                   PIPE_TYPE_MESSAGE,
                                   PIPE_UNLIMITED_INSTANCES,
                                   BUF_SIZE2,
                                   BUF_SIZE2,
                                   0,
                                   NULL);

    if (pipe == INVALID_HANDLE_VALUE)
    {
        printf("Create Pipe Error: %d\n", GetLastError());
        return 1;
    }

    while (true)
    {
        if (ConnectNamedPipe(pipe, NULL))
            connected = true;
        else
            connected = false;

        if (connected)
        {

            printf("connected.\n");

            bool quit = false;

            while (!quit)
            {
                printf("Waiting for command... ");
                string command(64, '\0');

                if (!ReadFile(pipe, &command[0], command.size(), NULL, NULL))
                {
                    auto readError = GetLastError();
                    if (readError != ERROR_IO_PENDING)
                    {
                        printf("ReadFile Error: %d\n", readError);
                        CloseHandle(pipe);
                        return -3;
                    }
                }

                if (command[0] != '\0')
                    printf("Received.\n");

                printf("Command: %s\n\n", command.c_str());

                istringstream parser{command};
                string keyword;
                parser >> keyword;


                if (keyword == "set"){
                    cout << "1";
                    if (func_set(pipe, data, parser)){
                        printf("func_set Error!\n");
                        CloseHandle(pipe);
                        return -2;
                    }
                }
                else if(keyword == "get"){
                    cout << "2";
                    if (func_get(pipe, data, parser)){
                        printf("func_get Error!\n");
                        CloseHandle(pipe);
                        return -2;
                    }
                }
                else if(keyword == "delete"){
                    cout << "3";
                    if (func_delete(pipe, data, parser)){
                        printf("func_delete Error!\n");
                        CloseHandle(pipe);
                        return -2;
                    }
                }
                else if(keyword == "list"){
                    cout << "4";
                    if (func_list(pipe, data)){
                        printf("func_list Error!\n");
                        CloseHandle(pipe);
                        return -2;
                    }
                }
                else if(keyword == "quit"){
                    cout << "4";
                    if (!DisconnectNamedPipe(pipe))
                        {
                            printf("DisconnectNamedPipe Error: %d\n", GetLastError());
                            CloseHandle(pipe);
                            return -4;
                        }
                        quit = true;
                }
                else
                {
                    string response = "No such command! Try again!\n";
                    cout << response;
                    if (!WriteFile(pipe, response.c_str(), response.size(), NULL, NULL))
                        {
                            auto Error = GetLastError();
                            if (Error != ERROR_IO_PENDING)
                            {
                                printf("WriteFile Error: %d", Error);
                                CloseHandle(pipe);
                                return -2;
                            }
                        }
                }
            }
        }
        else
        {
            CloseHandle(pipe);
            break;
        }
    }
}

int func_set(HANDLE pipe, map<string, string> &data, istringstream &parser)
{
    string name;
    string value;
    parser >> name >> value;

    data[name] = value;
    string response = "acknowledged\n";
    if (!WriteFile(pipe, response.c_str(), response.size(), NULL, NULL))
    {
        auto Error = GetLastError();
        if (Error != ERROR_IO_PENDING)
        {
            printf("WriteFile Error: %d", Error);
            return 1;
        }
    }
    return 0;

}

int func_get (HANDLE pipe, map<string, string> &data, istringstream &parser)
{
    string get_param;
    parser >> get_param;

    char response[256] = "\0";


    if (strcmp(get_param.c_str(), "all") == 0)
    {
        strcpy(response, "All data:\n");

        for (auto item : data)
        {
            strcat(response, item.first.c_str());
            strcat(response, " ");
            strcat(response, item.second.c_str());
            strcat(response, "\n");
        }

    }
    else if (data.find(get_param.c_str()) != data.end())
    {
        strcat(response, "found: ");
        strcat(response, data[get_param.c_str()].c_str());
        strcat(response, "\n");
    }
    else
    {
        strcat(response, "missing\n");
    }

    if (!WriteFile(pipe, response, strlen(response), NULL, NULL))
    {
        auto Error = GetLastError();
        if (Error != ERROR_IO_PENDING)
        {
            printf("WriteFile Error: %d", Error);
            return 1;
        }
    }
    return 0;
}


int func_list(HANDLE pipe, map<string, string> &data)
{
    string List = "";

    for (auto item : data)
    {
        List += item.first + ", ";
    }
    if (List.length() == 0)
        List = "List is empty\n";
    else
    {
        List[List.length() - 2] = '\n';
        List[List.length() - 1] = '\0';
    }
    if (!WriteFile(pipe, List.c_str(), List.size(), NULL, NULL))
    {
        auto Error = GetLastError();
        if (Error != ERROR_IO_PENDING)
        {
            printf("WriteFile Error: %d", Error);
            return 1;
        }
    }
    return 0;
}

int func_delete(HANDLE pipe, map<string, string> &data, istringstream &parser)
{
    string key;
    parser >> key;

    char response[256] = "\0";
    auto delKeyIt = data.find(key.c_str());

    if (delKeyIt == data.end())
        strcat(response, "missing\n");
    else
    {
        strcat(response, key.c_str());
        strcat(response, "-");
        strcat(response, data[key.c_str()].c_str());
        strcat(response, " deleted\n");
        data.erase(delKeyIt);
    }

    if (!WriteFile(pipe, response, strlen(response), NULL, NULL))
    {
        auto Error = GetLastError();
        if (Error != ERROR_IO_PENDING)
        {
            printf("WriteFile Error: %d", Error);
            return 1;
        }
    }
    return 0;
}
