#include <iostream>
#include "windows.h"
#include <string>
#include <fstream>
#include <limits>
using std::string, std::cout, std::cin, std::ifstream, std::ofstream, std::ios;

string filename;
int start_employeers;
int processes;
HANDLE* modifying_and_reading_mutex;

HANDLE hMutex;

struct employee
{
    int num;
    char name[10];
    double hours;
};

employee NULL_employee;

struct query
{
    char modifier;
    int id;
};

employee Input_employee() {
    employee result;

    // Input employee number with validation
    while (true) {
        cout << "Employee num: ";
        cin >> result.num;
        if (cin.fail() || result.num < 0) {
            cin.clear(); // Clear the error flag
            cin.ignore(10000000000, '\n'); // Discard invalid input
            cout << "Invalid input. Please enter a positive integer.\n";
        } else {
            cin.ignore(10000000000, '\n'); // Clear the input buffer
            break;
        }
    }

    // Input employee name with validation
    while (true) {
        cout << "Employee name: ";
        cin >> result.name;
        if (strlen(result.name) >= 10) {
            cout << "Name too long. Please enter a name with fewer than 10 characters.\n";
        } else {
            break;
        }
    }

    // Input employee hours with validation
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

void CreateBinaryFile(const std::string& filename)
{
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile)
    {
        std::cerr << "Failed to create binary file!" << std::endl;
        return;
    }
    outFile.close();
}

employee readData(int num) {
    ifstream start_read(filename, ios::binary);
    if (!start_read) {
        std::cerr << "Failed to open file for reading\n";
        NULL_employee.num = -1;
        return NULL_employee;
    }

    employee record;
    bool found = false;
    
    // Read through all records
    while (start_read.read(reinterpret_cast<char*>(&record), sizeof(record))) {
        if (record.num == num) {
            found = true;
            break;
        }
    }

    start_read.close();
    
    if (!found) {
        NULL_employee.num = -1;
        return NULL_employee;
    }
    
    return record;
}

void readAllData()
{
    ifstream start_read(filename, ios::binary);
    employee record;
    for(int i = 0; i < start_employeers; i++)
    {
        start_read.read(reinterpret_cast<char*>(&record), sizeof(record));
        cout << "Employee " << i + 1 << ":\n";
        Output_employee(record);
        cout << "\n";
    }
    start_read.close();
}

HANDLE* GLobal_Events;

void modifyData(const employee& new_Employee, int num)
{
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file." << std::endl;
        return;
    }

    std::streamsize recordSize = sizeof(employee);
    employee record;
    while (file.read(reinterpret_cast<char*>(&record), recordSize)) {
        if (record.num == num) {
            // Переход на начало записи
            file.seekp(-recordSize, std::ios::cur);

            // Перезапись записи
            file.write(reinterpret_cast<const char*>(&new_Employee), recordSize);
            if (!file) {
                std::cerr << "Error writing to file." << std::endl;
            }
            file.close();
            return;
        }
    }

    std::cerr << "Employee with number " << num << " not found." << std::endl;
    file.close();
}


void createSenderProcess(int number) {
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::wstring commandLine = L"Client/client.exe " + std::to_wstring(number);

    if (CreateProcessW(
            NULL,
            &commandLine[0],
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_CONSOLE,
            NULL,
            NULL,
            &si,
            &pi)) {
        std::wcout << L"Started sender.exe with PID: " << pi.dwProcessId << "\n";
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        std::wcerr << L"Failed to create sender.exe. Error: " << GetLastError() << std::endl;
    }
}

void CreatePipes(HANDLE& hPipe_Modifiers, HANDLE& hPipe_Employeers, int* number)
{
    std::wstring pipe_name_modifier = L"\\\\.\\pipe\\MyPipeM" + std::to_wstring(*number);
    std::wstring pipe_name_employee = L"\\\\.\\pipe\\MyPipeE" + std::to_wstring(*number);
    
        hPipe_Modifiers = CreateNamedPipeW(
        pipe_name_modifier.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        processes,
        512,
        512,
        0,
        NULL);
        
        hPipe_Employeers = CreateNamedPipeW(
        pipe_name_employee.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        processes,
        512,
        512,
        0,
        NULL);
}

HANDLE Semaphore;

