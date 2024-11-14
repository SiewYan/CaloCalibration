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
#include "TRandom3.h"

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
typedef tuple<double,TH1D*> result;

// 
double twopi = 2*TMath::Pi();
int nbin=31;
double xmin=0.;
double xmax=3100.;
int counter=0;


// Parameter for simultated annealing
std::pair<double,double> bound = {0.5,1.5};
double scale_knot = 1.0;
double initialTemp = 100.;
double minTemp = 1.;
double coolingRate = 0.9;
double initialstepSize = 0.1;
double minStepSize = 0.01;    // A minimum step size for precision
int maxIterations = 20;
int greedyIterations = 10;
double improvementThreshold = 1e-4;
int noImprovementCount = 1;
int maxNoImprovementIter = 3;

//
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
TH1D* getHisto(TTreeReader& reader, double start, double end, int ncalo_, int nxtal_, bool isData, double scale=0.){

  // HARD CODED RANGE
  //double range1=500.;
  double range1=440.;
  //double range2=3200.;

  // in C++ ROOT, we have to predefine every branch
  reader.Restart();
  TTreeReaderValue<int>          calo_Index_v1 (reader , "calo_Index_sca" );
  TTreeReaderValue<double>       cluster_time  (reader , "cluster_time"  );
  TTreeReaderArray<int>          calo_Index_v2 (reader , "calo_Index_vec" );
  TTreeReaderArray<int>          xtal_Index    (reader , "xtal_Index"    );
  TTreeReaderArray<double>       xtal_energy   (reader , "xtal_energy"   ); //

  std::string scale_str = std::to_string(scale);
  std::replace(scale_str.begin(), scale_str.end(), '.', 'p');

  if (isData) counter++;
  
  TH1D* hist_en = (!isData) ?
    new TH1D( "mc"   , Form( "MC Calo %i Xtal %i ; Crystal Energy [MeV]; Events", ncalo_, nxtal_),nbin,xmin,xmax) :
    new TH1D( Form("data_%i",counter) , Form( "Data Calo %i Xtal %i scale %s ; Crystal Energy [MeV]; Events", ncalo_, nxtal_, scale_str.c_str()),nbin,xmin,xmax);
  ;
  
  unsigned int nevents = 0;
  while (reader.Next()) {
    
    // time selection
    if ( *cluster_time < start ) continue;
    if ( *cluster_time > end ) continue;
    
    int size_ = xtal_Index.GetSize();
    
    // calo number selection
    int ucalo_Index_v1 = (int) *calo_Index_v1;
    if ( isData && ucalo_Index_v1 != ncalo_) continue;
    
    for (int m = 0; m < size_ ; m++) {
      
      int ncalo_v2 = calo_Index_v2[m];
      int xtalIdx = xtal_Index[m];

      double energy = xtal_energy[m];
      
      // calo number selection
      if ( !isData && ncalo_v2 != ncalo_) continue;
      
      // xtal number selection
      if ( xtalIdx != nxtal_ ) continue;
      
      if (scale!=0.){
	//if (energy*scale > range1 && energy*scale < range2 )
	if (energy*scale > range1)
	  hist_en->Fill(energy*scale);
      }
      else {
	//if (energy > range1 && energy < range2)
	if (energy > range1){
	  hist_en->Fill(energy);
	}
      }
      
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

void save(vector<TH1D*> hist, string& name){

  TFile *fout = TFile::Open( name.c_str() , "RECREATE" );

  for (auto &ihist : hist)
    ihist->Write();
  
  fout->Close();
}

result calculateKSStatistic(TH1* h1, TTreeReader& rdata, double data_tstart , double data_tend, int ncalo, int nxtal, double scaleFactor) {
 
  TH1D* h2_scaled = getHisto ( rdata , data_tstart   , data_tend   , ncalo , nxtal , true , scaleFactor );
  h2_scaled->ClearUnderflowAndOverflow();
  
  // Compute the Kolmogorov-Smirnov statistic between h1 and the scaled h2
  double ksStatistic = h1->KolmogorovTest(h2_scaled, "M");

  result results{ksStatistic,h2_scaled};
  return results;
}


void annealing(TTreeReader& rdata, TTreeReader& rmc, Dtuple& ddata , Dtuple& dmc , int ncalo, int nxtal, string& name ){
  
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
  
  // get the histograms
  vector<TH1D*> saveHisto;
  TH1D* hist_mc   = getHisto ( rmc   , mc_tstart   , mc_tend   , ncalo , nxtal , false );
  saveHisto.push_back(hist_mc);
  double endpoint = getMaxValue(hist_mc);
  cout<<" maximum range of MC used in the scan : "<< endpoint << " MeV"<< endl;
  
  // scale vector
  cout<<" -- Initialize annealing -- "<<endl;
  
  result results = calculateKSStatistic(hist_mc, rdata, data_tstart, data_tend, ncalo, nxtal, scale_knot);
  double KS_knot = get<0>(results);
  saveHisto.emplace_back(get<1>(results));
  
  TRandom3 randGen(0);
  
  //
  auto p = fs::path(name);
  fstream fout_anneal;
  fout_anneal.open( Form( "%s/annealing_calo%02d-xtal%02d_%s.txt" , outdir.c_str() , ncalo , nxtal , p.filename().replace_extension().c_str() ), ios::out );
  
  double temperature = initialTemp;
  double scale_opt = 0.;
  double KS_opt = scale_knot;
  fout_anneal << scale_knot << " " << KS_knot << " " << temperature << endl;

  for (int iteration = 1 ; iteration <= maxIterations ; ++iteration) {

    // Adjust step size according to the temperature
    double stepSize = initialstepSize * (temperature / initialTemp);
    if (stepSize < minStepSize) stepSize = minStepSize;  // Prevent step size from becoming too small
    
    // Perturb the scale factor randomly using the dynamic step size
    double scale_new = scale_knot + randGen.Uniform(-stepSize, stepSize);    
    
    // Perturb the scale factor randomly
    //double scale_new = scale_knot + randGen.Uniform(-0.1, 0.1);  // Random step within [-0.1, 0.1]
    
    if ( scale_new <= bound.first || scale_new >= bound.second ) continue;
    
    // Calculate KS statistic for the new solution
    results = calculateKSStatistic(hist_mc, rdata, data_tstart, data_tend, ncalo, nxtal, scale_new);
    double KS_new = get<0>(results);
    saveHisto.emplace_back(get<1>(results));
    
    // annealing probability
    if ( KS_new < KS_knot || randGen.Uniform() < TMath::Exp(-(KS_new - KS_knot) / temperature)) {
      
      scale_knot = scale_new;
      KS_knot = KS_new;
      
      // Update the best solution found
      if (KS_new < KS_opt) {
	scale_opt = scale_new;
	KS_opt = KS_new;
	noImprovementCount = 0; // Reset the counter if a better KS value is found
      }
      else {
	// Check if the improvement is less than the threshold
	if (fabs(KS_new - KS_opt) < improvementThreshold) {
	  noImprovementCount++;
	} else {
	  noImprovementCount = 0;  // Reset if significant improvement happens
	}
	
	if (noImprovementCount > maxNoImprovementIter) {
	  std::cout << "No significant improvement in last " << maxNoImprovementIter
		    << " iterations. Stopping early...\n";
	  break;
	}
      }
    }
    
    
    // Cool down the system
    temperature *= coolingRate;
    
    cout << "Iteration : " << iteration << " ; Scale factor : " << scale_knot
	 << " ; KS : " << KS_knot << " ; Temp : " << temperature << std::endl;
    
    fout_anneal << scale_knot << " " << KS_knot << " " << temperature << endl;
    
    if (temperature < minTemp) break;
  } // end of interation
  fout_anneal << scale_opt << " " << KS_opt << " " << temperature << endl;
  fout_anneal.close();
  
  cout<<"scale_best : " << scale_opt << " ; correspond to this KS : " << KS_opt << endl;
  
  fstream fout;
  fout.open( Form( "%s/scale_calo%02d-xtal%02d_%s.txt" , outdir.c_str() , ncalo , nxtal , p.filename().replace_extension().c_str() ), ios::out );
  fout << ncalo << " " << nxtal << " " << scale_knot << " " << KS_knot << endl;
  fout.close();
  
  string out_name = Form( "%s/save_calo%02d-xtal%02d_%s.root" , outdir.c_str() , ncalo , nxtal , p.filename().replace_extension().c_str() );
  cout<<"save root file : "<< out_name << endl;
  
  save( saveHisto, out_name );
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
  annealing( data, mc, dtuple, mctuple, ncalo, nxtal, dataFiles[0] );
  cout<<" -- DONE -- "<<endl;
  timer.Stop();
  timer.Print();
  
  return 0;
}
