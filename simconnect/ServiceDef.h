#pragma once
#ifndef SERVICEDEF
#define SERVICEDEF

#include "../common_sys.h"
#include <SimConnect.h>

class ServiceDef {
private:
    HANDLE hSimConnect = 0;

    static void executeEventRequest(ENUM eventID, DWORD data) {
        lSimVarsValue[eventID - THIRD_PARTY_EVENT_ID_MIN] = data;
    }
    static void CALLBACK DispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext) {
        switch (pData->dwID) {
            case SIMCONNECT_RECV_ID_EVENT: {
                SIMCONNECT_RECV_EVENT* evt = (SIMCONNECT_RECV_EVENT*)pData;
                executeEventRequest((ENUM)evt->uEventID, evt->dwData);
            }
            /*
            case SIMCONNECT_RECV_ID_SYSTEM_STATE: {
                //add kill here? but stop trigger is sent while navigating/console so probably not a good idea?
            }*/
            default: {
                break;
            }
        }
    }
public:
    bool handleSimConnect(int service_id) {
        switch (service_id) {
            case PANEL_SERVICE_PRE_INSTALL: {
                HRESULT hr = SimConnect_Open(&hSimConnect, "A32NX_wasm_sys", nullptr, 0, 0, 0);
                if (SUCCEEDED(hr)) {
                    return true;
                }

            }
            default:
                break;
        }
        return false;
    }

    bool registerToEvents() {
        //this listens to the event, masks the event after receiving it
        //a single notification group can at max hold 1000events
        //all third party applications should refer to events by adding THIRD_PARTY_EVENT_ID_MIN to the enum definitions provided under data.h for lVars
        for (int i = BATT1_ONLINE; i < totalLVarsCount; i++) {
            HRESULT add_client = SimConnect_AddClientEventToNotificationGroup(hSimConnect, SDK_CONTROL, THIRD_PARTY_EVENT_ID_MIN + i, 1);
            if (FAILED(add_client)) {
                return false;
            }
        }
        HRESULT group_priority = SimConnect_SetNotificationGroupPriority(hSimConnect, SDK_CONTROL, SIMCONNECT_GROUP_PRIORITY_HIGHEST);
        if(SUCCEEDED(group_priority)){
            return true;
        }
        return false;
    }

    void handleSimDispatch() {
        SimConnect_CallDispatch(hSimConnect, DispatchProc, nullptr);
    }

    bool simStopCheck(int service_id) {
        switch (service_id)
        {
            case PANEL_SERVICE_PRE_KILL: {
                return true;
            }
            default:
                return false;
        }
    }

    bool handleSimDisconnect() {
        HRESULT hr = SimConnect_Close(hSimConnect);
        if (SUCCEEDED(hr)) {
            return true;
        }
        return false;
    }
};
#endif