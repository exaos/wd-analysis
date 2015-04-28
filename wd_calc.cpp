// -*- mode: c++ ; coding: utf-8

// This script need the following to be executed first:
//     [] gROOT->LoadMacro("digipulse.c+");
// If digipulse.h/c has been compiled into a library, try this:
//     [] gROOT->LoadMacro("libdigipulse.so");

//============================================================
// WaveDump data structure

typedef struct WaveDump {
  UInt_t   nLen;
  UInt_t   nBoardID;
  UInt_t   nPattern;
  UInt_t   nChannel;
  UInt_t   nEventID;
  UInt_t   nTimeStamp;
  UInt_t   nDAC;
  UShort_t WAVE[4200]; // The input wave points should less than this
} WaveDump_t;

//============================================================
// WaveDump output calcuation
// This depends on the TTree "wdPulse", which should be generated by wd2tree.py

void wd_calc( TChain       *tc /* The Tree pointer */,
              ParaPulse_t  *para_p /* analysis parameters */,
              const char   *fn_out  = "pqout.root" /* output filename */,
              Bool_t        bExtP = true   /* if extend portraits needed */,
              Bool_t        bPSD  = false  /* if PSD needed */,
              ParaPSD_QR_t *para_psd  = NULL /* parameters for PSD */,
              const char   *t_id = ""  /* calculation ID */,
              const char   *t_name  = "PulseQ" /* output tree name */,
              const char   *t_title = "Pulse Quantities" /* output tree title */,
              const char   *f_flag  = "UPDATE"  /* open file mode */ )
{
  WaveDump_t wd;

  // associate branch "Wave" to ~wd~
  tc->SetBranchAddress("Wave", &wd.nLen);

  //----------> Check parameters
  
  if( para_psd == NULL ) bPSD = false;

  // output data
  TTree    *tPQ;  // Physical quantites
  TFile    *fOut; // Output file
  
  // init output data
  
  fOut = new TFile( fn_out, f_flag );
  tPQ  = new TTree( t_name, t_title );

  // branches
  PulsePortrait_t    *p_pp = new PulsePortrait_t();
  PulseQ_t           *p_pq = new PulseQ_t();
  PulsePortraitExt_t *p_pp_ex;
  PulsePSD_QR_t      *p_psd;

  tPQ->Branch("pp", p_pp, "Base/D:Swing/D:Peak/D:PeakH/D:Trigger/D");
  tPQ->Branch("pq", p_pq, "Q/D:Qtot/D:Qpre/D:Qpost/D");
  
  if( bExtP ) {
    p_pp_ex = new PulsePortraitExt_t();
    tPQ->Branch("pp_ex", p_pp_ex, "Ppre/D:Ppost/D:FWHM/D:Tpre/D:Tpost/D");
  }

  if( bPSD ) {
    p_psd = new PulsePSD_QR_t();
    tPQ->Branch("psd", p_psd, "PSD/D:Qlong/D:Qshort/D:Qtail/D");
  }
  
  // histograms
  TH1F  *h1_q_w;  // wide range Qtot
  TH1F  *h1_q_n;  // narrow range Qtot
  TH1F  *h1_psd;  // PSD
  TH2F  *h2_q_psd;// Qtot vs. PSD
  TH2F  *h2_q_qt; // Qtot vs. Qtail

  char   sntmp[32], sttmp[356];

  sprintf(sntmp, "h1_q_w_%s", t_id);
  sprintf(sttmp, "Qtot (%s: long range)", t_id);
  h1_q_w = new TH1F( sntmp, sttmp, 2048, 0, 1.8e5 );
  
  sprintf(sntmp, "h1_q_n_%s", t_id);
  sprintf(sttmp, "Qtot (%s: narrow range)", t_id);
  h1_q_n = new TH1F( sntmp, sttmp, 2048, 0, 6.0e4 );

  // if PSD enabled
  if( bPSD ) {
    sprintf(sntmp, "h1_psd_%s", t_id);
    sprintf(sttmp, "PSD using Q-R-Method (%s)", t_id);
    h1_psd = new TH1F( sntmp, sttmp, 1024, 0, 1. );

    sprintf(sntmp, "h2_q_psd_%s", t_id);
    sprintf(sttmp, "Qtot vs. PSD (%s)", t_id);
    h2_q_psd = new TH2F(sntmp, sttmp, 256, 0., 1., 256, 0., 1.6e4);

    sprintf(sntmp, "h2_q_qt_%s", t_id);
    sprintf(sttmp, "Qtot vs. Qtail (%s)", t_id);
    h2_q_qt = new TH2F(sntmp, sttmp, 256, 0., 5.e3, 256, 0., 1.6e4);
  }

  // loop
  
  Long64_t cnt_ok = 0;
  Long64_t cnt_ko = 0;
  Long64_t cnt_ko_pex = 0;
  Long64_t cnt_ko_psd = 0;
  Long64_t N = tc->GetEntries();

  std::cout << t_id << ": Processing Tree " << tc->GetName() << " ..." << std::endl;
  
  // pulse data
  PulseForm_t *p_pulse = new PulseForm_t();
  uint32_t    *d_orig;
  int         *d_usig;
  int          nlen = 0;
  
  p_pulse->ndigi = 2;

  for(Long64_t i=0; i< N; i++) {
    
    Bool_t   isFailed = false;
    
    tc->GetEntry(i);
    
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
    if( bExtP && ! dp_get_portrait_ext( p_pp_ex, &nlen, d_usig, p_pp, para_p ) ) {
      // std::cerr << "Failed to calculate extended portrait!" << std::endl;
      cnt_ko_pex++;
      isFailed = true;
    }
    
    // calculate PSD
    if( bPSD && ! dp_get_psd_qr( p_psd, &nlen, d_usig, p_pp, para_p, para_psd ) ) {
      // std::cerr << "Failed to calculate PSD!" << std::endl;
      cnt_ko_psd++;
      isFailed = true;
    }
    
    if( isFailed ) continue;

    // generate histograms
    h1_q_w->Fill(p_pq->fQtot);
    h1_q_n->Fill(p_pq->fQtot);

    if( bPSD ) {
      h1_psd->Fill(p_psd->fPSD);
      h2_q_psd->Fill(p_psd->fPSD, p_pq->fQtot);
      h2_q_qt->Fill(p_psd->fQtail, p_pq->fQtot);
    }

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
  if( bExtP ) {
    std::cout << "ExtP Failed: " << cnt_ko_pex*100/N << "%, " << cnt_ko_pex << std::endl;
  }
  if( bPSD ) {
    std::cout << "PSD Failed: " << cnt_ko_psd*100/N << "%, " << cnt_ko_psd << std::endl;
  }

  // histograms
  h1_q_w->Write();
  h1_q_n->Write();
  if( bPSD ) {
    h1_psd->Write();
    h2_q_psd->Write();
    h2_q_qt->Write();
  }
  
  // end
  tPQ->Write();
  fOut->Close();

  // free
  delete p_pp;
  delete p_pq;
  if( bExtP ) delete p_pp_ex;
  if( bPSD )  delete p_psd;
}

