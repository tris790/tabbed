#include <corrector/corrector.hpp>

std::wstring Corrector::autocomplete(std::wstring input)
{
    auto corrected = L"*123*" + input;
    return corrected;
}