#include <iostream>
#include <string>
#include "TMath.h"
#include "tclap/CmdLine.h"
#include <filesystem>
#include <fstream>

#include "TFile.h"
#include "TFitResult.h"
#include "TF1.h"

#include "blinder/Blinders.hh"


using namespace std;

double twopi = 2*TMath::Pi();
namespace fs = std::filesystem;
blinding::Blinders::fitType ftype = blinding::Blinders::kOmega_a;
blinding::Blinders getBlinded( ftype, "estimate time" );

int main(int argc, char **argv) {

  TCLAP::CmdLine cmd( "estimateTime" , ' ' , "2.0" );
  TCLAP::ValueArg<std::string> in_     ( "f" , "fitin"     , "fit root file"                , true  , "some.txt"   , "string" , cmd );
  TCLAP::MultiArg<double>      time_   ( "t" , "time"      , "Time Parameter list"          , true  , "double"          , cmd );
  
  cmd.parse( argc, argv );

  std::string in = in_.getValue();
  auto p = fs::path(in);
  //cout << p.parent_path() << endl;

  TFile* fit_root = new TFile( in.c_str() );
  TF1* fit_func = (TF1*) fit_root->Get( "fit_func" );

  double rval = fit_func->GetParameter(3);
  double omega = getBlinded.paramToFreq(rval);
  double phase = fit_func->GetParameter(4);

  cout << " -- reading -- " << endl;
  cout << " rval  : " << rval << endl;
  cout << " omega : " << omega << endl;
  cout << " phase : " << phase << endl;

  double t_start = time_.getValue()[0];
  double t_end = time_.getValue()[1];

  double pstart = phase / omega;
  double period1 = (twopi + phase) / omega;
  double period2 = (2*twopi + phase) / omega;
  double period3 = (3*twopi + phase) / omega;
  double pdiff = period1 - pstart;

  cout << "period start           : " << pstart  << " us" <<endl;
  cout << "first period elapased  : " << period1  << " us ; delta(t) : "<< period1 - pstart <<endl;
  cout << "second period elapased : " << period2  << " us ; delta(t) : "<< period2 - period1 <<endl;
  cout << "third period elapased  : " << period3  << " us ; delta(t) : "<< period3 - period2 <<endl;

  int ti = TMath::Nint((t_start - pstart)/pdiff);
  int tf = TMath::Nint((t_end - pstart)/pdiff);
  
  cout << "--> Elapsed " << ti << " period, from pstart, to reach : " << pstart + (ti*pdiff) << " us" << endl;
  cout << "--> "<< ti+1 << " period will be " << pstart + ((ti+1)*pdiff) << " us" << endl;

  cout << "--> Elapsed " << tf << " period, from pstart, to reach : " << pstart + (tf*pdiff) << " us" << endl;
  cout << "--> "<< tf+1 << " period will be " << pstart + ((tf+1)*pdiff) << " us" << endl;

  double pre_determined_start = pstart + ((ti+1)*pdiff);
  // add 3 period
  double go_3period = pstart + ((ti+3)*pdiff);

  cout << "Starting from " << pre_determined_start  << "us , after 3 periods is " << go_3period << endl;

  // output csv file
  fstream fout;

  // opens an existing csv file or creates a new file.
  cout<< " --> output : "<< Form("%s/%s_cond.txt", p.parent_path().c_str(), p.filename().replace_extension().c_str() ) << endl;
  fout.open(Form("%s/%s_cond.txt", p.parent_path().c_str(), p.filename().replace_extension().c_str() ), ios::out ); //| ios::app);

  fout << omega << " " << phase << " " << pstart + ((ti+1)*pdiff) << " " << pstart + ((tf+1)*pdiff) << "\n" ;
  
  return 0;
}
