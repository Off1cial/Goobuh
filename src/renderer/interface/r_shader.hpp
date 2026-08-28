#pragma once

#include <string>

class IShader
{
  public:
    virtual ~IShader() = default;
    virtual void Use() = 0;

  private:
    std::string vertpath;
    std::string fragpath;

};