#pragma once

#include <exception>
#include <ostream>

namespace ILP
{

  class Exception : public std::exception
  {
    public:
      std::string msg;

      Exception(std::string s) :msg(s)
      {
      }

      virtual const char* what() const noexcept override;
  };

  std::ostream& operator<<( std::ostream& oss, ILP::Exception &e);


}

