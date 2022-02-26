#include <Windows.h>
#include "IModule.h"
#include "IMLoggerService.h"
#include "IMRenderQueueService.h"

typedef MWD::Module::IModule* (*FUNC)(const MString& name);
typedef void (*FUNC2)(const MString& name);

using namespace MWD::Module;
using namespace MWD::Render;
using namespace MWD::Service;
using namespace MWD::Service::Log;

int main() {
    HMODULE h = LoadLibraryA("F:\\c++projects\\mwdui\\lib\\im_module.dll");
    if (h) {
        SIID SIID_IM_SERVICE_LOGGER{
                1,
                1
        };

        //GMRegisterServiceFunc f = (GMRegisterServiceFunc)GetProcAddress(h, "GMRegisterService");
        //MWD::Service::IMService* s = f(SIID_IM_SERVICE_LOGGER, nullptr);
        //if (s->QueryInterface(SIID_IM_SERVICE_LOGGER)) {
        //    MLoggerService* c = dynamic_cast<MLoggerService*>(s);
        //
        //    c->Init();
        //    c->Log(IMLogLevel::DEBUG, IM_SIMPLE_LOGGER_EVENT("test", "testMain"));
        //
        //}
        //
        //GMUnregisterServiceFunc f2 = (GMUnregisterServiceFunc)GetProcAddress(h, "GMUnregisterService");
        //f2(SIID_IM_SERVICE_LOGGER);
    }


}

