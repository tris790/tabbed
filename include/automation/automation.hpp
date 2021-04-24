#pragma once

class Automation {
public: 
    Automation();
    ~Automation();
    Automation(Automation &automation);
    void getCurrentWidget();
    void registerOnFocusEvent();

};