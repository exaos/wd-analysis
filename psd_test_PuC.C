
// #include "digipulse.h"

// WaveDump data structure

typedef struct WaveDump {
  UInt_t nLen;
  UInt_t nBoard;
  UInt_t nChannel;
  UInt_t nEventID;
  UInt_t nPattern;
  UInt_t nTimeStamp;
  UInt_t nDAC;
  UShort_t WAVE[4200]; // The input wave points should less than this
} WaveDump_t;

//==================================================

// check the best PSD resolution range for Pu-C
void psd_test_PuC(TTree *T,
                  ParaPulse_t *para_p,
                  const char *fout = "test/BC501-PuC-ranges.root" )
{
  // gROOT->LoadMacro("digipulse.c+g");
  // gROOT->LoadMacro("libdigipulse.so");
  
  // PSD parameters
  
  ParaPSD_QR_t  para_psd;
  
  para_psd.fT1 = -10;   // 10 ns before the peak
  para_psd.fT2 = -10;   // usually = fT1
  para_psd.fT3 =  10;
  para_psd.fT4 =  100;  // 100 ns after the peak

  // Tree

  WaveDump_t wd;
  T->SetBranchAddress("Wave", &wd.nLen);

  // output data
  TTree    *tPQ  = new TTree("pq_BC501","PSD test for BC501");  // Physical quantites
  TFile    *fOut = new TFile("test/BC501-PSDs.root", "RECREATE"); // Output file

  // branches
  PulsePortrait_t    *p_pp = new PulsePortrait_t();
  PulseQ_t           *p_pq = new PulseQ_t();
  PulsePortraitExt_t *p_pp_ex = new PulsePortraitExt_t();
  PulsePSD_QR_t      *p_psd   = new PulsePSD_QR_t();
  PulsePSD_QR_t      *p_psd_fib = new PulsePSD_QR_t();
  ParaPSD_QR_t       *para_psd_f= new ParaPSD_QR_t();

  tPQ->Branch("pp", p_pp, "Base/D:Swing/D:Peak/D:PeakH/D:Trigger/D");
  tPQ->Branch("pq", p_pq, "Q/D:Qtot/D:Qpre/D:Qpost/D");
  tPQ->Branch("pp_ex",   p_pp_ex,   "Ppre/D:Ppost/D:FWHM/D:Tpre/D:Tpost/D");
  tPQ->Branch("psd",     p_psd,     "PSD/D:Qlong/D:Qshort/D:Qtail/D");
  tPQ->Branch("psd_fib", p_psd_fib, "PSD/D:Qlong/D:Qshort/D:Qtail/D");
  tPQ->Branch("psd_para",para_psd_f,"T1/D:T2/D:T3/D:T4");
  
  // histograms
  TH1F  *h1_q_w = new TH1F( "h1_q_w", "Qtot: wide range", 2048, 0, 1.8e5 );  // wide range Qtot
  TH1F  *h1_q_n = new TH1F( "h1_q_n", "Qtot: narrow range", 2048, 0, 6.0e4 );  // narrow range Qtot
  TH1F  *h1_psd = new TH1F( "h1_psd", "PSD Using Q-R method", 1024, 0, 1. );  // PSD
  TH2F  *h2_q_psd = new TH2F("h2_q_psd", "Qlong vs. PSD",
                             256, 0., 1., 256, 0., 1.6e4); // Qtot vs. PSD
  TH2F  *h2_q_qt = new TH2F("h1_q_qt", "Qlong vs. Qtail",
                            256, 0., 5.e3, 256, 0., 1.6e4); // Qtot vs. Qtail
  TH1F  *h1_psd_f = new TH1F( "h1_psd_f", "PSD Using Q-R method (fixed)",
                              1024, 0, 1. );  // PSD
  TH2F  *h2_q_psd_f = new TH2F("h2_q_psd_f", "Qlong vs. PSD (fixed)",
                               256, 0., 1., 256, 0., 1.6e4); // Qtot vs. PSD
  TH2F  *h2_q_qt_f = new TH2F("h1_q_qt_f", "Qlong vs. Qtail (fixed)",
                              256, 0., 5.e3, 256, 0., 1.6e4); // Qtot vs. Qtail

  // loop
  
  Long64_t cnt_ok = 0;
  Long64_t cnt_ko = 0;
  Long64_t cnt_ko_pex = 0;
  Long64_t cnt_ko_psd = 0;
  Long64_t N = T->GetEntries();

  std::cout << "Processing Tree " << T->GetName() << " ..." << std::endl;
  
  // pulse data
  PulseForm_t *p_pulse = new PulseForm_t();
  uint32_t    *d_orig;
  int         *d_usig;
  int          nlen = 0;
  
  p_pulse->ndigi = 2;

  for(Long64_t i=0; i< N; i++) {
    
    Bool_t   isFailed = false;
    
    T->GetEntry(i);
    
    p_pulse->nlen = wd.nLen;
    p_pulse->data = wd.WAVE;

    // cast data
    
    if( ! dp_cast_data( &d_orig, &d_usig, &nlen, p_pp, p_pulse, para_p ) ) {
      cnt_ko++;
      continue;
    }

    // calculate Qtot
    if( ! dp_get_q( p_pq, &nlen, d_usig, p_pp, para_p ) ) {
      cnt_ko++;
      continue;
    }

    // calculate extended portrait
    if( ! dp_get_portrait_ext( p_pp_ex, &nlen, d_usig, p_pp, para_p ) ) {
      // std::cerr << "Failed to calculate extended portrait!" << std::endl;
      cnt_ko_pex++;
      isFailed = true;
    }
    
    // calculate PSD
    if( !dp_get_psd_qr( p_psd, &nlen, d_usig, p_pp, para_p, &para_psd ) ) {
      // std::cerr << "Failed to calculate PSD!" << std::endl;
      cnt_ko_psd++;
      isFailed = true;
    }
    
    if( !dp_get_psd_qr_fib( p_psd_fib, &nlen, d_usig, p_pp, para_p, para_psd_f ) ) {
      cnt_ko_psd++;
      isFailed = true;
    }
    
    if( isFailed ) continue;

    // generate histograms
    h1_q_w->Fill(p_pq->fQtot);
    h1_q_n->Fill(p_pq->fQtot);

    h1_psd->Fill(p_psd->fPSD);
    h2_q_psd->Fill(p_psd->fPSD, p_psd->fQlong);
    h2_q_qt->Fill(p_psd->fQtail, p_psd->fQlong);
    
    h1_psd_f->Fill(p_psd_fib->fPSD);
    h2_q_psd_f->Fill(p_psd_fib->fPSD,  p_psd_fib->fQlong);
    h2_q_qt_f->Fill(p_psd_fib->fQtail, p_psd_fib->fQlong);

    // fill tree
    tPQ->Fill();

    // next
    free(d_orig);
    free(d_usig);

    cnt_ok++;
  }

  std::cout << "Nlen: " << nlen
            << "; Entries processed: " << cnt_ok << " / " << N << std::endl
            << "Successed: " << (Double_t) cnt_ok * 100 / N << "%" << std::endl
            << "Failed: " << cnt_ko*100/N << "%, " << cnt_ko << std::endl;
  std::cout << "ExtP Failed: " << cnt_ko_pex*100/N << "%, " << cnt_ko_pex << std::endl;
  std::cout << "PSD Failed: " << cnt_ko_psd*100/N << "%, " << cnt_ko_psd << std::endl;

  // histograms
  fOut->cd();
  
  h1_q_w->Write();
  h1_q_n->Write();

  h1_psd->Write();
  h2_q_psd->Write();
  h2_q_qt->Write();

  h1_psd_f->Write();
  h2_q_psd_f->Write();
  h2_q_qt_f->Write();

  // end
  tPQ->Write();
  fOut->Close();

  // free
  delete p_pp;
  delete p_pq;
  delete p_pp_ex;
  delete p_psd;
}

//============================================================
