#include <iostream>
#include <string>
#include "TMath.h"
#include "tclap/CmdLine.h"
#include <filesystem>
#include <fstream>
#include <tuple>
#include <map>

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"
#include "ROOT/RVec.hxx"
#include "ROOT/RSnapshotOptions.hxx"

using namespace std;
using namespace ROOT::RDF;
using namespace ROOT::VecOps;
namespace fs = std::filesystem;

typedef tuple<double,double,double,double> Dtuple;
//typedef vector<RResultPtr<TH1D>> H1D;
//typedef map<int, RResultPtr<TH1D>> xtalptr;
//typedef map<int, TH1D*> xtalpair;

double twopi = 2*TMath::Pi();
TH1DModel energy1  = TH1DModel( "xtalenergy"  , "Crystal Energy Spectrum; Crystal Energy [MeV]; Events" , 31, 0, 3100);

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

vector<string> getFileList(std::string &txtIn){

  vector<string> flist;
  ifstream file(txtIn);
  string fstr;
  int count=0;
  while (getline(file, fstr)) {
    if(fstr.find('#')==std::string::npos){
      flist.push_back(fstr);
      count++;
    }
  }
  
  cout<<"number of file consumed : "<< count <<endl;
  return flist;
}

void getMedium(TH1D* hist){

  // calculate and print the "median"
  double x, q;
  q = 0.5; // 0.5 for "median"
  //hist->ComputeIntegral(); // just a precaution
  hist->GetQuantiles(1, &x, &q);
  cout << "median = " << x << endl;
}

void checkrange(TH1D* hist, double lowerlimit, vector<double> qval){

  // ( lowerlimit , upperlimit ]
  int nbin = hist->GetXaxis()->GetNbins();
  int binmax = hist->FindLastBinAbove();
  double xmax = hist->GetXaxis()->GetBinCenter(binmax);
  double halfbin = hist->GetXaxis()->GetBinCenter(1);
  
  cout<<"binmax : " << binmax << endl;
  cout<<"xmax   : " << xmax << endl;

  double total_integral = (lowerlimit==0) ? hist->Integral() :
    hist->Integral( hist->GetXaxis()->FindFixBin(lowerlimit), hist->GetXaxis()->FindFixBin(xmax+halfbin))
    ;
  double lower_integral = total_integral*qval[0]; // 0.3
  double upper_integral = total_integral*qval[1]; // 0.8
  
  vector<pair<int,double>> quantile_bin;
  int count=0;
  double content=0.;
  
  for( int i = 1; i < nbin+1; i++ ){

    // xbin
    double xbin = halfbin + (hist->GetXaxis()->GetBinCenter(i));
    
    if (lowerlimit > 0 && xbin <= lowerlimit ) continue;
    if (lowerlimit > 0 && xbin > xmax+halfbin ) break;

    double xcontent = hist->GetBinContent(i);
    content+= xcontent;
    
    cout << "bin number : " << i <<" ; xbin : "<< halfbin + (hist->GetXaxis()->GetBinCenter(i)) <<" MeV ; xcontent : "<< xcontent <<" ; cumulative content : " << content << " ; percentage : " << (content/total_integral)*100 <<" %"<< endl;

    // lower bin
    if ( content >= lower_integral && count == 0 ){
      cout << "lower edge bin number : " << i << endl;
      quantile_bin.push_back( make_pair( i , halfbin + (hist->GetXaxis()->GetBinCenter(i)) ) );
      count++;
    }
    
    // upper bin
    if ( content >= upper_integral && count == 1 ){
      cout << "upper edge bin number : " << i << endl;
      quantile_bin.push_back( make_pair( i , halfbin + (hist->GetXaxis()->GetBinCenter(i)) ) );
      count++;
    }
  }

  cout<<"hdata integral       : " << total_integral << endl;
  getMedium(hist);
  cout<<"hdata lower integral : " << lower_integral <<" ; the bin number : " << quantile_bin[0].first << " ; xbin : " << quantile_bin[0].second <<" MeV"<<endl;
  cout<<"hdata upper integral : " << upper_integral <<" ; the bin number : " << quantile_bin[1].first << " ; xbin : " << quantile_bin[1].second <<" MeV"<<endl;
  
}

