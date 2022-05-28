#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include "dllinjector.h"

void start(LPCSTR lpApplicationName)
{
    // additional information
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    // set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // start the program up
    CreateProcessA
    (
        lpApplicationName,   // the path
        NULL,                // Command line
        NULL,                   // Process handle not inheritable
        NULL,                   // Thread handle not inheritable
        FALSE,                  // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,     // Opens file in a separate console
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi           // Pointer to PROCESS_INFORMATION structure
    );
        // Close process and thread handles. 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
}

int getProcID(const std::string p_name, Napi::Env env)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 structprocsnapshot = { 0 };

	structprocsnapshot.dwSize = sizeof(PROCESSENTRY32);

	if (snapshot == INVALID_HANDLE_VALUE)return 0;
	if (Process32First(snapshot, &structprocsnapshot) == FALSE)return 0;

	while (Process32Next(snapshot, &structprocsnapshot))
	{
		if (!strcmp(structprocsnapshot.szExeFile, p_name.c_str()))
		{
			CloseHandle(snapshot);
			return structprocsnapshot.th32ProcessID;
		}
	}
	CloseHandle(snapshot);
	Napi::TypeError::New(env, "Unable to find process id").ThrowAsJavaScriptException();
	return 0;

}

bool injectDLL(const std::string DLL_Path, const int pid, Napi::Env env)
{
	long dll_size = DLL_Path.length() + 1;
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	if (hProc == NULL)
	{
		Napi::TypeError::New(env, "Failed to open target process").ThrowAsJavaScriptException();
		return false;
	}

	LPVOID MyAlloc = VirtualAllocEx(hProc, NULL, dll_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (MyAlloc == NULL)
	{
		Napi::TypeError::New(env, "Failed to allocate memory in target process").ThrowAsJavaScriptException();
		return false;
	}

	int IsWriteOK = WriteProcessMemory(hProc , MyAlloc, DLL_Path.c_str() , dll_size, 0);
	if (IsWriteOK == 0)
	{
		Napi::TypeError::New(env, "Failed to write in target process memory").ThrowAsJavaScriptException();
		return false;
	}

	DWORD dWord;
	LPTHREAD_START_ROUTINE addrLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(LoadLibrary("kernel32"), "LoadLibraryA");
	HANDLE ThreadReturn = CreateRemoteThread(hProc, NULL, 0, addrLoadLibrary, MyAlloc, 0, &dWord);
	if (ThreadReturn == NULL)
	{
		Napi::TypeError::New(env, "Failed to create remote thread").ThrowAsJavaScriptException();
		return false;
	}

	if ((hProc != NULL) && (MyAlloc != NULL) && (IsWriteOK != ERROR_INVALID_HANDLE) && (ThreadReturn != NULL))
	{
		return true;
	}

	return false;
}


int dllInject::inject(std::string pathToDll, std::string processName, Napi::Env env)
{
	int procID = getProcID(processName, env);
	int result = injectDLL(pathToDll, procID, env);
	return 0;
}	
int dllInject::execute(std::string pathToExe)
{
	start((LPCSTR) pathToExe.c_str());
	return 0;
}
Napi::Number dllInject::injectWrapped(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	if (info.Length() < 2 || info[0].IsNumber() || info[1].IsNumber()) 
	{
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
    } 
	
	std::string path = info[0].As<Napi::String>();
	Napi::String procName = info[1].As<Napi::String>();
	
	std::replace(path.begin(), path.end(), '/', '\\');
	Napi::String newPath = Napi::String::New(env, path);
	
	int result = dllInject::inject(newPath, procName, env);
	return Napi::Number::New(env, 1);
}

Napi::Number dllInject::executeWrapped(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	if (info.Length() < 1 || info[0].IsNumber() || info[1].IsNumber()) 
	{
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
    } 
	
	Napi::String path = info[0].As<Napi::String>();
	int result = dllInject::execute(path);
	return Napi::Number::New(env, result);
}

Napi::Object dllInject::init(Napi::Env env, Napi::Object exports) 
{
	exports.Set("execute", Napi::Function::New(env, dllInject::executeWrapped));
    exports.Set("inject", Napi::Function::New(env, dllInject::injectWrapped));
    return exports;
}