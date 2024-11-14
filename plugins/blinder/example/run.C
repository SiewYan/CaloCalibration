void run(){

  gSystem->Load("../libBlinders.so");
  gROOT->ProcessLine(".include ../rlib/include/");
  gROOT->ProcessLine(".x fitWithBlinders.C");
}