template<typename T>
auto plotXtal( T &rdf_ , int &ncalo_ , int &nxtal_ , bool isMC=false ){
  
  // https://root.cern/doc/master/classROOT_1_1RDataFrame.html
  // avoid just-in-time compilation

  // lambda function for Data
  auto eval_data = [&](
		       RVec<int>& Vxtal_,
		       RVec<double>& Venergy_
		       ){
    
    RVec<float> energy_out;
    
    const int Vsize_ = Venergy_.size();
    for ( auto i = 0 ; i < Vsize_ ; i++ ){
      if ( Vxtal_[i] != nxtal_ ) continue;
      energy_out.emplace_back(Venergy_[i]);
    }
    
    return energy_out;
  };
  
  // lambda function for MC
  auto eval_mc = [&](
		     RVec<int>& Vcalo_,
		     RVec<int>& Vxtal_,
		     RVec<double>& Venergy_
		     ){
    
    RVec<float> energy_out;
    
    const int Vsize_ = Venergy_.size();
    for ( auto i = 0 ; i < Vsize_ ; i++ ){
      if ( Vcalo_[i] != ncalo_ ) continue;
      if ( Vxtal_[i] != nxtal_ ) continue;
      energy_out.emplace_back(Venergy_[i]);
    }
    
    return energy_out;
  };
  
  /////////////////////////////////////////////////

  if (isMC){
    cout<<"Range analyzer running on MC"<<endl;
    return rdf_.Define( "crytal_energy" , eval_mc , { "calo_Index_v2" , "xtal_Index" , "xtal_energy" } ).Histo1D(energy1, "crytal_energy");
  }
  else{
    cout<<"Range analyzer running on DATA"<<endl;
    return rdf_.Filter(Form("calo_Index_v1 == %d",ncalo_)).Define( "crytal_energy" , eval_data , { "xtal_Index" , "xtal_energy" } ).Histo1D(energy1, "crytal_energy");
  }
}

