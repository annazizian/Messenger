#pragma once 
//a-n include e anum b-n, a-n include e anum c-n, b-n include e anum helpers.h, c-n include e anum helpers.h
//qani vor includey copy-paste er, kstacvi a-i mej helpers.h-i parunakutyunic 2 hat, eghav generate_guid erku angam ksahmanvi
#include <string> 
#include <cstdlib>
#include <ctime>

std::string generate_guid(){
    std::string s = ""; 
    for (int i=0;i<55;i++){
        s += rand()%26+97;
    }
    return s;
} 

int get_ts(){
    return std::time(nullptr);
}
