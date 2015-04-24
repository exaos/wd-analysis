
//==================================================

// check the best PSD resolution range for Pu-C
{
  gROOT->LoadMacro("digipulse.c+g");
  // gROOT->LoadMacro("libdigipulse.so");
  gROOT->LoadMacro("wd_calc.cpp");
  
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
  para_p.bSmooth = false;  // wheather smooth data
  para_p.fp_smth = NULL;   // data smooth function:
                           //   bool f(double **, const int *, const int *)
  para_p.fp_ocut = NULL;   // cut function for original data:
                           //   bool f(int *, const uint32_t *)
  para_p.fp_ucut = NULL;   // cut function for unified data:
                           //   bool f(int *, const int *)

  Bool_t  bExtP = false;
  
  // PSD parameters
  
  Bool_t   bPSD = true;
  ParaPSD_QR_t  para_psd;
  
  para_psd.fT1 = -10;   // 10 ns before the peak
  para_psd.fT2 = -10;   // usually = fT1
  para_psd.fT4 =  100;  // 100 ns after the peak
  para_psd.fPeak = 0;   // fPeak should be set during calculation

  // output file
  
  const char *fout = "test/BC501-PuC-ranges.root";

  // Tree
  
  TChain *T = new TChain("wdPulse");
  T->Add("test/wd-BC501-PuC.root");

  // range 1
  char t_id[32];
  char t_name[64];
  for(int i=5; i<25; i+=2) {
    para_psd.fT3 = i;
    sprintf(t_id,   "BC501_PuC_%d", i);
    sprintf(t_name, "pq_BC501_PuC_%d", i);
    wd_calc( T, &para_p, fout, bExtP, bPSD, &para_psd,
             t_id, t_name, "Pulse Quantities of BC501 (Pu-C)");
  }
}

//============================================================
