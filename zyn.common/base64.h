#ifndef BASE64_H_FOR_ZYNLAB
#define BASE64_H_FOR_ZYNLAB

#include <string>

std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);

#endif // BASE64_H_FOR_ZYNLAB
