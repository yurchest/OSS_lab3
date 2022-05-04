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
```

### 2. Цикл работы сервера
```cpp
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
```

--- 

### Дополнительные функции

### 1. wait_for_connection_client
Подключение к каналу и обработка ошибок
```cpp
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
```


# Клиент
При написании клинта использовал [пример с сайта Microsoft](https://docs.microsoft.com/ru-ru/windows/win32/ipc/named-pipe-client?redirectedfrom=MSDN)
### 1. Подключение к именованному каналу
```cpp  
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
```

### 2. Основной цикл работы клиента
```cpp 
while(1){
      printf(">> ");
      string command;
      string response(1024, '\0');
      
      getline(cin, command);

      WriteFile(hPipe, command.c_str(), command.size(),NULL, NULL);


      ReadFile(hPipe, &response[0], response.size(), NULL, NULL);
      printf("%s \n\n", response.c_str());


   }
```
---
# Скриншоты работы программы
## Клиент  
![image](https://user-images.githubusercontent.com/61864601/166830644-6bb54274-7fbe-4342-b744-291145f6aefb.png)


## Сервер  
![image](https://user-images.githubusercontent.com/61864601/166830672-dc9b531d-3888-4ad9-a4d8-f3ea988edb3d.png)






