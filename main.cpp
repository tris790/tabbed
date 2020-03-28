#include <windows.h>
#include <stdio.h>
#include <UIAutomation.h>
#include <iostream>
#include <comdef.h>

#define EDIT_CONTROL_TYPE_ID 50004

class FocusChangedEventHandler : public IUIAutomationFocusChangedEventHandler
{
private:
    LONG _refCount;

public:
    int _eventCount;

    //Constructor.
    FocusChangedEventHandler() : _refCount(1), _eventCount(0)
    {
    }

    //IUnknown methods.
    ULONG STDMETHODCALLTYPE AddRef()
    {
        ULONG ret = InterlockedIncrement(&_refCount);
        return ret;
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

    // IUIAutomationFocusChangedEventHandler methods.
    HRESULT STDMETHODCALLTYPE HandleFocusChangedEvent(IUIAutomationElement *pSender)
    {
        _eventCount++;
        CONTROLTYPEID ctrlType;
        pSender->get_CurrentControlType(&ctrlType);

        RECT boundingBox;
        pSender->get_CurrentBoundingRectangle(&boundingBox);

        BOOL isPassword;
        pSender->get_CurrentIsPassword(&isPassword);

        if (ctrlType == EDIT_CONTROL_TYPE_ID && !isPassword)
        {

            // object patternObj;
            // if (element.TryGetCurrentPattern(ValuePattern.Pattern, out patternObj))
            // {
            //     var valuePattern = (ValuePattern)patternObj;
            //     return valuePattern.Current.Value;
            // }
            // else if (element.TryGetCurrentPattern(TextPattern.Pattern, out patternObj))
            // {
            //     var textPattern = (TextPattern)patternObj;
            //     return textPattern.DocumentRange.GetText(-1).TrimEnd('\r'); // often there is an extra '\r' hanging off the end.
            // }
            // else
            // {
            //     return element.Current.Name;
            // }
            void *patternObj;
            PATTERNID valuePatternId = 1;
            PATTERNID textPatternId = 2;
            printf("UIA_ValuePatternId\n");
            if (pSender->GetCurrentPatternAs(UIA_ValuePatternId, IID_IUIAutomationValuePattern, &patternObj))
            {
                printf("value pattern\n");
                BSTR bs;
                static_cast<IUIAutomationValuePattern *>(patternObj)->get_CurrentValue(&bs);
                wchar_t *str = _bstr_t(bs, false);
                wprintf(L">> You are are focused in a textbox (%s) [%d, %d]\n", str, boundingBox.left, boundingBox.top);
            }
            printf("UIA_TextPatternId\n");
            if (pSender->GetCurrentPatternAs(UIA_TextPatternId, IID_IUIAutomationTextPattern, &patternObj))
            {
                printf("text pattern\n");
                BSTR bs;
                IUIAutomationTextRange *range;
                static_cast<IUIAutomationTextPattern *>(patternObj)->get_DocumentRange(&range);
                range->GetText(-1, &bs);
                wchar_t *str = _bstr_t(bs, false);
                wprintf(L">> You are are focused in a textbox (%s) [%d, %d]\n", str, boundingBox.left, boundingBox.top);
            }

            return S_OK;
        }
    }
};

HRESULT AddFocusHandler(IUIAutomation *pAutomation)
{
    // CFocusHandler is a class that implements IUIAutomationFocusChangedEventHandler.
    FocusChangedEventHandler *pFocusHandler = new FocusChangedEventHandler();
    if (!pFocusHandler)
    {
        return E_OUTOFMEMORY;
    }
    IUIAutomationFocusChangedEventHandler *pHandler;
    pFocusHandler->QueryInterface(IID_IUIAutomationFocusChangedEventHandler, (void **)&pHandler);
    HRESULT hr = pAutomation->AddFocusChangedEventHandler(NULL, pHandler);
    pFocusHandler->Release();
    return hr;
}

int main()
{
    HRESULT hr;
    int ret = 0;
    FocusChangedEventHandler *pEHTemp = NULL;

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    IUIAutomation *pAutomation = NULL;
    hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), (void **)&pAutomation);
    if (FAILED(hr) || pAutomation == NULL)
    {
        ret = 1;
        goto cleanup;
    }

    pEHTemp = new FocusChangedEventHandler();
    if (pEHTemp == NULL)
    {
        ret = 1;
        goto cleanup;
    }

    wprintf(L"-Adding Event Handlers.\n");
    hr = pAutomation->AddFocusChangedEventHandler(NULL, (IUIAutomationFocusChangedEventHandler *)pEHTemp);
    if (FAILED(hr))
    {
        ret = 1;
        goto cleanup;
    }

    wprintf(L"-Press any key to remove event handler and exit\n");
    getchar();

    wprintf(L"-Removing Event Handlers.\n");
    hr = pAutomation->RemoveFocusChangedEventHandler((IUIAutomationFocusChangedEventHandler *)pEHTemp);
    if (FAILED(hr))
    {
        ret = 1;
        goto cleanup;
    }

    // Release resources and terminate.
cleanup:
    if (pEHTemp != NULL)
        pEHTemp->Release();

    if (pAutomation != NULL)
        pAutomation->Release();

    CoUninitialize();
    return ret;
}