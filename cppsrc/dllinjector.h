#include <napi.h>
namespace dllInject {
  int inject(std::string pathToDll, std::string processName, Napi::Env env);
  int execute(std::string pathToExe);
  Napi::Number injectWrapped(const Napi::CallbackInfo& info);
  Napi::Number executeWrapped(const Napi::CallbackInfo& info);
  Napi::Object init(Napi::Env env, Napi::Object exports);
}