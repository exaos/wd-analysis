
// Failed to run using CINT!!
// seems to be a bug in CINT!
{
  gROOT->LoadMacro("digipulse.c+g");
  gROOT->LoadMacro("psd_test_PuC.C");

    
  // wave parameters
  
  ParaPulse_t para_p;
  para_p.polar     = p_m;      // polarity: negative
  para_p.bAutoBase = true;     // automaticly search the baseline
  para_p.fBase     = 1024;     // default baseline
  para_p.fBinResolution = 1;   // bin resolution: 1 ns
  para_p.fVResolution   = 1;   // volt resolution: 1 mV
  para_p.fSwing         = 3;   // wave fluctuation: ~ 3mV
  para_p.fTrigger       = 0.4; // trigger position: 40%
  para_p.fThreshold     = 10;  // threshold: 10 mV
  para_p.fDCoffset      = 0.;  // DC offset (unit: mV) -- used with baseline
  para_p.bGate       = true;   // software gate
  para_p.fGate.start = 50;     // soft-gate: start
  para_p.fGate.end   = 350;    // soft-gate: end
  // para_p.bSmooth = false;  // wheather smooth data
  // para_p.fp_smth = NULL;   // data smooth function:
                           //   bool f(double **, const int *, const int *)
  para_p.fp_ocut = NULL;   // cut function for original data:
                           //   bool f(int *, const uint32_t *)
  para_p.fp_ucut = NULL;   // cut function for unified data:
                           //   bool f(int *, const int *)

  TChain *T = new TChain("wdPulse");
  T->Add("test/wd-BC501-PuC.root");

  psd_test_PuC(T, &para_p);
}

