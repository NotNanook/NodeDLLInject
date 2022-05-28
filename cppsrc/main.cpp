#include <napi.h>
#include "dllinjector.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  return dllInject::init(env, exports);
}

NODE_API_MODULE(dllinjector, InitAll)