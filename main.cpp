#include <windows.h>
#include <stdio.h>
#include <UIAutomation.h>
#include <iostream>
#include <comdef.h>

class FocusChangedEventHandler : public IUIAutomationFocusChangedEventHandler
{
private:
    LONG _refCount;

public:
    FocusChangedEventHandler() : _refCount(1) {}

    ULONG STDMETHODCALLTYPE AddRef()
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

    HRESULT STDMETHODCALLTYPE HandleFocusChangedEvent(IUIAutomationElement *pSender)
    {
        CONTROLTYPEID ctrlType;
        pSender->get_CurrentControlType(&ctrlType);

        RECT boundingBox;
        pSender->get_CurrentBoundingRectangle(&boundingBox);

        BOOL isPassword;
        pSender->get_CurrentIsPassword(&isPassword);

        if (ctrlType == UIA_EditControlTypeId && !isPassword)
        {
            void *patternObj;
            printf("Found textbox\n");
            if (!pSender->GetCurrentPatternAs(UIA_ValuePatternId, IID_IUIAutomationValuePattern, &patternObj))
            {
                printf("value pattern\n");
                BSTR bs;
                static_cast<IUIAutomationValuePattern *>(patternObj)->get_CurrentValue(&bs);
                wchar_t *str = _bstr_t(bs, false);
                printf(">> You are focused in a textbox (%S) [%d, %d]\n", str, boundingBox.left, boundingBox.top);
            }
            else if (!pSender->GetCurrentPatternAs(UIA_TextPatternId, IID_IUIAutomationTextPattern, &patternObj))
            {
                printf("text pattern\n");
                BSTR bs;
                IUIAutomationTextRange *range;
                static_cast<IUIAutomationTextPattern *>(patternObj)->get_DocumentRange(&range);
                range->GetText(-1, &bs);
                wchar_t *str = _bstr_t(bs, false);
                printf(">> You are focused in a textbox (%S) [%d, %d]\n", str, boundingBox.left, boundingBox.top);
            }
        }
        return S_OK;
    }
};

HRESULT AddFocusHandler(IUIAutomation *uiAutomationPtr)
{
    FocusChangedEventHandler *pFocusHandler = new FocusChangedEventHandler();
    if (!pFocusHandler)
    {
        return E_OUTOFMEMORY;
    }
    IUIAutomationFocusChangedEventHandler *pHandler;
    pFocusHandler->QueryInterface(IID_IUIAutomationFocusChangedEventHandler, (void **)&pHandler);
    HRESULT hr = uiAutomationPtr->AddFocusChangedEventHandler(NULL, pHandler);
    pFocusHandler->Release();
    return hr;
}

HRESULT RegisterEventHandlers(IUIAutomation *uiAutomationPtr, FocusChangedEventHandler *focusChangedEventHandlerPtr)
{
    printf("Adding Event Handlers.\n");
    return uiAutomationPtr->AddFocusChangedEventHandler(NULL, (IUIAutomationFocusChangedEventHandler *)focusChangedEventHandlerPtr);
}

HRESULT UnRegisterEventHandlers(IUIAutomation *uiAutomationPtr, FocusChangedEventHandler *focusChangedEventHandlerPtr)
{
    printf("Removing Event Handlers.\n");
    return uiAutomationPtr->RemoveFocusChangedEventHandler((IUIAutomationFocusChangedEventHandler *)focusChangedEventHandlerPtr);
}

void Cleanup(IUIAutomation *uiAutomationPtr, FocusChangedEventHandler *focusChangedEventHandlerPtr)
{
    if (focusChangedEventHandlerPtr != NULL)
        focusChangedEventHandlerPtr->Release();

    if (uiAutomationPtr != NULL)
        uiAutomationPtr->Release();

    CoUninitialize();
}

int main()
{
    HRESULT hr;
    FocusChangedEventHandler *focusChangedEventHandlerPtr = NULL;

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    IUIAutomation *uiAutomationPtr = NULL;
    hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), (void **)&uiAutomationPtr);
    if (FAILED(hr) || uiAutomationPtr == NULL)
    {
        Cleanup(uiAutomationPtr, focusChangedEventHandlerPtr);
        return 1;
    }

    focusChangedEventHandlerPtr = new FocusChangedEventHandler();
    if (focusChangedEventHandlerPtr == NULL)
    {
        Cleanup(uiAutomationPtr, focusChangedEventHandlerPtr);
        return 1;
    }

    if (FAILED(RegisterEventHandlers(uiAutomationPtr, focusChangedEventHandlerPtr)))
    {
        Cleanup(uiAutomationPtr, focusChangedEventHandlerPtr);
        return 1;
    }

    printf("Press any key to remove event handler and exit\n");
    getchar();

    UnRegisterEventHandlers(uiAutomationPtr, focusChangedEventHandlerPtr);

    Cleanup(uiAutomationPtr, focusChangedEventHandlerPtr);
}