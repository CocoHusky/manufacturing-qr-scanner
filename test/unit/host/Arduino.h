#pragma once
#include <string>
#include <cstdint>
#include <type_traits>
class String:public std::string {
public:
 using std::string::string;
 using std::string::operator=;
 String():std::string(){}
 String(const std::string& s):std::string(s){}
 template<class I,typename std::enable_if<std::is_integral<I>::value,int>::type=0> explicit String(I i):std::string(std::to_string(i)){}
 int indexOf(char c,int start=0)const {auto n=find(c,start);return n==npos?-1:(int)n;}
 String substring(int a,int b)const{return substr(a,b-a);}
};
