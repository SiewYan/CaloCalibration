#include <TFile.h>
#include <TROOT.h>
#include <TH1D.h>
#include <TChain.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>
#include <TTreeReaderArray.h>
#include "ROOT/RVec.hxx"
#include <TGraph.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "TMath.h"
#include "tclap/CmdLine.h"
#include <TStopwatch.h>
#include <filesystem>
#include <tuple>
#include <map>

using namespace std;
using namespace ROOT::VecOps;
namespace fs = std::filesystem;

typedef tuple<double,double,double,double> Dtuple;
typedef tuple<pair<double,double>,pair<TH1D*,TH1D*>,TGraph*,int> results;

double twopi = 2*TMath::Pi();
//TH1DModel energy1  = TH1DModel( "xtalenergy"  , "Crystal Energy Spectrum; Crystal Energy [MeV]; Events" , 31, 0, 3100);

int nbin=31;
double xmin=0.;
double xmax=3100.;

string outdir="/home/siew/gm2/df-spectrum/data/materials/scale-opt";

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

void initReader(TChain* ch_in, const std::string& txtIn){
  
  ifstream file(txtIn);
  string fstr;
  int count=0;
  while (getline(file, fstr)) {
    if(fstr.find('#')==std::string::npos){
      ch_in->Add(fstr.c_str());
      count++;
    }
  }
  cout<<"number of file consumed : "<< count <<endl;
  
}

// v1 for data (scaler type calo number)
// v2 for mc (vector type calo number)
TH1D* getHisto_data(TTreeReader& reader, double start, double end, int ncalo_, int nxtal_){

  // in C++ ROOT, we have to predefine every branch
  reader.Restart();
  //TTreeReaderValue<unsigned int> calo_Index_v1 (reader , "calo_Index_sca" );
  TTreeReaderValue<int> calo_Index_v1 (reader , "calo_Index_sca" );
  TTreeReaderValue<double>       cluster_time  (reader , "cluster_time"  );
  TTreeReaderArray<int>          calo_Index_v2 (reader , "calo_Index_vec" );
  TTreeReaderArray<int>          xtal_Index    (reader , "xtal_Index"    );
  TTreeReaderArray<double>       xtal_energy   (reader , "xtal_energy"   ); //
  
  TH1D* hist_en = new TH1D("data_range", Form("Data Calo %i Xtal %i ; Crystal Energy [MeV]; Events", ncalo_, nxtal_),nbin,xmin,xmax);
  
  unsigned int nevents = 0;
  while (reader.Next()) {
    
    //cout<<"event number : " << nevents << endl;

    // time selection
    if ( *cluster_time < start ) continue;
    if ( *cluster_time > end ) continue;
    
    //cout<<"caloIdx_v1   : " << *calo_Index_v1 << endl;
    int ucalo_Index_v1 = (int) *calo_Index_v1;
    
    // calo number selection
    if (ucalo_Index_v1 != ncalo_) continue;
    
    int size_ = xtal_Index.GetSize();
    for (int m = 0; m < size_ ; m++) {
      int xtalIdx = xtal_Index[m];
      double energy = xtal_energy[m];
      // xtal number selection
      if (xtalIdx != nxtal_) continue;
      hist_en->Fill(energy);
    }
    
    nevents+=1;
  } // while

  return hist_en;
 
}


