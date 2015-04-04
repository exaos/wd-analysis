
// #include "digipulse.h"

typedef struct WaveDump {
  UInt_t nLen;
  UInt_t nBoard;
  UInt_t nChannel;
  UInt_t nEventID;
  UInt_t nPattern;
  UInt_t nTimeStamp;
  UInt_t nDAC;
  UShort_t WAVE[1029];
} WaveDump_t;

void wd_calc(const char *fnroot, const char *fnout="pqout.root")
{
  gROOT->LoadMacro("digipulse.c+g");
  
  WaveDump_t wd;
  
  TFile *f = new TFile(fnroot);
  TTree *T = f->Get("wdPulse");
  T->SetBranchAddress("Wave", &wd.nLen);
  
  Long64_t N = T->GetEntries();
  
  // data output
  PulseForm_t      fpulse;
  ParaPulse_t      p_pulse;
  ParaPSD1_t       p_psd1;
  PulseQuantity_t *pq;
  PulsePSD1_t     *psd1;
  
  TTree    *fPQ;  // Physical quantites
  TFile    *fOut; // Output file

  // init 
  fOut = new TFile(fnout,"RECREATE");
  fPQ  = new TTree("PulseQ","Pulse Quantities");

  pq   = new PulseQuantity_t();
  psd1 = new PulsePSD1_t();
  fPQ->Branch("pq", pq,
              "Base/D:Swing/D:Peak/D:PeakH/D:Ppre/D:Ppost/D:FWHM/D:Trigger/D:Tpre/D:Tpost/D:Q/D:Qtot/D:Qpre/D:Qpost/D");
  fPQ->Branch("psd1", psd1, "PSD1/D:Q1/D:Q2/D");

  // paras
  fpulse.ndigi = 2;
  p_pulse.bAutoBase = true;
  p_pulse.polar = p_m;
  p_pulse.fBase = 1024;
  p_pulse.fBinResolution = 1;
  p_pulse.fVResolution = 1;
  p_pulse.fSwing = 3;
  p_pulse.fThreshold = 5;
  p_psd1.fT1 = -10;
  p_psd1.fT2 =   5;
  p_psd1.fT3 =  50;
  p_psd1.fT4 =  90;

  // loop
  for(Long64_t i=0; i< /* 10 */ N; i++) {
    T->GetEntry(i);

    // printf("Origin:");
    // for(int j=110; j<120; j++) {
    //   printf(" %d", wd.WAVE[j]);
    // }
    // printf("\n");
    
    fpulse.nlen = wd.nLen;
    fpulse.data = wd.WAVE;
    
    if( ! get_quantity(&pq, &fpulse, &p_pulse) ||
        ! get_psd1(&psd1, &fpulse, &p_pulse, &p_psd1) )
      break;
    fPQ->Fill();
  }

  // end
  fPQ->Write();
  fOut->Close();
}

