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
typedef tuple<double,double,double,double> Dtuple;

// global
//H1D vecHists;

// apply scale factor in range
//auto applySF = [](double en, double sf) {
//  return (en>500) ? en*sf : en ;
// };

//auto applySF = [](double en, double sf) {
//  return en*sf ;
// };

auto applySFxtal = [](RVec<double>& en, double sf) {
  RVec<double> en_out;
  const auto size = en.size();
  for (size_t i=0; i < size; ++i) {
    en_out.emplace_back( en[i]*sf );
  }
  return en_out;
 };

Dtuple getParam(std::string &condIn){

  //
  Dtuple row;
  ifstream in( condIn );
  double omega_, phase_, t0_, t1_;
  while ( in >> omega_ >> phase_ >> t0_ >> t1_ ){
    row = make_tuple(omega_ , phase_, t0_, t1_);
    break;
  }

  return row;
}

void writeHists(H1D &vecIn, string fpath){

  TFile *fout = TFile::Open( fpath.c_str() , "RECREATE");
  
  for (auto& x : vecIn ){
    x.GetPtr()->Write();
  }
  
  fout->Close();
}

template<typename T>
auto applyScale(T &rdf, int& caloIdx, int& xtalIdx, double& scale ){

  // lambda function
  auto eval_scale = [&](
			RVec<double>& energy_,
			RVec<int>& xtalIdxVec
			){

    const int Vsize_ = energy_.size(); 
    RVec<double> scaled_energy;
    RVec<double> xscaled_energy;
    
    for ( int i = 0 ; i < Vsize_ ; i++ ){
      int ixtal = xtalIdxVec[i];
      if (ixtal != xtalIdx) continue;
      scaled_energy.emplace_back(energy_[i]*scale);
      xscaled_energy.emplace_back(energy_[i]);
    }
    return make_pair(scaled_energy,xscaled_energy);
  };
  
  return rdf
    .Filter(Form("calo_Index_v1 == %d",caloIdx))
    .Define( "senergy" , eval_scale , {"xtal_energy","xtal_Index"} )
    .Define( "eprime" , "senergy.first" )
    .Define( "xeprime" , "senergy.second" )
    ;
}

template<typename T>
auto selectCaloMC(T &rdf, int& caloIdx, int& xtalIdx ){

  // lambda function
  auto select_calo = [&](
			 RVec<double>& energy_,
			 RVec<int>&    caloIdxVec,
			 RVec<int>&    xtalIdxVec
			 ){
    
    const int Vsize_ = energy_.size();
    RVec<double> energy_out;
    
    for ( int i = 0 ; i < Vsize_ ; i++ ){
      int icalo = caloIdxVec[i];
      int ixtal = xtalIdxVec[i];
      if ( icalo != caloIdx ) continue;
      if ( ixtal != xtalIdx ) continue;
      
      energy_out.emplace_back(energy_[i]);
    }
    return energy_out;
  };

  return rdf.Define( "eprime" , select_calo , {"xtal_energy","calo_Index_v2","xtal_Index"} );

}

