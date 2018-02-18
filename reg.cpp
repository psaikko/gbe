#include "reg.h"
#include <iostream>

using namespace std;

ostream & operator << (ostream & out, const Registers & reg)
{
  cout << "Write " <<
       reg.AF << " " << reg.BC << " " << reg.DE << " " << reg.HL << " " << reg.SP << " " <<
       reg.PC << " " << reg.TCLK << " " << reg.IME << " " << reg.HALT << endl;
  //out << reg.AF << reg.BC << reg.DE << reg.HL << reg.SP << reg.PC << reg.TCLK << reg.IME << reg.HALT;
  out.write(reinterpret_cast<const char*>(&reg), sizeof(Registers));
  return out;
}

istream & operator >> (istream & in, Registers & reg)
{
  cout << "State " <<
       reg.AF << " " << reg.BC << " " << reg.DE << " " << reg.HL << " " << reg.SP << " " <<
       reg.PC << " " << reg.TCLK << " " << reg.IME << " " << reg.HALT << endl;

  //in >> reg.AF >> reg.BC >> reg.DE >> reg.HL >> reg.SP >> reg.PC >> reg.TCLK >> reg.IME >> reg.HALT;
  in.read(reinterpret_cast<char*>(&reg), sizeof(Registers));

  cout << "Read " <<
       reg.AF << " " << reg.BC << " " << reg.DE << " " << reg.HL << " " << reg.SP << " " <<
       reg.PC << " " << reg.TCLK << " " << reg.IME << " " << reg.HALT << endl;
  return in;
}