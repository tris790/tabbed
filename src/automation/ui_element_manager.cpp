#include <automation/ui_element_manager.hpp>
#include <UIAutomation.h>
#include <comutil.h>

void UIElementManager::setCurrentUIElement(IUIAutomationElement *newUIElement)
{
    this->currentUIElement = newUIElement;

    CONTROLTYPEID ctrlType;
    this->currentUIElement->get_CurrentControlType(&ctrlType);

    BOOL isPassword;
    this->currentUIElement->get_CurrentIsPassword(&isPassword);

    this->isTextbox = ctrlType == UIA_EditControlTypeId && !isPassword;
}

std::wstring UIElementManager::getCurrentText()
{
    if (this->isTextbox)
    {
        void* patternObject;
        if (!this->currentUIElement->GetCurrentPatternAs(UIA_ValuePatternId, IID_IUIAutomationValuePattern, &patternObject))
        {
            BSTR bs;
            static_cast<IUIAutomationValuePattern *>(patternObject)->get_CurrentValue(&bs);
            return _bstr_t(bs);
        }
        else if (!this->currentUIElement->GetCurrentPatternAs(UIA_TextPatternId, IID_IUIAutomationTextPattern, &patternObject))
        {
            BSTR bs;
            IUIAutomationTextRange *range;
            static_cast<IUIAutomationTextPattern *>(patternObject)->get_DocumentRange(&range);
            range->GetText(-1, &bs);
            return _bstr_t(bs, false);
        }
    }
    else
        return L"";
}

void UIElementManager::setCurrentText(std::wstring str)
{
}