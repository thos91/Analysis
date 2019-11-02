// system includes
#include <stdexcept>
#include <string>

// user includes
#include "wgStrings.hpp"
#include "wgLogger.hpp"

int my_stoi(const std::string& str, int& value, std::size_t* pos, int base) {
  // wrapping std::stoi because it may throw an exception

  try {
    value = std::stoi(str, pos, base);
    return 0;
  }

  catch (const std::invalid_argument& ia) {
    Log.eWrite("(" + str + ") Invalid argument: " + std::string(ia.what()));
    return -1;
  }

  catch (const std::out_of_range& oor) {
    Log.eWrite("(" + str + ") Out of range: " + std::string(oor.what()));
    return -2;
  }

  catch (const std::exception& e)
  {
    Log.eWrite("(" + str + ") Other exception: " + std::string(e.what()));
    return -3;
  }
}

int my_stoi(const char *str, int& value, std::size_t* pos, int base) {
  return my_stoi(std::string(str), value, pos, base);
}

int my_stof(const std::string& str, float& value, std::size_t* pos) {
  // wrapping std::stoi because it may throw an exception

  try {
    value = std::stof(str, pos);
    return 0;
  }

  catch (const std::invalid_argument& ia) {
    Log.eWrite("(" + str + ") Invalid argument: " + std::string(ia.what()));
    return -1;
  }

  catch (const std::out_of_range& oor) {
    Log.eWrite("(" + str + ") Out of range: " + std::string(oor.what()));
    return -2;
  }

  catch (const std::exception& e)
  {
    Log.eWrite("(" + str + ") Other exception: " + std::string(e.what()));
    return -3;
  }
}

int my_stof(const char *str, float& value, std::size_t* pos) {
  return my_stof(std::string(str), value, pos);
}

int my_stod(const std::string& str, double& value, std::size_t* pos) {
  // wrapping std::stoi because it may throw an exception

  try {
    value = std::stod(str, pos);
    return 0;
  }

  catch (const std::invalid_argument& ia) {
    Log.eWrite("(" + str + ") Invalid argument: " + std::string(ia.what()));
    return -1;
  }

  catch (const std::out_of_range& oor) {
    Log.eWrite("(" + str + ") Out of range: " + std::string(oor.what()));
    return -2;
  }

  catch (const std::exception& e)
  {
    Log.eWrite("(" + str + ") Other exception: " + std::string(e.what()));
    return -3;
  }
}

int my_stod(const char *str, double& value, std::size_t* pos) {
  return my_stod(std::string(str), value, pos);
}
