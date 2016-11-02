#include <ILP/Exception.h>

const char* ILP::Exception::what() const noexcept
{
  return msg.c_str();
}

std::ostream& operator<<( std::ostream& oss, ILP::Exception &e)
{
  return oss << e.msg;
}
