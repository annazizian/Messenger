#include <messenger/helpers.h>

std::string generate_guid()
{
    std::string s = ""; 
    for (int i=0;i<55;i++)
	{
        s += rand()%26+97;
    }
    return s;
} 

int get_ts()
{
    return std::time(nullptr);
}