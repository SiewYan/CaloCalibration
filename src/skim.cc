#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"
#include "ROOT/RVec.hxx"
#include "ROOT/RSnapshotOptions.hxx"
#include "ROOT/RCsvDS.hxx"

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

#include "TRandom3.h"

using namespace std;
using namespace ROOT::RDF;
using namespace ROOT::VecOps;
namespace fs = std::filesystem;
                                         
double smearing(double& energy){
  
  // Lawrence's
  //double Ce=0.028;
  //double Se=0.047;
  
  double Ce=0.0186;
  double Se=0.0356;
  
  double resolution = sqrt( pow(Ce,2) + ( pow(Se,2) / (energy/1000)  ) );
  double noise = gRandom->Gaus(0, resolution*energy);
  
  return energy + noise;
  
}

template<typename T>
auto smearXtal( T &rdf , const string xtal_energy_out , const string xtal_energy_in ){
  
  // lambda function
  auto smearIt = [&]( RVec<double>& energy_ ){
    
    const int Vsize_ = energy_.size();
    RVec<double> smeared_energy;

    for ( int i = 0 ; i < Vsize_ ; i++ ){
      double energy = energy_[i];
      smeared_energy.emplace_back(smearing(energy));
    }
    return smeared_energy;
  };

  return rdf.Define( xtal_energy_out , smearIt , {xtal_energy_in} );
  
}

