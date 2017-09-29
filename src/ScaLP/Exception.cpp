#include <ScaLP/Exception.h>

const char* ScaLP::Exception::what() const noexcept
{
  return msg.c_str();
}

std::ostream& ScaLP::operator<<( std::ostream& oss, ScaLP::Exception &e)
{
  return oss << e.msg;
}