DWORD WINAPI InstanceThread(LPVOID lpvParam)
{
    int* number = (int*)lpvParam;
    cout << *number << '\n';
    std::wstring eventName = L"Global\\ControlEvent" + std::to_wstring(*number);
    GLobal_Events[*number] = CreateEventW(NULL, TRUE, FALSE, eventName.c_str());
    if (GLobal_Events[*number] == NULL)
    {
        std::cerr << "CreateEvent failed: " << GetLastError() << '\n';
        return 1;
    }
    
    HANDLE hPipe_Modifiers, hPipe_Employeers;
    CreatePipes(hPipe_Modifiers, hPipe_Employeers, number);

    if (hPipe_Modifiers == INVALID_HANDLE_VALUE || hPipe_Employeers == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateNamedPipe failed: " << GetLastError() << '\n';
        return -1;
    }
    
    createSenderProcess(*number);

    cout << "Waiting for client connection...\n";
    BOOL connectedM = ConnectNamedPipe(hPipe_Modifiers, NULL);
    BOOL connectedE = ConnectNamedPipe(hPipe_Employeers, NULL);
    
    if (!connectedM && GetLastError() != ERROR_PIPE_CONNECTED) {
        std::cerr << "ConnectNamedPipe M failed: " << GetLastError() << '\n';
        CloseHandle(hPipe_Modifiers);
        CloseHandle(hPipe_Employeers);
        return 1;
    }
    
    if (!connectedE && GetLastError() != ERROR_PIPE_CONNECTED) {
        std::cerr << "ConnectNamedPipe E failed: " << GetLastError() << '\n';
        CloseHandle(hPipe_Modifiers);
        CloseHandle(hPipe_Employeers);
        return 1;
    }
    
    cout << "Both pipes connected successfully!\n";
    FlushFileBuffers(hPipe_Modifiers);
    FlushFileBuffers(hPipe_Employeers);

    while (true)
    {
        DWORD waitResult = WaitForSingleObject(GLobal_Events[*number], INFINITE);
        if (waitResult != WAIT_OBJECT_0)
        {
            std::cerr << "Wait failed: " << GetLastError() << '\n';
            break;
        }
        query receivedData = {0};
        DWORD bytesRead;
        if (!ReadFile(hPipe_Modifiers, &receivedData, sizeof(query), &bytesRead, NULL)) {
            DWORD err = GetLastError();
            if (err == ERROR_BROKEN_PIPE) {
                cout << "Client disconnected." << '\n';
                break;
            }
            std::cerr << "Read from pipe failed: " << err << '\n';
            continue;
        }
        if(receivedData.modifier == 'E')
        {
            break;
        }
        if(receivedData.modifier == 'R')
        {
            waitResult = WaitForSingleObject(Semaphore, 0);
            if(waitResult != WAIT_OBJECT_0)
            {
                employee emp;
                emp.num = -1;
                DWORD bytesWritten;
                if (!WriteFile(hPipe_Employeers, &emp, sizeof(employee), &bytesWritten, NULL))
                {
                    std::cerr << "Write to pipe failed: " << GetLastError() << '\n';
                    break;
                }
                SetEvent(GLobal_Events[*number]);
                continue;
            }
            employee emp = {0};
            {
                emp = readData(receivedData.id);
            }
            DWORD bytesWritten;
            if (!WriteFile(hPipe_Employeers, &emp, sizeof(employee), &bytesWritten, NULL)) {
                std::cerr << "Write to pipe failed: " << GetLastError() << '\n';
                break;
            }
            cout << "written to client\n";
            cout << "Waiting for semaphore releasing\n";
            if(emp.num != -1)
            {
                WaitForSingleObject(GLobal_Events[*number], INFINITE);
            }
            ReleaseSemaphore(Semaphore, 1, NULL);
            cout << "Semaphore released\n";

        }
        if(receivedData.modifier == 'M')
        {
            bool isfor = true;
            for(int i = 0; i < processes; i++)
            {
                waitResult = WaitForSingleObject(Semaphore, 0);
                if(waitResult != WAIT_OBJECT_0)
                {
                    employee emp;
                    emp.num = -1;
                    DWORD bytesWritten;
                    if (!WriteFile(hPipe_Employeers, &emp, sizeof(employee), &bytesWritten, NULL))
                    {
                        std::cerr << "Write to pipe failed: " << GetLastError() << '\n';
                        break;
                    }
                    SetEvent(GLobal_Events[*number]);
                    if(i > 0)
                    {
                        ReleaseSemaphore(Semaphore, i, NULL);
                    }
                    isfor = false;
                    break;
                }
            }
            if(!isfor)
            {
                continue;
            }
            employee emp = {0};
            {
                emp = readData(receivedData.id);
            }
            DWORD bytesWritten;
            if (!WriteFile(hPipe_Employeers, &emp, sizeof(employee), &bytesWritten, NULL))
            {
                std::cerr << "Write to pipe failed: " << GetLastError() << '\n';
                break;
            }
            SetEvent(GLobal_Events[*number]);
            ResetEvent(GLobal_Events[*number]);
            if(emp.num == -1)
            {
                ReleaseSemaphore(Semaphore, processes, NULL);
                continue;
            }
            WaitForSingleObject(GLobal_Events[*number], INFINITE);
            employee receivedEmp;
            DWORD bytesRead;
            BOOL readResult = ReadFile(
                hPipe_Employeers,
                &receivedEmp,
                sizeof(employee),
                &bytesRead,
                NULL
            );
            cout << '\n';
            // Output_employee(receivedEmp);
            cout << '\n';
            modifyData(receivedEmp, receivedData.id);
            cout << "waiting for stop semaphore\n";
            WaitForSingleObject(GLobal_Events[*number], INFINITE);
            cout << "releasing semaphore\n";
            ReleaseSemaphore(Semaphore, processes, NULL);
        }
    }
    DisconnectNamedPipe(hPipe_Modifiers);
    DisconnectNamedPipe(hPipe_Employeers);
    CloseHandle(hPipe_Modifiers);
    CloseHandle(hPipe_Employeers);
    return 0;
}

