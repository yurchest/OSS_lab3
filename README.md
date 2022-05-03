# Лабораторная работа №3 Межпроцессное взаимодействие
Вариант 2  
Выполнил: Черемных Юрий А-01-19

**Цель**: Написать пару программ, взаимодействующих через именованные каналы.
Программа-сервер хранит строки-значения по строкам-ключам. Программа-клиент,
отправляя программе-серверу команды, добавляет, удаляет и получает значения.
## Выполнение работы
# Сервер
Сервер создает именованый канал, ждет подключения клиента, обрабатывает его запросы и высылает ответную информацию

### 1. Создание именованного канала

```cpp
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
```

### 2. Цикл работы сервера
```cpp
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
                    if (func_set(pipe, data, parser)){
                        printf("func_set Error!\n");
                        CloseHandle(pipe);
                        return -2;
                    }
                }
                else if(keyword == "get"){
                    if (func_get(pipe, data, parser)){
                        printf("func_get Error!\n");
                        CloseHandle(pipe);
                        return -2;
                    }
                }
                else if(keyword == "delete"){
                    if (func_delete(pipe, data, parser)){
                        printf("func_delete Error!\n");
                        CloseHandle(pipe);
                        return -2;
                    }
                }
                else if(keyword == "list"){
                    if (func_list(pipe, data)){
                        printf("func_list Error!\n");
                        CloseHandle(pipe);
                        return -2;
                    }
                }
                else if(keyword == "quit"){
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
```

--- 

### Дополнительные функции

### 1. **func_set**  
Добавляет в словарь ```map<string, string> data``` указанный клиентом ключ ``` name ``` и его значение ``` value ```  
В ответ отпрвляет строку "acknowledged"

```cpp  
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

```
### 2. **func_get**  
Возвращает клиенту значение ключа или содержимое всего словаря если была получена команда ``` get all ```

```cpp  
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
```

### 3. **func_list**  
Возвращает клиенту список ключей. Если словарь пуст, то возвращет "List is empty"
```cpp  
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
```

### 3. **func_delete**  
Удаляет запись с указанном ключом.
```cpp  
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

```

# Клиент
При написании клинта использовал [пример с сайта Microsoft](https://docs.microsoft.com/ru-ru/windows/win32/ipc/named-pipe-client?redirectedfrom=MSDN)
### 1. Подключение к именованному каналу
```cpp  
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
```

### 2. Основной цикл работы клиента
```cpp 
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
```
---
# Скриншоты работы программы
## Клиент  
![image](https://user-images.githubusercontent.com/61864601/166513167-d4899d03-886f-4013-ba5f-bbf053a91728.png)
![image](https://user-images.githubusercontent.com/61864601/166513269-1de8cc83-0f01-43b4-bbe8-0ba48247ba72.png)

## Сервер  
![image](https://user-images.githubusercontent.com/61864601/166513326-24f13416-11d7-4685-8371-34c621bd5ae7.png)
![image](https://user-images.githubusercontent.com/61864601/166513378-9064ea69-eed1-493d-b117-e6a326731806.png)