template<typename T>
void phaseBinHisto(T &rdf, H1D &vecHists, bool isMC){
  
  // global
  //TH1DModel energycluster = TH1DModel( "energycluster"   , "Cluster Energy shape; Cluster Energy [MeV]; Events"                , 31, 0, 3100 );
  TH1DModel energy0check  = TH1DModel( "energy0xscaled"  , "Crystal Energy Inclusive (unscaled); Crystal Energy [MeV]; Events" , 31, 0, 3100 );
  TH1DModel energy0       = TH1DModel( "energy0"         , "Crystal Energy Inclusive (scaled); Crystal Energy [MeV]; Events"   , 31, 0, 3100 );
  // 10 bin
  TH1DModel energy1  = TH1DModel( "energy1"  , "E(#phi > 0 && #phi <= 1*(2#pi/10)); Crystal Energy [MeV]; Events"            , 31, 0, 3000);
  TH1DModel energy2  = TH1DModel( "energy2"  , "E(#phi > 1*(2#pi/10) && #phi <= 2*(2#pi/10)); Crystal Energy [MeV]; Events"  , 31, 0, 3100);
  TH1DModel energy3  = TH1DModel( "energy3"  , "E(#phi > 2*(2#pi/10) && #phi <= 3*(2#pi/10)); Crystal Energy [MeV]; Events"  , 31, 0, 3100);
  TH1DModel energy4  = TH1DModel( "energy4"  , "E(#phi > 3*(2#pi/10) && #phi <= 4*(2#pi/10)); Crystal Energy [MeV]; Events"  , 31, 0, 3100);
  TH1DModel energy5  = TH1DModel( "energy5"  , "E(#phi > 4*(2#pi/10) && #phi <= 5*(2#pi/10)); Crystal Energy [MeV]; Events"  , 31, 0, 3100);
  TH1DModel energy6  = TH1DModel( "energy6"  , "E(#phi > 5*(2#pi/10) && #phi <= 6*(2#pi/10)); Crystal Energy [MeV]; Events"  , 31, 0, 3100);
  TH1DModel energy7  = TH1DModel( "energy7"  , "E(#phi > 6*(2#pi/10) && #phi <= 7*(2#pi/10)); Crystal Energy [MeV]; Events"  , 31, 0, 3100);
  TH1DModel energy8  = TH1DModel( "energy8"  , "E(#phi > 7*(2#pi/10) && #phi <= 8*(2#pi/10)); Crystal Energy [MeV]; Events"  , 31, 0, 3100);
  TH1DModel energy9  = TH1DModel( "energy9"  , "E(#phi > 8*(2#pi/10) && #phi <= 9*(2#pi/10)); Crystal Energy [MeV]; Events"  , 31, 0, 3100);
  TH1DModel energy10 = TH1DModel( "energy10" , "E(#phi > 9*(2#pi/10) && #phi <= 10*(2#pi/10)); Crystal Energy [MeV]; Events" , 31, 0, 3100);

  auto pecheck   = rdf.Histo1D( energy0check , (!isMC) ? "xeprime" : "eprime" );
  auto pe0       = rdf.Histo1D( energy0 , "eprime" );                   // scaled inclusive
  auto pe1       = rdf.Filter("phase_mod > 0 && phase_mod <= twoPi/10").Histo1D( energy1 , "eprime" );
  auto pe2       = rdf.Filter("phase_mod > twoPi/10 && phase_mod <= 2*twoPi/10"   ).Histo1D( energy2 , "eprime" );
  auto pe3       = rdf.Filter("phase_mod > 2*twoPi/10 && phase_mod <= 3*twoPi/10" ).Histo1D( energy3 , "eprime" );
  auto pe4       = rdf.Filter("phase_mod > 3*twoPi/10 && phase_mod <= 4*twoPi/10" ).Histo1D( energy4 , "eprime" );
  auto pe5       = rdf.Filter("phase_mod > 4*twoPi/10 && phase_mod <= 5*twoPi/10" ).Histo1D( energy5 , "eprime" );
  auto pe6       = rdf.Filter("phase_mod > 5*twoPi/10 && phase_mod <= 6*twoPi/10" ).Histo1D( energy6 , "eprime" );
  auto pe7       = rdf.Filter("phase_mod > 6*twoPi/10 && phase_mod <= 7*twoPi/10" ).Histo1D( energy7 , "eprime" );
  auto pe8       = rdf.Filter("phase_mod > 7*twoPi/10 && phase_mod <= 8*twoPi/10" ).Histo1D( energy8 , "eprime" );
  auto pe9       = rdf.Filter("phase_mod > 8*twoPi/10 && phase_mod <= 9*twoPi/10" ).Histo1D( energy9 , "eprime" );
  auto pe10      = rdf.Filter("phase_mod > 9*twoPi/10 && phase_mod <= 10*twoPi/10").Histo1D( energy10 , "eprime" );

  vecHists.emplace_back(pecheck);
  vecHists.emplace_back(pe0);
  vecHists.emplace_back(pe1);
  vecHists.emplace_back(pe2);
  vecHists.emplace_back(pe3);
  vecHists.emplace_back(pe4);
  vecHists.emplace_back(pe5);
  vecHists.emplace_back(pe6);
  vecHists.emplace_back(pe7);
  vecHists.emplace_back(pe8);
  vecHists.emplace_back(pe9);
  vecHists.emplace_back(pe10);

}

