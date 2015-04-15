
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

void wd_calc( TChain *tc, ParaPulse_t *p_pulse, ParaPSD1_t *p_psd1,
              const char *fnout="pqout.root" )
{
  // gROOT->LoadMacro("digipulse.c+");
  // gROOT->LoadMacro("libdigipulse.so");

  WaveDump_t wd;
  
  // TFile *f = new TFile(fnroot);
  // TTree *T = f->Get("wdPulse");
  tc->SetBranchAddress("Wave", &wd.nLen); 
  
  // data output
  PulseForm_t      fpulse;
  PulseQuantity_t *pq;
  PulsePSD1_t     *psd1;
  
  TTree    *fPQ;  // Physical quantites
  TFile    *fOut; // Output file

  // histograms
  TH1F  *h1 = new TH1F("h1psd1","PSD1 of pulse",1024, 0, 1);
  TH2F  *h2a = new TH2F("h2qt_psd1", "Qtot-PSD1 of Pulse",
                        256, 0, 20000, 256, 0, 1.);
  TH2F  *h2b = new TH2F("h2qt_q1", "Qtot-Q2 of Pulse",
                        256, 0, 20000, 256, 0, 10000);

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
  Long64_t cnt_ok = 0;
  Long64_t cnt_ko_q = 0;
  Long64_t cnt_ko_psd1 = 0;
  Long64_t N = tc->GetEntries();
  for(Long64_t i=0; i< N; i++) {
    tc->GetEntry(i);

    // printf("Origin:");
    // for(int j=110; j<120; j++) {
    //   printf(" %d", wd.WAVE[j]);
    // }
    // printf("\n");
    
    fpulse.nlen = wd.nLen;
    fpulse.data = wd.WAVE;
    
    if( ! get_quantity(&pq, &fpulse, p_pulse) ) {
      // std::cerr << "Failed to calculate quantities!" << std::endl;
      cnt_ko_q++;
      continue;
    }      
    if( ! get_psd1(&psd1, &fpulse, p_pulse, p_psd1) ) {
      // std::cerr << "Failed to calculate PSD1!" << std::endl;
      cnt_ko_psd1++;
      continue;
    }
    
    h1->Fill(psd1->fPSD1);
    h2a->Fill(pq->fQtot, psd1->fPSD1);
    h2b->Fill(pq->fQtot, psd1->fQ2);
    fPQ->Fill();

    cnt_ok ++;
  }

  std::cout << "Entries processed: " << cnt_ok << " / " << N << std::endl
            << "Percent: " << (Double_t) cnt_ok * 100 / N << std::endl
            << "Failed: Q=" << cnt_ko_q << "; PSD1=" << cnt_ko_psd1 << std::endl;

  // histograms
  h1->Write();
  h2a->Write();
  h2b->Write();
  
  // end
  fPQ->Write();
  fOut->Close();

  // free
  delete pq;
  delete psd1;
}

void wd_calc_q( TChain *tc, ParaPulse_t *p_pulse, const char *fnout="pqout.root" )
{
  WaveDump_t wd;
  
  tc->SetBranchAddress("Wave", &wd.nLen); 
  
  // data output
  PulseForm_t      fpulse;
  PulseQuantity_t *pq;
  
  TTree    *fPQ;  // Physical quantites
  TFile    *fOut; // Output file

  // init 
  fOut = new TFile(fnout,"RECREATE");
  fPQ  = new TTree("PulseQ","Pulse Quantities");

  pq   = new PulseQuantity_t();
  psd1 = new PulsePSD1_t();
  fPQ->Branch("pq", pq,
              "Base/D:Swing/D:Peak/D:PeakH/D:Ppre/D:Ppost/D:FWHM/D:Trigger/D:Tpre/D:Tpost/D:Q/D:Qtot/D:Qpre/D:Qpost/D");

  // paras
  fpulse.ndigi = 2;

  // loop
  Long64_t cnt_ok = 0;
  Long64_t cnt_ko_q = 0;
  Long64_t N = tc->GetEntries();
  for(Long64_t i=0; i< N; i++) {
    tc->GetEntry(i);
    
    fpulse.nlen = wd.nLen;
    fpulse.data = wd.WAVE;
    
    if( ! get_quantity(&pq, &fpulse, p_pulse) ) {
      // std::cerr << "Failed to calculate quantities!" << std::endl;
      cnt_ko_q++;
      continue;
    }      

    fPQ->Fill();
    cnt_ok++;
  }

  std::cout << "Entries processed: " << cnt_ok << " / " << N << std::endl
            << "Percent: " << (Double_t) cnt_ok * 100 / N
            << "; Failed: Q=" << cnt_ko_q << std::endl;

  // end
  fPQ->Write();
  fOut->Close();

  // free
  delete pq;
}