int main(int argc, char **argv) {

  // start time
  TStopwatch time;
  time.Start();

  TCLAP::CmdLine cmd( "skim" , ' ' , "1.0" );
  TCLAP::ValueArg<std::string> in_     ( "f" , "in"     , "Text file in"              , true  , "some.txt"   , "string" , cmd );
  TCLAP::ValueArg<std::string> out_    ( "o" , "out"    , "Root file out path"        , true  , "some.root"  , "string" , cmd );
  TCLAP::ValueArg<std::string> tree_   ( "t" , "tree"   , "Tree folder name"          , false  , "GGAnalyzer" , "string" , cmd );
  //TCLAP::ValueArg<int>         nevent_ ( "e" , "nevent" , "Number event"              , false  , -999   , "int" , cmd );
  TCLAP::ValueArg<int>         ncore_  ( "n" , "ncore"  , "Number of core used"       , false  , 3      , "int" , cmd );
  TCLAP::SwitchArg             isRE_   ( "x" , "isRE"   , "isRE"                      , cmd   , false );

  cmd.parse( argc, argv );

  string in   = in_.getValue();
  string out  = out_.getValue();
  string tree = tree_.getValue();
  //int nevent  = nevent_.getValue();
  bool isRE   = isRE_.getValue();
  int ncore   = ncore_.getValue();

  bool isMC = (
               in.find("reconWest") != std::string::npos ||
               in.find("reconEast") != std::string::npos ||
               in.find("Run") != std::string::npos ||
	       in.find("run") != std::string::npos ||
	       in.find("data-") != std::string::npos ||
	       in.find("_re") != std::string::npos ||
	       in.find("_rw") != std::string::npos
               ) ? false : true;
  
  // check argument
  cout<<"text file    : "<< in     <<endl;
  cout<<"output root  : "<< out    <<endl;
  //cout<<"nevent       : "<< nevent <<endl;
  cout<<"isRE         : "<< isRE   <<endl;
  cout<<"ncore        : "<< ncore  <<endl;
  cout<<"isMC         : "<< isMC   <<endl;
  
  // multi-thread
  if (ncore==0){
    ROOT::DisableImplicitMT();
    cout<<"Running on one core : "<<endl;
  }
  else{
    ROOT::EnableImplicitMT(ncore);
    ROOT::EnableThreadSafety();
    cout<<"Multi-threading, using ncore : "<< ncore   <<endl;
  }
  
  string outpath = (contain(out,".root")) ? out : Form( "%s/draw_%s.root" , out.c_str(), fs::path(in).stem().c_str() );
  
  // processing filelsit from text
  vector<string> flist;
  if (!(contain(in,".txt"))){
    cout<<"Input must be text file"<<endl;
    return 0;
  }
  else{
    // filelist
    std::ifstream file(in);
    std::string str;
    while (std::getline(file, str)) { flist.push_back(str); } 
  }

  auto df = ROOT::RDataFrame(tree,flist);
  auto rdf = RNode(df);

  // we are in business
  // 1. apply time randomization on mc
  // 2. convert clock tick to time (x1.25 ONLY for data)
  // 3. standardize time in micro second (mc and data)
  // 4. skim data, remove 8 GeV event

  // initialization
  TRandom3* rnd_ = new TRandom3(123456789);

  // definition
  //rdf = rdf.Define("rand", [&rnd_](){ return 149.2*(rnd_->Uniform()-0.5); });
  if (!isMC){
    // data does not have vector of calo index, and calo_HitCaloNumEAvg
    rdf = rdf.Define( "dummy1" , [](){return -9999;} );
    rdf = rdf.Define( "dummy2" , [](){return RVec<int>(2,-9999);} );

    // time randomization and clock tick conversion for RE
    if (isRE){
      // 149.2 is synchrotron frequency                                                                                                                                                                                    
      double rnd_time = 149.2 * (rnd_->Uniform() - 0.5);
      rdf = rdf.Define("time_re","(time*1.25) + "+ std::to_string(rnd_time));
    }
    
    // resolve RW and RE definition
    rdf = rdf.Define( "data_time"           , (isRE) ? "time_re/1000." : "cluster_Time"     );
    rdf = rdf.Define( "data_cluster_energy" , (isRE) ? "energy"        : "cluster_Energy"   );
    rdf = rdf.Define( "data_xtal_energy"    , (isRE) ? "crystalEnergy" : "xtal_Energy"      );
    rdf = rdf.Define( "data_caloIndex"      , (isRE) ? "caloIndex"     : "cluster_CaloNum"  );
    rdf = rdf.Define( "data_xtalIndex"      , (isRE) ? "crystalIndex"  : "xtal_XtalNum"     );
  }
  
  // rename
  rdf = rdf.Define("cluster_time"        , (isMC) ? "hit_Time"         : "data_time"            ); // micro-second
  rdf =	rdf.Define("cluster_energy"      , (isMC) ? "hit_Edep"         : "data_cluster_energy"  );
  rdf = rdf.Define("xtal_energy"         , (isMC) ? "track_Edep"       : "data_xtal_energy"     ); // vector
  rdf = rdf.Define("calo_Index_sca"      , (isMC) ? "hit_CaloNum"      : "data_caloIndex"       ); // MC and Data int
  rdf =	rdf.Define("calo_Index_vec"      , (isMC) ? "track_CaloNum"    : "dummy2"               ); // MC's is vector, data's is int
  rdf = rdf.Define("xtal_Index"          , (isMC) ? "track_XtalNum"    : "data_xtalIndex"       );
  rdf =	rdf.Define("calo_HitCaloNumEAvg" , (isMC) ? "hit_CaloNumEAvg"  : "dummy1"               );

  /*
  // positron information
  // hits.trackID == 4 && hits.pdgID == -11
  std::string positron="track_trackID == 4 && track_pdgID == -11";
  if (isMC){
    //rdf = rdf.Define( "positron_time"       , Form( "track_Time[%s]"    , positron.c_str() ) );
    rdf = rdf.Define( "positron_time"       , "cluster_time" );
    rdf = rdf.Define( "positron_calo_Index" , Form( "track_CaloNum[%s]" , positron.c_str() ) );
    rdf = rdf.Define( "positron_xtal_Index" , Form( "track_XtalNum[%s]" , positron.c_str() ) );
    rdf = rdf.Define( "positron_edep"       , Form( "track_Edep[%s]"    , positron.c_str() ) );
    rdf = rdf.Define( "positron_energy"     , Form( "track_Energy[%s]"  , positron.c_str() ) );
    rdf = rdf.Define( "positron_trackID"    , Form( "track_trackID[%s]"  , positron.c_str() ) );
    rdf = rdf.Define( "positron_pdgID"      , Form( "track_pdgID[%s]"  , positron.c_str() ) );
    rdf = smearXtal( rdf , "positron_energy_smear"  , "positron_energy" );
    rdf = smearXtal( rdf , "positron_edep_smear"  , "positron_edep" );
  }
  */
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // skim
  if (!isMC){
    rdf = rdf.Filter( "cluster_energy<8000" , "remove event greater than 8 GeV" );
    rdf = rdf.Filter( "cluster_time>25" , "remove event less than 25 micro second" );
    
    //rdf = rdf.Define( "xtal_energy_v2" , "xtal_energy"  ); // dummy for data
  }
  else{
    
    // smearing
    rdf = smearXtal( rdf , "xtal_energy_smear"  , "xtal_energy" );
    
    rdf = rdf.Filter( "cluster_energy>0" , "deposited energy must be more than zero" );
    rdf = rdf.Filter( "cluster_time>0"  , "cluster time must be positive" );
    
  }

  // output
  std::vector<std::string> variables;
  
  if (!isMC){
    variables= {
      "cluster_time",
      "cluster_energy",
      "xtal_energy",
      "calo_Index_sca",
      "calo_Index_vec",
      "xtal_Index"
      //"beamIntegralVsRun"
      //"calo_HitCaloNumEAvg" 
    };
  }
  else{
    variables= {
      "cluster_time",
      "cluster_energy",
      "xtal_energy",
      "xtal_energy_smear",
      "calo_Index_sca",
      "calo_Index_vec",
      "xtal_Index",
      "calo_HitCaloNumEAvg"
      //"positron_calo_Index",
      //"positron_xtal_Index",
      //"positron_time",
      //"positron_energy",
      //"positron_edep",
      //"positron_energy_smear",
      //"positron_edep_smear"
    };
  }
  
  auto outdf = rdf;
  outdf.Snapshot( "xtal_study" , outpath , variables );
  
  auto report = outdf.Report();
  report->Print();
  ROOT::RDF::SaveGraph( outdf , "graph_skim.dot" );
  
  time.Stop();
  time.Print();
  
  return 0;
}
