#include "sync.h"

using namespace std;

ostream & operator << (ostream &out, const SyncTimer &st) {
  long long elapsed = st.elapsed_ms();
  out.write(reinterpret_cast<char*>(&elapsed), sizeof(long long));
  return out;
}

istream & operator >> (istream &in, SyncTimer &st) {
  long long elapsed;
  in.read(reinterpret_cast<char*>(&elapsed), sizeof(long long));
  st.start();
  st.offset = elapsed;
  return in;
}