// v1 for data (scaler type calo number)
// v2 for mc (vector type calo number)
TH1D* getHisto_mc(TTreeReader& reader, double start, double end, int ncalo_, int nxtal_){

  // in C++ ROOT, we have to predefine every branch
  reader.Restart();
  TTreeReaderValue<int>          calo_Index_v1 (reader , "calo_Index_sca" );
  TTreeReaderValue<double>       cluster_time  (reader , "cluster_time"  );
  TTreeReaderArray<int>          calo_Index_v2 (reader , "calo_Index_vec" );
  TTreeReaderArray<int>          xtal_Index    (reader , "xtal_Index"    );
  TTreeReaderArray<double>       xtal_energy   (reader , "xtal_energy"   ); //
  
  TH1D* hist_en = new TH1D("mc_range", Form("Data Calo %i Xtal %i ; Crystal Energy [MeV]; Events", ncalo_, nxtal_),nbin,xmin,xmax);
  
  unsigned int nevents = 0;
  while (reader.Next()) {
    
    //cout<<"event number : " << nevents << endl;

    // time selection
    if ( *cluster_time < start ) continue;
    if ( *cluster_time > end ) continue;
    
    // same size with calo, xtal, and energy
    int size_ = xtal_Index.GetSize();
    for (int n = 0; n < size_ ; n++) {
      int ncalo_v2 = calo_Index_v2[n];
      int xtalIdx = xtal_Index[n];
      double energy =	xtal_energy[n];
      
      // calo number selection
      if (ncalo_v2 != ncalo_) continue;
      // xtal number selection
      if (xtalIdx != nxtal_) continue;
      
      hist_en->Fill(energy);
      
    } // n
    nevents+=1;
  } // while
  
  return hist_en;
  
}

double getMaxValue(TH1D* hist){

  //int nbin = hist->GetXaxis()->GetNbins();
  int binmax = hist->FindLastBinAbove();
  double xmax = hist->GetXaxis()->GetBinCenter(binmax);
  double halfbin = hist->GetXaxis()->GetBinCenter(1);

  return xmax+halfbin;
}

