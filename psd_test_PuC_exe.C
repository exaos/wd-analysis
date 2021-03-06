
// Failed to run using CINT!!
// seems to be a bug in CINT!
{
  gROOT->LoadMacro("digipulse.c+g");
  gROOT->LoadMacro("psd_test_PuC.C");
  gROOT->LoadMacro("wd_calc.cpp");
    
  // wave parameters
  
  ParaPulse_t para_p;
  para_p.ePolar    = ep_m;     // polarity: negative
  para_p.bAutoBase = true;     // automaticly search the baseline
  para_p.fBase     = 1024;     // default baseline
  para_p.fBinResolution = 1;   // bin resolution: 1 ns
  para_p.fVResolution   = 1;   // volt resolution: 1 mV
  para_p.fSwing         = 3;   // wave fluctuation: ~ 3mV
  para_p.fTrigger       = 0.4; // trigger position: 40%
  para_p.fThreshold     = 10;  // threshold: 10 mV
  para_p.fDCoffset      = 0.;  // DC offset (unit: mV) -- used with baseline
  para_p.bGate        = true;  // software gate
  para_p.fsGate.start = 50;    // soft-gate: start
  para_p.fsGate.end   = 350;   // soft-gate: end

  para_p.pf_ocut = NULL;   // cut function for original data:
                           //   bool f(int *, const uint32_t *)
  para_p.pf_ucut = NULL;   // cut function for unified data:
                           //   bool f(int *, const int *)

  const char *fout = "test/BC501-PuC.root";
  
  TChain *T = new TChain("wdPulse");
  T->Add("test/wd-BC501-PuC.root");

  psd_test_PuC(T, &para_p, fout);

  // 
  Bool_t  bExtP = true;
  
  // PSD parameters
  
  Bool_t   bPSD = true;
  ParaPSD_QR_t  para_psd;
  
  para_psd.fT1 = -10;   // 10 ns before the peak
  para_psd.fT2 = -10;   // usually = fT1
  para_psd.fT3 =  10;   // 10 ns after the peak
  para_psd.fT4 =  90;  // 100 ns after the peak


  // range 1
  wd_calc( T, &para_p, fout, bExtP, bPSD, &para_psd,
           "BC501PuC_1",
           "pq_BC501PuC_1",
           "Pulse Quantities of BC501 (Pu-C)",
           "RECREATE" );

}

