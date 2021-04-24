#include <cstdio>
#include <autocomplete/autocomplete.hpp>
#include <string>

int main()
{
    auto autocompleter = Autocomplete {};
    auto result = autocompleter.autocomplete("bla bla");
    printf("Test 123, %i", result);
    return 0;
}