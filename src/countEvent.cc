#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"
#include "ROOT/RVec.hxx"
#include "ROOT/RSnapshotOptions.hxx"

#include "Math/Vector4D.h"
#include "TStopwatch.h"
#include "TMath.h"
#include "TStyle.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>

#include "utility" // std::pair
#include <algorithm> // for std::find
#include <iterator> // for std::begin, std::end
#include <thread>
#include <filesystem>

#include "blinder/Blinders.hh"
#include "tclap/CmdLine.h"
#include "misc/string_io.h"

using namespace std;
using namespace ROOT::RDF;

int main(int argc, char **argv) {
  
  // start time
  TStopwatch time;
  time.Start();

  TCLAP::CmdLine cmd( "analysis" , ' ' , "1.0" );
  TCLAP::ValueArg<std::string> inFile_   ( "f" , "inFile"    , "Root file in"              , true  , "dummy"    , "string" , cmd );
  //TCLAP::SwitchArg             isMC_     ( "d" , "isMC"      , "isMC or isData"            , cmd   , false );
  TCLAP::ValueArg<int>         n_        ( "n" , "ncore"     , "Number of core used"       , false  , 5   , "int" , cmd );

  cmd.parse( argc, argv );

  string inFile = inFile_.getValue();
  //bool isMC = isMC_.getValue();
  int ncore = n_.getValue();
  
  // check argument
  cout<<"inFile   : "<< inFile  <<endl;
  cout<<"ncore    : "<< ncore   <<endl;

  // multi-thread
  ROOT::EnableImplicitMT(ncore);
  //ROOT::DisableImplicitMT();
  ROOT::EnableThreadSafety();
  cout<<"Multi-threading, using ncore : "<< ncore   <<endl;

  // filelist
  if (!(contain(inFile,".txt"))){
    cout<<"Input must be in txt"<<endl;
    return 0;
  }

  vector<string> flist;
  ifstream file(inFile);
  string fstr;
  while (getline(file, fstr)) { if(fstr.find('#')==std::string::npos) flist.push_back(fstr); }

  ROOT::RDataFrame df( "xtalTree/xtal_info" , flist );
  auto rdf = RNode(df);
  
  auto nevts = rdf.Count();
  
  std::cout<<"Number of event : "<< nevts.GetValue() <<std::endl;
  
  time.Stop();
  time.Print();
  
  return 0;
}
