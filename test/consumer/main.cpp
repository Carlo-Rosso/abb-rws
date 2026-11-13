#include <abb_rws/rws_interface.h>

int main()
{
  // Only checks that the headers compile and the library links.
  using RWSVersion = abb::rws::POCOClient::RWSVersion;
  return RWSVersion::RWS2 == RWSVersion::RWS1 ? 1 : 0;
}
