#include "blinder/Blinders.hh"
#include "TMath.h"
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"
#include "TF1.h"
#include "TChain.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"
#include "ROOT/RVec.hxx"
#include "ROOT/RSnapshotOptions.hxx"
#include "TFitResult.h" 

#include <iostream>
#include <string>
#include <filesystem>
#include <string>

#include "tclap/CmdLine.h"
#include "misc/string_io.h"

using namespace std;
using namespace ROOT::RDF;
namespace fs = std::filesystem;

blinding::Blinders::fitType ftype = blinding::Blinders::kOmega_a;
blinding::Blinders getBlinded( ftype, "Calo acceptance study" );

double twopi = 2*TMath::Pi();

//
ROOT::Double_v blinded_5params(ROOT::Double_v *x, double *p){

    double norm = p[0];
    double life = p[1];
    double asym = p[2];
    double R = p[3];
    double phi = p[4];
    double time = x[0];

    double omega = getBlinded.paramToFreq(R);
    return norm*TMath::Exp(-time/life)*(1 + asym*TMath::Cos(omega*time - phi));
}

//
ROOT::Double_v blinded_9params(ROOT::Double_v *x, double *p){

  double time = x[0];

  // wiggle
  double norm = p[0]; // N
  double life = p[1]; // tau
  double asym = p[2]; // A
  double R = p[3];    // R
  double omega = getBlinded.paramToFreq(R);
  double phi = p[4];  // phi                                                                                                                                                                  
  //double wiggle = norm*TMath::Exp(-time/life)*(1 - asym*TMath::Cos(omega*time+phi));
  double wiggle = norm*TMath::Exp(-time/life)*(1 + asym*TMath::Cos(omega*time - phi));

  // cbo
  double tau_cbo = p[5]; // taucbo
  double A_cbo = p[6];   // Acbo
  double omega_cbo = p[7];
  double phi_cbo = p[8];
  double cbo = 1 - TMath::Exp(-time/tau_cbo)*A_cbo*TMath::Cos(omega_cbo*time+phi_cbo);

  return wiggle * cbo;
}

string pct(double abval, double eval){
  return Form( "%.3f" , abs(eval/abval*100));
}

