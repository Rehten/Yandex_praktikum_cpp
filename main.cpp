#include <iostream>

#include "transport_catalogue.h"

using namespace std;

int
main()
{
  TransportCatalogue catalogue(
    make_unique<RawRequestHandler>(),
    make_unique<RawResponseSeller>()
  );

  catalogue.listen_db_commands_from(cin);

  catalogue.listen_output_commands_from(cin, cout);

  return 0;
}
