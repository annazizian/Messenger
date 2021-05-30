#include <messenger/helpers.h>

std::string generate_guid()
{
    // Generate a random guid with 55 lowercase english letters
    std::string s = ""; 
    for (int i=0;i<55;i++)
	{
        s += rand()%26+97;
    }
    return s;
} 

int get_ts()
{
    // get the current timestamp
    return std::time(nullptr);
}