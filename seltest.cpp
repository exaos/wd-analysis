
void seltest(const char *fn)
{
  gROOT->LoadMacro("digipulse.c+g");
  gROOT->LoadMacro("wdpsel.C+g");

  wdpsel *wd = new wdpsel();
  
  TFile *f = new TFile(fn);
  TTree *T = (TTree*)f->Get("wdPulse");
  
  T->Process(wd);

  f->Close();
}