int main(int argc, char **argv) {
  
  // start time
  TStopwatch time;
  time.Start();

  TCLAP::CmdLine cmd( "range analyzer" , ' ' , "1.0" );
  TCLAP::MultiArg<std::string> dataIn_   ( "d" , "datafiles"        , "Data text file and CSV"        , true  , "string" , cmd );
  TCLAP::MultiArg<std::string> mcIn_     ( "m" , "mcfiles"          , "MC text file and CSV"          , true  , "string" , cmd );
  TCLAP::ValueArg<int>         calo_     ( "c" , "ncalo"            , "Calo number"                   , true  ,10  , "int"    , cmd );
  TCLAP::ValueArg<int>         xtal_     ( "x" , "nxtal"            , "Xtal number"                   , true  ,10   , "int"    , cmd );
  TCLAP::ValueArg<int>         lowlimit_ ( "l" , "lowerlimit"       , "Lower edge limit"              , false ,500.  , "double"    , cmd );
  TCLAP::MultiArg<double>      useQ_     ( "q" , "percentilerange"  , "Using percentile"              , false  , "double" , cmd );
  TCLAP::ValueArg<int>         core_     ( "n" , "ncore"            , "Number of core"                , false  ,3  , "int"    , cmd );
  
  cmd.parse( argc, argv );
  vector<std::string> dataFiles = dataIn_.getValue();
  vector<std::string> mcFiles = mcIn_.getValue();
  int ncalo = calo_.getValue();
  int nxtal = xtal_.getValue();
  double lowerlimit = lowlimit_.getValue();
  vector<double> useQ = useQ_.getValue();
  int ncore = core_.getValue();
  
  auto p = fs::path(dataFiles[1]);
  //cout << p.parent_path() << endl;
  
  // consume
  vector<string> dataList = getFileList(dataFiles[0]);
  vector<string> mcList = getFileList(mcFiles[0]);
  
  Dtuple dtuple   = getParam(dataFiles[1]);
  Dtuple mctuple  = getParam(mcFiles[1]);

  double data_omega  = get<0>(dtuple);
  double data_phase  = get<1>(dtuple);
  double data_tstart = get<2>(dtuple);
  double data_tend   = get<3>(dtuple);

  double mc_omega    = get<0>(mctuple);
  double mc_phase    = get<1>(mctuple);
  double mc_tstart   = get<2>(mctuple);
  double mc_tend     = get<3>(mctuple);

  if (ncore!=0){
    ROOT::EnableImplicitMT(ncore);
    ROOT::EnableThreadSafety();
  }
  else{
    cout<<"--> DisableImplicitMT"<<endl;
    ROOT::DisableImplicitMT();
  }
  
  cout << " -- reading -- " << endl;
  cout << " Data list txt      : " << dataFiles[0] << endl;
  cout << " Data condition txt : " << dataFiles[1] << endl;
  cout << " Data Omega         : " << data_omega   << endl;
  cout << " Data Phase         : " << data_phase   << endl;
  cout << " Data start time    : " << data_tstart  << endl;
  cout << " Data end time      : " << data_tend    << endl;
  cout << " -------------------- " << endl;
  cout << " MC list txt        : " << mcFiles[0]   << endl;
  cout << " MC condition txt   : " << mcFiles[1]   << endl;
  cout << " MC Omega           : " << mc_omega     << endl;
  cout << " MC Phase           : " << mc_phase     << endl;
  cout << " MC start time      : " << mc_tstart    << endl;
  cout << " MC end time        : " << mc_tend      << endl;
  cout << " -- running on -- " << endl;
  cout << " calorimeter number : " << ncalo        << endl;
  cout << " crystal number     : " << nxtal        << endl;
  cout << " lower edge limit   : " << lowerlimit   << endl;
  cout << " lower edge percent : " << useQ[0]      << endl;
  cout << " upper edge percent : " << useQ[1]      << endl;
  cout << " number of core     : " << ncore        << endl;
  
  ROOT::RDataFrame ddf( "xtal_study" , dataList );
  //ROOT::RDataFrame mdf( "xtal_study" , mcList );
  auto data_rdf = RNode(ddf);
  //auto mc_rdf = RNode(mdf); 

  // time window (FOR NOW USE DATA RANGE)
  data_rdf = data_rdf.Filter(Form("cluster_time>%f && cluster_time<%f",data_tstart,data_tend),"select time window");
  //mc_rdf   = mc_rdf.Filter(Form("cluster_time>%f && cluster_time<%f",mc_tstart,mc_tend),"select time window");
  
  auto hdata_ = plotXtal(data_rdf,ncalo,nxtal,false);
  //auto hmc_  = plotXtal(mc_rdf,ncalo,nxtal,true);

  ////////////// Analysis ////////////////////////////////
  TH1D* hdata = hdata_.GetPtr();
  //TH1D* hmc = hmc_.GetPtr();

  // lower limit ; Q1%, Q2%.
  checkrange(hdata,lowerlimit,useQ);
  
  //auto hmc   = plotXtal(mc_rdf,ncalo,nxtal,true);
  //cout<<"HERE"<<endl;
  //cout<<"hmc integral : " << hmc.GetPtr()->Integral() << endl;
  
  // action
  //xtalpair xtalhist;
  //xtalptr::iterator iptr = hmc.begin();
  //while (iptr != hmc.end()) {
  //  xtalhist[iptr->first] = iptr->second.GetPtr();
  //}
  
  //
  //fstream fout;
  //fout.open(Form("%s/range_calo%02d_%s.txt", p.parent_path().c_str(), uncalo_ , p.filename().replace_extension().c_str() ), ios::out ); //| ios::app);
  
  //xtalpair::iterator it = xtalhist.begin();
  //while (it != xtalhist.end()) {
  //  cout << "Key: " << it->first
  //	 << ", Value: " << it->second->Integral() << endl;
  //  ++it;
  //}
  
  //fout << omega << " " << phase << " " << pstart + ((ti+1)*pdiff) << " " << pstart + ((tf+1)*pdiff) << "\n" ;

  time.Stop();
  time.Print();
  
  return 0;
}
