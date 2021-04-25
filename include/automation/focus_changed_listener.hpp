#pragma once
#include <UIAutomation.h>
#include <functional>

class FocusChangedListener : public IUIAutomationFocusChangedEventHandler
{
private:
    long _refCount;
    std::function<void(IUIAutomationElement*)> callback;

public:
    FocusChangedListener(std::function<void(IUIAutomationElement*)> callback) 
        : _refCount{1},
          callback{callback} {}

    unsigned long AddRef()
    {
        return InterlockedIncrement(&_refCount);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ret = InterlockedDecrement(&_refCount);
        if (ret == 0)
        {
            delete this;
            return 0;
        }
        return ret;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppInterface)
    {
        if (riid == __uuidof(IUnknown))
            *ppInterface = static_cast<IUIAutomationFocusChangedEventHandler *>(this);
        else if (riid == __uuidof(IUIAutomationFocusChangedEventHandler))
            *ppInterface = static_cast<IUIAutomationFocusChangedEventHandler *>(this);
        else
        {
            *ppInterface = NULL;
            return E_NOINTERFACE;
        }
        this->AddRef();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE HandleFocusChangedEvent(IUIAutomationElement *sender)
    {
        callback(sender);
        // CONTROLTYPEID ctrlType;
        // sender->get_CurrentControlType(&ctrlType);

        // BOOL isPassword;
        // sender->get_CurrentIsPassword(&isPassword);
        // int curPid = 0;
        // sender->get_CurrentProcessId(&curPid);
        // if (ctrlType == UIA_EditControlTypeId && !isPassword)
        // {
        // }
        return S_OK;
    }
};