results scanScale(TTreeReader& rdata, TTreeReader& rmc, Dtuple& ddata , Dtuple& dmc , int ncalo, int nxtal, double range1, double range2,
		  vector<TH1D*>& vecHists, vector<double>& vecScale, bool isKS=true ){

  cout<<" -- Initialize scale scanner -- "<<endl;
  // unpack
  double dstart = get<2>(ddata);
  double dend   = get<3>(ddata);
 
  double mstart   = get<2>(dmc);
  double mend     = get<3>(dmc);

  vector<double> distance;
  
  for(int k=0; k<71; k++) {
    vecHists.at(k)->Reset( "ICES" );
  }
  ///

  // in C++ ROOT, we have to predefine every branch
  rdata.Restart();
  //TTreeReaderValue<unsigned int> dcalo_Index_v1 (rdata , "calo_Index_sca" ); // calo_Index_v1
  TTreeReaderValue<int> dcalo_Index_v1 (rdata , "calo_Index_sca" ); // calo_Index_v1
  TTreeReaderValue<double>       dcluster_time  (rdata , "cluster_time"   );  // cluster_time
  TTreeReaderArray<int>          dcalo_Index_v2 (rdata , "calo_Index_vec" );  // calo_Index_v2
  TTreeReaderArray<int>          dxtal_Index    (rdata , "xtal_Index"     );
  TTreeReaderArray<double>       dxtal_energy   (rdata , "xtal_energy"    );
  
  // in C++ ROOT, we have to predefine every branch
  rmc.Restart();
  TTreeReaderValue<int>          mcalo_Index_v1 (rmc , "calo_Index_sca" ); // calo_Index_v1
  TTreeReaderValue<double>       mcluster_time  (rmc , "cluster_time"  );
  TTreeReaderArray<int>          mcalo_Index_v2 (rmc , "calo_Index_vec" ); // calo_Index_v2
  TTreeReaderArray<int>          mxtal_Index    (rmc , "xtal_Index"    );
  TTreeReaderArray<double>       mxtal_energy   (rmc , "xtal_energy"   ); //xtal_energy_v2

  cout<<Form(" -- Preparing data histograms (%g < energy < %g) MeV with different scale -- ",range1,range2)<<endl;
  ////////////////////////////////////////////////////////////////////////
  // Data processing
  TH1D* data_en_cut = new TH1D(
			       Form("data_en_calo%02d_xtal%02d_%g_%g",ncalo,nxtal,range1,range2),
			       Form("Data Calo %02d xtal %02d, %g < energy < %g; Crystal Energy [MeV]; Events", ncalo,nxtal,range1,range2), nbin,xmin,xmax);
  
  unsigned int nevents = 0;
  while (rdata.Next()){
    // time selection
    if ( *dcluster_time < dstart ) continue;
    if ( *dcluster_time > dend ) continue;
    
    //cout<<"caloIdx_v1   : " << *calo_Index_v1 << endl;
    int ucalo_Index_v1 = (int) *dcalo_Index_v1;
  
    // calo number selection
    if (ucalo_Index_v1 != ncalo) continue;

    // vechist loop 
    for(int k=0; k<71; k++) {
      
      int size_ = dxtal_Index.GetSize();
      for (int m = 0 ; m < size_ ; m++){
	int xtalIdx = dxtal_Index[m];
	double energy = dxtal_energy[m];
	
	//xtal number selection
	if (xtalIdx != nxtal) continue;
	
        // fill once! for cross check
        if (k==0){
          if ( energy > range1 && energy < range2 )
            data_en_cut->Fill(energy);
        }
	
	if (vecScale.at(k)*energy > range1 && vecScale.at(k)*energy < range2 )
	  vecHists.at(k)->Fill(vecScale.at(k)*energy);
	
      } // xtal loop
    } // vector loop
    
    nevents+=1;
  } // while
  cout<<Form(" -- Preparing mc histogram (%g < energy < %g) MeV -- ",range1,range2)<<endl;
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  // MC processing (the Template)
  TH1D* mc_en_cut = new TH1D(
			     Form("mc_en_calo%02d_xtal%02d_%g_%g",ncalo,nxtal,range1,range2),
			     Form("MC Calo %02d xtal %02d, %g < energy < %g; Crystal Energy [MeV]; Events", ncalo,nxtal,range1,range2), nbin,xmin,xmax);
  
  nevents = 0;
  while (rmc.Next()) {
    
    //cout<<"event number : " << nevents << endl;
    // time selection
    if ( *mcluster_time < mstart ) continue;
    if ( *mcluster_time > mend ) continue;
    
    // same size with calo, xtal, and energy
    int msize_ = mxtal_Index.GetSize();
    for (int n = 0; n < msize_ ; n++) {
      int ncalo_v2 = mcalo_Index_v2[n];
      int xtalIdx = mxtal_Index[n];
      double energy = mxtal_energy[n];
      
      // calo number selection
      if (ncalo_v2 != ncalo) continue;
      // xtal number selection
      if (xtalIdx != nxtal) continue;

      if (energy > range1 && energy < range2)
	mc_en_cut->Fill(energy);
    } // n
    nevents+=1;
  } // while

  cout<<" -- Scanning scale -- "<<endl;
  /////////////////////////////////////////////////////////////////////////
  //////// Scale scanning
  /////////////////////////////////////////////////////////////////////////
  for (int k=0 ; k<71; k++){
    vecHists.at(k)->ClearUnderflowAndOverflow();
    // Kolmogorov-Smirnov or Anderson-Darling test
    double f0 = (isKS) ?
      vecHists.at(k)->KolmogorovTest(mc_en_cut,"M")
      : vecHists.at(k)->AndersonDarlingTest(mc_en_cut, "T");
    
    distance.push_back(f0);
  }
  
  /// check the performance of the optimization
  double sc[71], dist[71];
  for (int m=0 ; m<71; m++){
    sc[m] = vecScale.at(m);
    dist[m] = distance.at(m);
  }
  
  TGraph* grDist = new TGraph(71,sc,dist);
  grDist->SetName(Form("gr_calo%02d_xtal%02d_range_%g_%g",ncalo,nxtal,range1,range2));
  grDist->SetTitle(Form("Optimized KS Distance (calo %02d, xtal %02d) in range %g < energy < %g ;SCALE ; KS Distance",ncalo,nxtal,range1,range2));

  // compare mimimum of vecKS with iterKS
  int minIndexVec = min_element(distance.begin(),distance.end()) - distance.begin();
  double optScale = vecScale.at(minIndexVec); // scale
  double minVecDist = *min_element(distance.begin(), distance.end()); // min distance

  // output:
  results out= make_tuple( make_pair(optScale,minVecDist) , make_pair(data_en_cut,mc_en_cut) , grDist , minIndexVec );

  return out;
  
}

