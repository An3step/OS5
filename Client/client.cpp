#include <windows.h>
#include <iostream>
#include <string>
using namespace std;

struct employee
{
    int num;
    char name[10];
    double hours;
};

struct query
{
    char modifier;
    int id;
};

employee Input_employee() {
    employee result;
    while (true) {
        cout << "Employee num: ";
        cin >> result.num;
        if (cin.fail() || result.num < 0) {
            cin.clear();
            cin.ignore(10000000000, '\n');
            cout << "Invalid input. Please enter a positive integer.\n";
        } else {
            cin.ignore(10000000000, '\n');
            break;
        }
    }
    while (true) {
        cout << "Employee name: ";
        cin >> result.name;
        if (strlen(result.name) >= 10) {
            cout << "Name too long. Please enter a name with fewer than 10 characters.\n";
        } else {
            break;
        }
    }
    while (true) {
        cout << "Employee hours: ";
        cin >> result.hours;
        if (cin.fail() || result.hours < 0) {
            cin.clear();
            cin.ignore(1000000, '\n');
            cout << "Invalid input. Please enter a non-negative number for hours.\n";
        } else {
            cin.ignore(1000000, '\n');
            break;
        }
    }

    return result;
}

void Output_employee(employee& Employee)
{
    cout << "Employee num: ";
    cout << Employee.num;
    cout << "\nEmployee name: ";
    cout << Employee.name;
    cout << "\nEmployee hours: ";
    cout << Employee.hours;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Error: Process number not provided\n";
        system("pause");
        return -1;
    }

    int processNum = atoi(argv[1]);
    // Open event for synchronization
    wstring eventName = L"Global\\ControlEvent" + to_wstring(processNum);
    HANDLE hEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, eventName.c_str());
    if (hEvent == NULL) {
        cerr << "OpenEvent failed: " << GetLastError() << endl;
        system("pause");
        return 1;
    }
    cout << "NUM = " << processNum << '\n';
    // Connect to pipes
    wstring pipeNameM = L"\\\\.\\pipe\\MyPipeM" + to_wstring(processNum);
    wstring pipeNameE = L"\\\\.\\pipe\\MyPipeE" + to_wstring(processNum);

    HANDLE hPipeM = CreateFileW(
        pipeNameM.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    HANDLE hPipeE = CreateFileW(
        pipeNameE.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hPipeM == INVALID_HANDLE_VALUE || hPipeE == INVALID_HANDLE_VALUE) {
        cerr << "Failed to connect to pipes: " << GetLastError() << endl;
        system("pause");
        return 1;
    }

    cout << "Connected to server pipes successfully!\n";

while (true)
{
    cout << "\nChoose operation:\n"
         << "1. Read employee record\n"
         << "2. Modify employee record\n"
         << "3. Exit\n"
         << "Your choice: ";
    
    int choice;
    cin >> choice;

    if (choice == 3)
    {
            query request;
            request.modifier = 'E';
            request.id = -1;

            // Очистка буфера перед отправкой
            FlushFileBuffers(hPipeM);
            
            // Отправка запроса
            DWORD bytesWritten;
            if (!WriteFile(hPipeM, &request, sizeof(query), &bytesWritten, NULL)) {
                cerr << "Write failed: " << GetLastError() << endl;
                break;
            }
            SetEvent(hEvent);
            break;
    }

    cout << "Enter employee ID: ";
    int id;
    cin >> id;
    
    // Подготовка запроса
    query request = {0};
    request.modifier = (choice == 1) ? 'R' : 'M';
    request.id = id;
    DWORD bytesWritten;
    if (!WriteFile(hPipeM, &request, sizeof(query), &bytesWritten, NULL))
    {
        cerr << "Write failed: " << GetLastError() << endl;
        break;
    }
    employee response = {0};
    DWORD bytesRead;
    SetEvent(hEvent);
    WaitForSingleObject(hEvent, INFINITE);
    ResetEvent(hEvent);
    cout << "Got employee from server\n";
    if (!ReadFile(hPipeE, &response, sizeof(employee), &bytesRead, NULL))
    {
        cerr << "Read failed: " << GetLastError() << endl;
        break;
    }
    if (response.num == -1)
    {
        cout << "Employee not found or modifying right now!" << endl;
        // string q;
        // cout << "Enter string to stop rule over record: ";
        // cin >> q;
        // SetEvent(hEvent);
        continue;
    }
    else
    {
        FlushFileBuffers(hPipeE);
        Output_employee(response);
    }
    if(choice == 1)
    {
            string q;
            cout << "Enter string to stop rule over record: ";
            cin >> q;
            SetEvent(hEvent);
    }
    else if(choice == 2)
    {
        cout << '\n';
        response = Input_employee();
        cout << "Started sending\n";
        DWORD bytesWritten2;
        if (!WriteFile(hPipeE, &response, sizeof(employee), &bytesWritten2, NULL))
        {
            cerr << "Write to pipe failed: " << GetLastError() << endl;
            break;
        }
        else
        {
            cout << "Changed succesfully!\n";
        }
        SetEvent(hEvent);
        WaitForSingleObject(hEvent, INFINITE);
        ResetEvent(hEvent);
        string q;
        cout << "Enter string to stop rule over record: ";
        cin >> q;
        SetEvent(hEvent);
    }
}

    CloseHandle(hPipeM);
    CloseHandle(hPipeE);
    CloseHandle(hEvent);
    system("pause");
    return 0;
}