int main(int argc, char **argv){

  // start time
  TStopwatch time;
  time.Start();

  TCLAP::CmdLine cmd( "fitting" , ' ' , "1.0" );
  TCLAP::ValueArg<std::string> inFile_   ( "f" , "inFile"    , "Root file in"            , true  , "dummy"           , "string" , cmd );
  TCLAP::ValueArg<std::string> outPath_  ( "o" , "outPath"   , "Root file out path"      , true  , "./"              , "string" , cmd );
  TCLAP::ValueArg<std::string> fitModel_ ( "m" , "fitModel"  , "Fit model used"          , true  , "blinded_5params" , "string" , cmd );
  TCLAP::ValueArg<double>      enThres_  ( "e" , "enThres"   , "Energy threshold"        , false , 1700.             , "double" , cmd );
  TCLAP::MultiArg<double>      pList_    ( "p" , "pList"     , "Parameter list"          , true  , "double"          , cmd );
  TCLAP::MultiArg<double>      range_    ( "r" , "range"     , "fit range start and end" , false , "double"          , cmd );
  TCLAP::SwitchArg             isMC_     ( "c" , "isMC"      , "isMC or isData"          , cmd   , false );
  TCLAP::ValueArg<int>         n_        ( "n" , "n"         , "Number of core used"       , false  , 5   , "int" , cmd );
  
  cmd.parse( argc, argv );

  std::string inFile = inFile_.getValue();
  std::string outPath = outPath_.getValue();
  std::string fitModel = fitModel_.getValue();
  double enThres = enThres_.getValue();
  std::vector<double> pList = pList_.getValue();
  std::vector<double> range = range_.getValue();
  bool isMC = isMC_.getValue(); // 1 isMC ; 0 isData
  int ncore = n_.getValue();
  string outpath = (contain(outPath,".root")) ? outPath : Form( "%s/fit_%s.root" , outPath.c_str(), fs::path(inFile).stem().c_str() );
  
  // check argument                                                                                                                                                                           
  std::cout<<"inFile   : "<< inFile <<std::endl;
  std::cout<<"outPath  : "<< outPath <<std::endl;
  std::cout<<"fitModel : "<< fitModel <<std::endl;
  std::cout<<"enThres  : "<< enThres <<std::endl;
  //for (auto i : pList )   std::cout<<" pList : "<< i <<std::endl;
  std::cout<<"ncore    : "<< ncore <<std::endl;   
  std::cout<<"isMC     : "<< isMC <<std::endl;
  
  // perform check
  // parameter check
  if ( (fitModel == "blinded_5params") && (pList.size() != 5) ){
    std::cout<<"Parameters given are not in total of 5, aborting!!!"<<std::endl;
    return 0;
  }
  if ( (fitModel == "blinded_9params") && (pList.size() != 9) ){
    std::cout<<"Parameters given are not in total of 9, aborting!!!"<<std::endl;
    return 0;
  }
  
  // filelist
  if (!(contain(inFile,".txt"))){
    cout<<"Input must be in txt"<<endl;
    return 0;
  }

  // start working
  ROOT::EnableImplicitMT(ncore);
  ROOT::EnableThreadSafety();

  vector<string> flist;
  ifstream file(inFile);
  string fstr;
  while (getline(file, fstr)) { flist.push_back(fstr); }

  ROOT::RDataFrame df( "xtal_study" , flist );
  auto rdf = RNode(df);

  // define derived variable
  rdf = rdf.Define("twoPi" , to_string(twopi) );
  rdf = rdf.Define("Time"      , "cluster_time" );
  rdf = rdf.Define("Energy"    , "cluster_energy" );

  // switches
  rdf = rdf.Define("isAboveE_thres", (isMC) ? "Energy>1600" : "Energy>1700" );
  if (!isMC) rdf = rdf.Define("upperBound","Time<600 && Energy<8000").Filter("upperBound");
  
  // plot
  auto wiggle = rdf.Filter("isAboveE_thres").Histo1D( { "wiggle" , "wiggle;time [#mus];N" , 4000, 0, 0.1492*4000 } , "Time" );

  TFile *fout = TFile::Open( outpath.c_str() , "RECREATE");

  TH1D* hwiggle = (TH1D*) wiggle.GetPtr();

  cout<<"Initializing fit"<<endl;
  
  TF1 *fit_func;
  // default range
  double range0= (range.size()==0) ? 30 : range[0] ;
  double range1= (range.size()==0) ? 500 : range[1];
  
  if ( fitModel == "blinded_5params"){
    fit_func = new TF1("fit_func", blinded_5params, range0,range1, pList.size());

    fit_func->SetParNames("N","#tau","A","R","#phi");
    fit_func->SetParameters(pList[0], pList[1], pList[2], pList[3], pList[4]); // 2000 , 64.4 , 0.4 , 0 , 1
    
    std::cout<<std::endl;
    std::cout<<"Initial Parameters"<<std::endl;
    std::cout<<"Range     : ["<< range0 << " , " << range1 << "]"<<std::endl;
    std::cout<<"N         : "<< pList[0] <<std::endl;
    std::cout<<"tau       : "<< pList[1] <<std::endl;
    std::cout<<"A         : "<< pList[2] <<std::endl;
    std::cout<<"R         : "<< pList[3] <<std::endl;
    std::cout<<"phi       : "<< pList[4] <<std::endl;
    std::cout<<std::endl;

    // nominal constrans                                                                                                                                                    
    fit_func->SetParLimits(1,64,65);
    fit_func->SetParLimits(2,0.3,0.45);
    fit_func->SetParLimits(4,3.14,6.29);

  }
  else if ( fitModel == "blinded_9params" ){
    fit_func = new TF1("fit_func", blinded_9params, range0,range1, pList.size());
    
    fit_func->SetParNames("N","#tau","A","R","#phi","#tau_{cbo}","A_{cbo}","#omega_{cbo}","#phi_{cbo}");
    fit_func->SetParameters(pList[0], pList[1], pList[2], pList[3], pList[4], pList[5], pList[6], pList[7], pList[8]);
    std::cout<<std::endl;
    std::cout<<"Initial Parameters"<<std::endl;
    std::cout<<"Range     : ["<< range0 << " , " << range1 << "]"<<std::endl;
    std::cout<<"N         : "<< pList[0] <<std::endl;
    std::cout<<"tau       : "<< pList[1] <<std::endl;
    std::cout<<"A         : "<< pList[2] <<std::endl;
    std::cout<<"R         : "<< pList[3] <<std::endl;
    std::cout<<"phi       : "<< pList[4] <<std::endl;
    std::cout<<"tau_cbo   : "<< pList[5] <<std::endl;
    std::cout<<"A_cbo     : "<< pList[6] <<std::endl;
    std::cout<<"omega_cbo : "<< pList[7] <<std::endl;
    std::cout<<"phi_cbo   : "<< pList[8] <<std::endl;
    std::cout<<std::endl;

    // nominal constrains                                                                                                                                                                   
    fit_func->SetParLimits(1,64,65);     // tau
    fit_func->SetParLimits(2,0.3,0.45);  // A
    fit_func->SetParLimits(4,3.14,6.29); // phi
    fit_func->SetParLimits(5,120,170);   // tau_cbo
    fit_func->SetParLimits(6,-0.1,0.1);  // A_cbo
    fit_func->SetParLimits(7,2.25,2.38); // omega_cbo                                                                                                                                 
    fit_func->SetParLimits(8,17.2,25.5); // phi_cbo added
    
  }
  else{
    std::cout<<"Error, not fit model found"<<std::endl;
    return 0;
  }

  fit_func->SetLineColor(2);
  fit_func->SetNpx(1000);
  //int fitStatus = hwiggle->Fit(fit_func,"REMLMULTITHREAD");
  TFitResultPtr fit_res = hwiggle->Fit(fit_func,"REMLSMULTITHREAD");
  int fitStatus = fit_res;

  fit_res->SetName(Form("fit_%s",fitModel.c_str()));
  fit_func->Write();
  fit_res->Write();
  hwiggle->Write();
  fout->Close();

  //
  // fit performance
  double chi2 = fit_func->GetChisquare();
  double dof = fit_func->GetNDF();
  
  double omegaR = getBlinded.paramToFreq(fit_func->GetParameter(3));
  double pstart = fit_func->GetParameter(4) / omegaR;
  double period1 = (twopi + fit_func->GetParameter(4)) / omegaR;
  double period2 = (2*twopi + fit_func->GetParameter(4)) / omegaR;
  double period3 = (3*twopi + fit_func->GetParameter(4)) / omegaR;
  double nperiod = fmod(100*omegaR - fit_func->GetParameter(4),twopi);
  
  std::ofstream out( Form( "%s/fit_%s_chi_%f.txt" , outPath.c_str(), fs::path(inFile).stem().c_str() , (chi2/dof) ) );
  
  // usual
  out << "Fit status : " << fitStatus <<endl;
  out << "Fit valid  : " << fit_res->IsValid() <<endl;
  out << "chi2       : " << Form( "%.3f" , chi2 )                      << " ; dof        : "<< Form( "%.3f" , dof )                      << " ; chi2/dof  : "<< Form( "%.3f" , chi2 / dof ) << endl;
  out << "Prob       : " << fit_func->GetProb() <<endl;
  out << "nEvent     : " << hwiggle->GetEntries() <<endl;
  out << "n          : " << fit_func->GetParameter(0) << " +/- "<< fit_func->GetParError(0) << " ("<< pct( fit_func->GetParameter(0) , fit_func->GetParError(0) ) << " %)" <<endl;
  out << "tau        : " << fit_func->GetParameter(1) << " +/- "<< fit_func->GetParError(1) << " ("<< pct( fit_func->GetParameter(1) , fit_func->GetParError(1) ) << " %)" <<endl;
  out << "Asym       : " << fit_func->GetParameter(2) << " +/- "<< fit_func->GetParError(2) << " ("<< pct( fit_func->GetParameter(2) , fit_func->GetParError(2) ) << " %)" <<endl;
  out << "R          : " << fit_func->GetParameter(3) << " +/- "<< fit_func->GetParError(3) << " ("<< pct( fit_func->GetParameter(3) , fit_func->GetParError(3) ) << " %)" <<endl;
  out << "Phase      : " << fit_func->GetParameter(4) << " +/- "<< fit_func->GetParError(4) << " ("<< pct( fit_func->GetParameter(4) , fit_func->GetParError(4) ) << " %)" <<endl;
  if (fitModel == "blinded_9params"){
    out << "tau_cbo    : " << fit_func->GetParameter(5) << " +/- "<< fit_func->GetParError(5) << " ("<< pct( fit_func->GetParameter(5) , fit_func->GetParError(5) ) << " %)" <<endl;
    out << "A_cbo      : " << fit_func->GetParameter(6) << " +/- "<< fit_func->GetParError(6) << " ("<< pct( fit_func->GetParameter(6) , fit_func->GetParError(6) ) << " %)" <<endl;
    out << "w_cbo      : " << fit_func->GetParameter(7) << " +/- "<< fit_func->GetParError(7) << " ("<< pct( fit_func->GetParameter(7) , fit_func->GetParError(7) ) << " %)" <<endl;
    out << "Phase_cbo  : " << fit_func->GetParameter(8) << " +/- "<< fit_func->GetParError(8) << " ("<< pct( fit_func->GetParameter(8) , fit_func->GetParError(8) ) << " %)" <<endl;
  }
  out << "omega(R)               : " << omegaR                    <<endl;
  out << "period start           : " << pstart  << " us" <<endl;
  out << "first period elapased  : " << period1  << " us ; delta(t) : "<< period1 - pstart <<endl;
  out << "second period elapased : " << period2  << " us ; delta(t) : "<< period2 - period1 <<endl;
  out << "third period elapased  : " << period3  << " us ; delta(t) : "<< period3 - period2 <<endl;
  out << "nperiod                : " << nperiod << endl;         
  
  out.close();

  time.Stop();
  time.Print();

  return 0;
}