void save(vector<TH1D*> hist, vector<TGraph*> gr, string& name){

  TFile *fout = TFile::Open( name.c_str() , "RECREATE" );

  for (auto &ihist : hist)
    ihist->Write();

  for (auto &igr : gr)
    igr->Write();
  
  fout->Close();
  
}

void scanRange(TTreeReader& rdata, TTreeReader& rmc, Dtuple& ddata , Dtuple& dmc , int ncalo, int nxtal, string& name, bool isKS=true ){
  
  // reading data time
  double data_tstart = get<2>(ddata);
  double data_tend   = get<3>(ddata);

  // reading mc time
  double mc_tstart   = get<2>(dmc);
  double mc_tend     = get<3>(dmc);

  cout << " -- dataset configuration -- " << endl;
  cout << " Data start time    : " << data_tstart  << endl;
  cout << " Data end time      : " << data_tend    << endl;
  cout << " -------------------- " << endl;
  cout << " MC start time      : " << mc_tstart    << endl;
  cout << " MC end time        : " << mc_tend      << endl;

  //
  // 2. pass window selection (DATA and MC)
  // 3. scan on lower and upper range : pick i lower and j upper then get the ks-scale (from 0.5 - 1.5)
  // 4. make the text output for calo and xtal.
  //
  
  // get the histograms
  cout<<" -- Identify Histogram Range -- "<<endl;
  //TH1D* hist_data = getHisto_data( rdata , data_tstart , data_tend , ncalo , nxtal );
  TH1D* hist_mc   = getHisto_mc  ( rmc   , mc_tstart   , mc_tend   , ncalo , nxtal );
  
  //double endpoint1 = getMaxValue(hist_data);
  double endpoint = getMaxValue(hist_mc);
  //double endpoint  = (endpoint1 > endpoint2) ? endpoint2 : endpoint1 ;
  cout<<" maximum range of MC used in the scan : "<< endpoint << " MeV"<< endl;

  // scale vector
  cout<<" -- Initialize scale scanning -- "<<endl;
  vector<TH1D*> vecHists;
  vector<double> vecScale;
  // scale 0.5 - 1.50 scaling factors in step of 0.01 scale prober
  // scale 0.5 - 1.20 scaling factors in step of 0.01 scale prober
  for(int k=0; k<71; k++) {
    double scale = 0.7 + (k-20)*0.01;
    vecHists.push_back(new TH1D(Form("Scan_clean%i",k), Form("Data scaled event-by-event with scale %f", scale ) , nbin, xmin, xmax));
    vecScale.push_back(scale);
  }

  auto p = fs::path(name);
  fstream fout;
  fout.open( Form( "%s/scale_calo%02d-xtal%02d_%s.txt" , outdir.c_str() , ncalo , nxtal , p.filename().replace_extension().c_str() ), ios::out );
  cout<<"save text file : "<< Form( "%s/scale_calo%02d-xtal%02d_%s.txt" , outdir.c_str() , ncalo , nxtal , p.filename().replace_extension().c_str() ) << endl;
  
  //fout.open(Form("%s/scale_calo%02d-xtal%02d_%s.txt", p.parent_path().c_str(), ncalo, nxtal , p.filename().replace_extension().c_str() ), ios::out ); //| ios::app);
  //cout<<"save text file : "<< Form( "%s/scale_calo%02d-xtal%02d_%s.txt", p.parent_path().c_str(), ncalo, nxtal , p.filename().replace_extension().c_str() ) << endl;
  
  vector<TH1D*> saveHisto;
  vector<TGraph*> saveGraph;
    
  // lower edge
  int counter=0;
  cout<<" -- Running range scanner -- "<<endl;
  double vlow = 1.;
  for (int i = 1 ; i < 7 ; i++){
    double ilow = i - (vlow);
    // upper edge
    double vhigh = 1.;
    for (int j = 1 ; j < 7 ; j++){
      double jup = j - (vhigh);
      
      //if (ilow<jup && endpoint*ilow < 400){
      if (ilow<jup){
	//cout<<"i : "<< ilow*100 << " % ; j : " << jup*100 <<" %"<<endl;
	//cout<<"i : "<< endpoint*ilow << " MeV ; j : "<< endpoint*jup <<" MeV"<< endl;
	
	results result = scanScale(rdata, rmc, ddata , dmc , ncalo, nxtal, endpoint*ilow, endpoint*jup, vecHists, vecScale, isKS );
	saveHisto.emplace_back(get<1>(result).first);
	saveHisto.emplace_back(get<1>(result).second);
	saveGraph.emplace_back(get<2>(result));

	cout<<"lower edge ("<<ilow*100<<" %) : "<< endpoint*ilow << " MeV ; upper edge ("<<jup*100<<" %) : "<< endpoint*jup <<" MeV ; optimized scale : "<< get<0>(result).first << endl;
	
	// ncalo ; nxtal ; low-percent ; up-percent ; max-range ; opt-scale ; opt-distance 
	fout << ncalo <<" "<< nxtal <<" "<< ilow << " " << jup << " "<< endpoint << " " << get<0>(result).first <<" "<< get<0>(result).second <<endl;
	counter++;
	//if (counter==2) break;
      }
      vhigh+=0.8;
    } // j
    //if (counter==2) break;
    vlow+=0.8;
  } // i
  
  cout<<" -- Finalizing results -- "<<endl;
  fout.close();
  
  string out_name = Form( "%s/save_calo%02d-xtal%02d_%s.root" , outdir.c_str() , ncalo , nxtal , p.filename().replace_extension().c_str() );
  cout<<"save root file : "<< out_name << endl;
  save( saveHisto, saveGraph, out_name );
  
}

