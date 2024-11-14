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

#include "tclap/CmdLine.h"
#include "misc/string_io.h"

using namespace std;
using namespace ROOT::RDF;
using namespace ROOT::VecOps;
namespace fs = std::filesystem;

double twopi = 2*TMath::Pi();

typedef vector<RResultPtr<TH1D>> H1D;

int nperiod(double t_, double phi_, double omega0){

  // time corresponding to number of period
  return (t_*omega0 - phi_)/twopi;

}

double tperiod(int n_, double phi_, double omega0){

  // time corresponding to number of period
  return ((n_*twopi)+phi_)/omega0;

}

void writeHists(H1D &vecIn, string fpath){

  TFile *fout = TFile::Open( fpath.c_str() , "RECREATE");
  
  for (auto& x : vecIn ){
    x.GetPtr()->Write();
  }
  // normalize to 1
  //TH1D *h0 = (TH1D*)gROOT->GetListOfFiles()->FindObject("energy0d")->Clone("norm_energy0d");
  //TH1D *h1 = (TH1D*)gROOT->GetListOfFiles()->FindObject("energy90d")->Clone("norm_energy90d");
  //TH1D *h2 = (TH1D*)gROOT->GetListOfFiles()->FindObject("energy180d")->Clone("norm_energy180d");

  //h0->SetLineColor(4);
  //h1->SetLineColor(2);
  //h2->SetLineColor(1);

  //h0->Scale(1./h0->Integral());
  //h1->Scale(1./h1->Integral());
  //h2->Scale(1./h2->Integral());

  //h0->Write();
  //h1->Write();
  //h2->Write();
  
  fout->Close();
}

template<typename T>
void phaseVal(T &rdf, H1D &vecHists){

  TH1DModel wiggle         = TH1DModel( "wiggle"         ,  "wiggle;time [#mus];N"              , 4000, 0, 0.1492*4000 );
  TH1DModel wiggle_3prd    = TH1DModel( "wiggle_3prd"    ,  "wiggle at 3 periods;time [#mus];N" , 4000, 0, 0.1492*4000 );
  TH1DModel wiggle_ph      = TH1DModel( "wiggle_ph"      ,  "wiggle_ph;phase [rad];N"           , 4000,0,0.1492*4000*2*3.142 );
  TH1DModel wiggle_ph_mod  = TH1DModel( "wiggle_ph_mod"  ,  "wiggle_ph_mod;phase [rad];N"       , 10,0,2*3.142 );

  TH1DModel energy0d    = TH1DModel( "energy0d"    , "E(#phi=0 deg); Cluster Energy [MeV]; a.u."   , 31, 0, 3100);
  TH1DModel energy90d   = TH1DModel( "energy90d"   , "E(#phi=90 deg); Cluster Energy [MeV]; a.u."  , 31, 0, 3100);
  TH1DModel energy180d  = TH1DModel( "energy180d"  , "E(#phi=180 deg); Cluster Energy [MeV]; a.u." , 31, 0, 3100);

  // selection
  auto plot1 = rdf.Filter("isAboveE_thres && isInWindow").Histo1D( wiggle , "Time"   );
  auto plot2 = rdf.Filter("isAboveE_thres && isIn3prdWin").Histo1D( wiggle_3prd , "Time"   );
  auto plot3 = rdf.Filter("isAboveE_thres && isInWindow").Histo1D( wiggle_ph , "phase"   );
  auto plot4 = rdf.Filter("isAboveE_thres && isInWindow").Histo1D( wiggle_ph_mod , "phase_mod"   );

  auto plot5 = rdf.Filter("isInWindow && TMath::Nint(phase_mod*180/TMath::Pi()) == 0").Histo1D( energy0d , "clusterEdep" );
  auto plot6 = rdf.Filter("isInWindow && TMath::Nint(phase_mod*180/TMath::Pi()) == 90").Histo1D( energy90d , "clusterEdep" );
  auto plot7 = rdf.Filter("isInWindow && TMath::Nint(phase_mod*180/TMath::Pi()) == 180").Histo1D( energy180d , "clusterEdep" );

  vecHists.emplace_back(plot1);
  vecHists.emplace_back(plot2);
  vecHists.emplace_back(plot3);
  vecHists.emplace_back(plot4);
  vecHists.emplace_back(plot5);
  vecHists.emplace_back(plot6);
  vecHists.emplace_back(plot7);

}

