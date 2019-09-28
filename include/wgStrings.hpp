#ifndef WGSTRINGS_H
#define WGSTRINGS_H


// system includes
#include <string>

int my_stoi(const char *str, int& value, std::size_t* pos = 0, int base = 10);
int my_stoi(const std::string& str, int& value, std::size_t* pos = 0, int base = 10);

int my_stof(const char *str, float& value, std::size_t* pos = 0);
int my_stof(const std::string& str, float& value, std::size_t* pos = 0);

int my_stod(const char *str, float& value, std::size_t* pos = 0);
int my_stod(const std::string& str, float& value, std::size_t* pos = 0);


#endif /* WGSTRINGS_H */