int main(int argc, char **argv) {

  gROOT->SetBatch(true);
  
  // start time
  TStopwatch timer;
  timer.Start();

  TCLAP::CmdLine cmd( "range analyzer" , ' ' , "1.0" );
  TCLAP::MultiArg<std::string> dataIn_   ( "d" , "datafiles"        , "Data text file and CSV"        , true  , "string" , cmd );
  TCLAP::MultiArg<std::string> mcIn_     ( "m" , "mcfiles"          , "MC text file and CSV"          , true  , "string" , cmd );
  TCLAP::ValueArg<int>         calo_     ( "c" , "ncalo"            , "Calo number"                   , true  ,10  , "int"    , cmd );
  TCLAP::ValueArg<int>         xtal_     ( "x" , "nxtal"            , "Xtal number"                   , true  ,10   , "int"    , cmd );
  
  cmd.parse( argc, argv );
  vector<std::string> dataFiles = dataIn_.getValue();
  vector<std::string> mcFiles = mcIn_.getValue();
  int ncalo = calo_.getValue();
  int nxtal = xtal_.getValue();
  
  auto p = fs::path(dataFiles[1]);
  //cout << p.parent_path() << endl;
  
  // consume
  TChain *ch_data = new TChain("xtal_study");
  TChain *ch_mc   = new TChain("xtal_study");

  initReader(ch_data,dataFiles[0]);
  initReader(ch_mc,mcFiles[0]);
  
  TTreeReader data(ch_data);
  TTreeReader mc(ch_mc);
  
  Dtuple dtuple   = getParam(dataFiles[1]);
  Dtuple mctuple  = getParam(mcFiles[1]);
  
  cout << " -- reading text files -- " << endl;
  cout << " Data list txt      : " << dataFiles[0] << endl;
  cout << " Data condition txt : " << dataFiles[1] << endl;
  cout << " MC list txt        : " << mcFiles[0]   << endl;
  cout << " MC condition txt   : " << mcFiles[1]   << endl;
  cout << " -- running on -- " << endl;
  cout << " calorimeter number : " << ncalo        << endl;
  cout << " crystal number     : " << nxtal        << endl;

  // 1. revert to C++ loop function
  // 2. pass window selection (DATA and MC)
  // 3. scan on lower and upper range : pick i lower and j upper then get the ks-scale (from 0.5 - 1.5)
  // 4. make the text output for calo and xtal.
  // 5. plot 2D histograms

  cout<<" -- Done Initialization -- "<<endl;
  scanRange( data, mc, dtuple, mctuple, ncalo, nxtal, dataFiles[0], true );
  cout<<" -- DONE -- "<<endl;
  timer.Stop();
  timer.Print();
  
  return 0;
}