int main(int argc, char **argv) {

  // start time
  TStopwatch time;
  time.Start();

  TCLAP::CmdLine cmd( "validation" , ' ' , "1.0" );
  TCLAP::ValueArg<std::string> inFile_   ( "f" , "inFile"    , "Root file in"              , true  , "dummy"    , "string" , cmd );
  TCLAP::ValueArg<std::string> outPath_  ( "o" , "outPath"   , "Root file out path"        , true  , "./"       , "string" , cmd );
  TCLAP::ValueArg<double>      thePhi_   ( "p" , "phi0"      , "Initial Phi value"         , true  , 5.12       , "double" , cmd );
  //TCLAP::ValueArg<int>         ncalo_    ( "c" , "ncalo"     , "Calorimeter Number"        , true , 10         , "int" , cmd );
  //TCLAP::ValueArg<double>      scale_    ( "m" , "scale"     , "Set Energy Scale"          , false  , 1.         , "double" , cmd );
  TCLAP::ValueArg<double>      t0_       ( "s" , "t_start"   , "The start time"            , true  , 0.          , "double" , cmd );
  TCLAP::ValueArg<double>      t1_       ( "e" , "t_end"     , "The end time"              , true  , 500.        , "double" , cmd );
  TCLAP::ValueArg<double>      r_    ( "r" , "omegaa"     , "The Omega Value"           , true  , -10        , "double" , cmd );
  //TCLAP::ValueArg<int>         nxtal_    ( "x" , "nxtal"     , "Crystal number"            , false  , -99         , "int" , cmd );
  TCLAP::SwitchArg             isMC_     ( "d" , "isMC"      , "isMC or isData"            , cmd   , false );
  TCLAP::ValueArg<int>         n_        ( "n" , "ncore"     , "Number of core used"       , false  , 5   , "int" , cmd );


  cmd.parse( argc, argv );

  string inFile = inFile_.getValue();
  string outPath = outPath_.getValue();
  double phi = thePhi_.getValue();
  //int nCalo = ncalo_.getValue();
  //double scale = scale_.getValue();
  double t0 = t0_.getValue();
  double t1 = t1_.getValue();
  double r = r_.getValue();
  //int nXtal = nxtal_.getValue();
  bool isMC = isMC_.getValue(); // 1 isMC ; 0 isData
  int ncore = n_.getValue();
  
  // check argument
  cout<<"inFile   : "<< inFile  <<endl;
  cout<<"outPath  : "<< outPath <<endl;
  cout<<"thePhi   : "<< phi     <<endl;
  //cout<<"ncalo    : "<< nCalo   <<endl;
  //cout<<"theScale : "<< scale   <<endl;
  cout<<"t0       : "<< t0      <<endl;
  cout<<"t1       : "<< t1      <<endl;
  //cout<<"nxtal    : "<< nXtal   <<endl;
  cout<<"isMC     : "<< isMC    <<endl;
  cout<<"ncore    : "<< ncore   <<endl;

  // multi-thread
  //may return 0 when not able to detect
  //const auto processor_count = std::thread::hardware_concurrency();
  //cout << "ncpu detected : " << processor_count << ", using it all!" << endl;
  ROOT::EnableImplicitMT(ncore);
  //ROOT::DisableImplicitMT();
  ROOT::EnableThreadSafety();
  cout<<"Multi-threading, using ncore : "<< ncore   <<endl;

  // startand end time
  double omega_a = r;
  double n0 = nperiod(t0, phi, omega_a);
  double n1 = nperiod(t1, phi, omega_a);
  double t3 = tperiod(n0+3, phi, omega_a);
  string outpath = (contain(outPath,".root")) ? outPath : Form( "%s/ana_%s.root" , outPath.c_str(), fs::path(inFile).stem().c_str() );
  
  cout<<endl;
  cout<<"On-the-fly computed variables"<<endl;
  cout<<"Omega a        : "<< omega_a <<" MHz"<<endl;
  cout<<"t0             : "<< t0 <<" us corresponds to period n : "<< n0 <<endl;
  cout<<"t1             : "<< t1 <<" us corresponds to period n : "<< n1 <<endl;
  cout<<"t3             : "<< t3 <<" us "<< endl;
  cout<<"elapsed period : "<< n1 - n0 <<endl;

  // filelist
  if (!(contain(inFile,".txt"))){
    cout<<"Input must be in txt"<<endl;
    return 0;
  }

  H1D vecHists;
  vector<string> flist;
  ifstream file(inFile);
  string fstr;
  while (getline(file, fstr)) { if(fstr.find('#')==std::string::npos) flist.push_back(fstr); }

  ROOT::RDataFrame df( "xtal_study" , flist );
  auto rdf = RNode(df);

  // define derived variable
  rdf = rdf.Define( "twoPi" , to_string(twopi) );
  //rdf = rdf.Define( "scale" , to_string(scale) );
  //rdf = rdf.Define( "scale"        , [&scale](){return scale;} );
  rdf = rdf.Define( "nCalo" , "calo_Index_sca" );
  rdf = rdf.Define( "Time"  ,"cluster_time" );

  // Cluster Energy definition
  rdf = rdf.Define( "clusterEdep"  , "cluster_energy" );
  
  //rdf = rdf.Define( "caloIsHit" , (isMC) ? "abs(caloHitCaloNumEAvg - nCalo) < 0.01" : "1==1" ); //
  rdf = rdf.Define( "phase"     , Form("(Time*%f)-%f",omega_a, phi) );
  rdf = rdf.Define( "phase_mod" , "fmod(phase,twoPi)" );
  //rdf = rdf.Define( "upperBound" , (!isMC) ? "Time<600 && clusterEdep<8000" : "1==1" ); //
  
  // switches
  rdf = rdf.Define("isAboveE_thres", (isMC) ? "clusterEdep>1600" : "clusterEdep>1700" );
  rdf = rdf.Define("isInWindow", Form("Time>%f && Time<%f",t0,t1) ); //
  rdf = rdf.Define("isIn3prdWin", Form("Time>%f && Time<%f",t0,t3) );
  //rdf = rdf.Define("isNcalo" , Form("nCalocluster==%d", nCalo) ); //
  
  // analysis
  phaseVal(rdf,vecHists);
  writeHists(vecHists,outpath);
 
  time.Stop();
  time.Print();
  
  return 0;
}
