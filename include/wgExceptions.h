#include <string>
#include <exception>

// This exception is thrown when a file is not found or its content is invalid
class wgInvalidFile : public std::exception
{
private:
  std::string what_message = " ";
public:
  explicit wgInvalidFile(std::string message) : what_message(message) { }
  const char* what() const noexcept override
  {
	return what_message.c_str();
  }
};

// This exception is thrown while scanning a file for a specific value and the
// value is not found
class wgElementNotFound : public std::exception
{
private:
  std::string what_message = " ";
public:
  explicit wgElementNotFound(std::string message) : what_message(message) { }
  const char* what() const noexcept override
  {
	return what_message.c_str();
  }
};