int main(int argc, char **argv) {

  // start time
  TStopwatch time;
  time.Start();

  TCLAP::CmdLine cmd( "analysis" , ' ' , "2.0" );
  TCLAP::MultiArg<std::string> fileIn_   ( "d" , "filein"        , "Data text file and CSV"     , true  , "string" , cmd );
  TCLAP::ValueArg<string>      fileout_  ( "o" , "fileout"       , "File directory"             , true  , "/home/siew/gm2/df-spectrum/data/analysis" , "string" , cmd );
  TCLAP::ValueArg<int>         calo_     ( "c" , "ncalo"         , "Calo number"                , true  ,10  , "int"    , cmd );
  TCLAP::ValueArg<int>         xtal_     ( "x" , "nxtal"         , "Xtal number"                , true  ,10   , "int"    , cmd );
  TCLAP::ValueArg<double>      scale_    ( "s" , "scale"         , "Scale factor"               , true  , 1   , "double" , cmd );
  TCLAP::ValueArg<int>         n_        ( "n" , "ncore"         , "Number of core used"        , false  , 3   , "int" , cmd );
  TCLAP::SwitchArg             isMC_     ( "m" , "isMC"          , "isMC or isData"             , cmd   , false );
  
  cmd.parse( argc, argv );
  vector<std::string> fileIn = fileIn_.getValue();
  string outdir = fileout_.getValue();
  int caloIdx = calo_.getValue();
  int xtalIdx = xtal_.getValue();
  double scale = scale_.getValue();
  int ncore = n_.getValue();
  bool isMC = isMC_.getValue();

  auto p = fs::path(fileIn[0]);
  string name = (contain(fileIn[0],"rw_")) ?
    Form( "%s/ana_RW_calo%02d-xtal%02d_%s.root",outdir.c_str() , caloIdx , xtalIdx , p.filename().replace_extension().c_str() ) :
    Form( "%s/ana_RE_calo%02d-xtal%02d_%s.root",outdir.c_str() , caloIdx , xtalIdx , p.filename().replace_extension().c_str() )
    ;

  if (isMC)
    name = Form( "%s/ana_MC_calo%02d-xtal%02d_%s.root",outdir.c_str() , caloIdx , xtalIdx , p.filename().replace_extension().c_str() );

  Dtuple ftuple = getParam(fileIn[1]);

  // reading data time
  double omega_a = get<0>(ftuple);
  double phi     = get<1>(ftuple);
  double t0      = get<2>(ftuple);
  double t1      = get<3>(ftuple);

  cout << " -- reading text files -- " << endl;
  cout << " file list txt      : " << fileIn[0] << endl;
  cout << " file condition txt : " << fileIn[1] << endl;
  cout << " is MC              : " << isMC      << endl;
  cout << " Scale used         : " << scale     << endl;
  cout << " -- reading cond files -- " << endl;
  cout << " Omega              : " << omega_a << " MeV " << endl;
  cout << " Phase              : " << phi     << " rad " << endl;
  cout << " start time         : " << t0      << " us " << endl;
  cout << " end time           : " << t1      << " us " << endl;
  cout << " -- running on -- " << endl;
  cout << " calorimeter number : " << caloIdx        << endl;
  cout << " crystal number     : " << xtalIdx        << endl;

  // multi-thread
  //may return 0 when not able to detect
  //const auto processor_count = std::thread::hardware_concurrency();
  //cout << "ncpu detected : " << processor_count << ", using it all!" << endl;
  ROOT::EnableImplicitMT(ncore);
  //ROOT::DisableImplicitMT();
  ROOT::EnableThreadSafety();
  cout<<"Multi-threading, using ncore : "<< ncore   <<endl;


  // filelist
  if (!(contain(fileIn[0],".txt"))){
    cout<<"Input must be in txt"<<endl;
    return 0;
  }

  H1D vecHists;
  vector<string> flist;
  ifstream file(fileIn[0]);
  string fstr;
  int count=0;
  while (getline(file, fstr)) { if(fstr.find('#')==std::string::npos) flist.push_back(fstr); count++; }
  cout<<"number of file consumed : "<< count <<endl;

  ROOT::RDataFrame df( "xtal_study" , flist );
  auto rdf = RNode(df);

  // Definition
  rdf = rdf.Define( "twoPi"      , to_string(twopi) );
  rdf = rdf.Define( "phase"      , Form("(cluster_time*%f)-%f",omega_a, phi) );
  rdf = rdf.Define( "phase_mod"  , "fmod(phase,twoPi)" );
  rdf = rdf.Define( "isInWindow" , Form("cluster_time>%f && cluster_time<%f",t0,t1) );

  // analysis
  rdf = rdf.Filter("isInWindow"); // select time window
  rdf = (!isMC) ? applyScale( rdf , caloIdx, xtalIdx, scale) : selectCaloMC( rdf , caloIdx, xtalIdx ); // select calo and xtal
  
  // make histogram
  phaseBinHisto(rdf,vecHists,isMC);
  writeHists(vecHists,name);
 
  time.Stop();
  time.Print();
  
  return 0;
}
