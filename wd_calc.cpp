

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

void wd_calc( const char  *fnroot, ParaPulse_t p_pulse, ParaPSD1_t p_psd1,
              const char *fnout="pqout.root" )
{
  // gROOT->LoadMacro("digipulse.c+");
  // gROOT->LoadMacro("libdigipulse.so");

  WaveDump_t wd;
  
  TFile *f = new TFile(fnroot);
  TTree *T = f->Get("wdPulse");
  T->SetBranchAddress("Wave", &wd.nLen);
  
  Long64_t N = T->GetEntries();
  
  // data output
  PulseForm_t      fpulse;
  PulseQuantity_t *pq;
  PulsePSD1_t     *psd1;
  
  TTree    *fPQ;  // Physical quantites
  TFile    *fOut; // Output file

  // histograms
  TH1F  *h1 = new TH1F("h1psd1","PSD1 of pulse",1024, 0, 1);
  TH2F  *h2a = new TH2F("h2qt_psd1", "Qtot-PSD1 of Pulse",
                        256, 0, 40000, 256, 0, 1.);
  TH2F  *h2b = new TH2F("h2qt_q1", "Qtot-Q2 of Pulse",
                        256, 0, 40000, 256, 0, 25000);

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

  // loop
  for(Long64_t i=0; i< N; i++) {
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
    h1->Fill(psd1->fPSD1);
    h2a->Fill(pq->fQtot, psd1->fPSD1);
    h2b->Fill(pq->fQtot, psd1->fQ2);
    fPQ->Fill();
  }

  // histograms
  h1->Write();
  h2a->Write();
  h2b->Write();
  
  // end
  fPQ->Write();
  fOut->Close();
}