int main()
{
    modifying_and_reading_mutex = new HANDLE[2];
    modifying_and_reading_mutex[0] = CreateMutex(NULL, FALSE, "Global\\MyGlobalMutexM");
    modifying_and_reading_mutex[1] = CreateMutex(NULL, FALSE, "Global\\MyGlobalMutexR");

    cout << "Input Process count: ";
    while(true)
    {
        cin >> processes;
        if (cin.fail() || processes < 1) {
            cin.clear();
            cin.ignore(10000000000, '\n');
            cout << "Invalid input. Please enter a positive integer.\n";
        }
        else
        {
            cin.ignore(10000000000, '\n');
            break;
        }
    }
    Semaphore = CreateSemaphore(NULL, processes, processes, NULL);
    cout << "Filename: ";
    cin >> filename;
    filename += ".bin";
    cout << "Start number of employeers: ";
    while(true)
    {
        cin >> start_employeers;
        if (cin.fail() || start_employeers < 1) {
            cin.clear();
            cin.ignore(10000000000, '\n');
            cout << "Invalid input. Please enter a positive integer.\n";
        } else {
            cin.ignore(10000000000, '\n');
            break;
        }
    }

    CreateBinaryFile(filename);
    ofstream start_output(filename, ios::binary);
    hMutex = CreateMutex(NULL, FALSE, NULL);
    employee record;
    for(int i = 0; i < start_employeers; i++)
    {
        cout << "Getting info of employee number " << i + 1 << '\n';
        record = Input_employee();
        start_output.write(reinterpret_cast<char*>(&record), sizeof(record));
    }
    start_output.close();
    ifstream start_read(filename, ios::binary);
    for(int i = 0; i < start_employeers; i++)
    {
        start_read.read(reinterpret_cast<char*>(&record), sizeof(record));
        cout << "Employee " << i + 1 << ":\n";
        Output_employee(record);
        cout << "\n";
    }
    start_read.close();
    GLobal_Events = new HANDLE[processes];
    HANDLE* hThreads = new HANDLE[processes];
    for(int i = 0; i < processes; i++)
    {
        DWORD IDThread;
        hThreads[i] = CreateThread(NULL, 0, InstanceThread, LPVOID(new int(i)), 0, &IDThread);
    }
    WaitForMultipleObjects(processes, hThreads, TRUE, INFINITE);
}
