#define wdpsel_cxx
// The class definition in wdpsel.h has been generated automatically by the ROOT
// utility TTree::MakeSelector(). This class is derived from the ROOT class
// TSelector. For more information on the TSelector framework see
// $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// The following methods are defined in this file:
//    Begin():        called every time a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// Root > T->Process("wdpsel.C")
// Root > T->Process("wdpsel.C","some options")
// Root > T->Process("wdpsel.C+")
//

#include "wdpsel.h"

#include <TH2.h>
#include <TStyle.h>

#include <iostream>
using namespace std;

//============================================================

void wdpsel::Init(TTree *tree)
{
  // The Init() function is called when the selector needs to initialize a new
  // tree or chain. Typically here the branch addresses and branch pointers of
  // the tree will be set.  It is normally not necessary to make changes to the
  // generated code, but the routine can be extended by the user if needed.
  // Init() will be called many times when running on PROOF (once per file to be
  // processed).

  // Set branch addresses and branch pointers
  if (!tree) return;
  // fChain->SetTree(tree); // --> ROOT6?
  fChain = tree;
  fChain->SetMakeClass(1);

  fChain->SetBranchAddress("Wave", &Wave_nLen, &b_Wave);
}

Bool_t wdpsel::Notify()
{
  // The Notify() function is called when a new file is opened. This can be
  // either for a new TTree in a TChain or when when a new TTree is started when
  // using PROOF. It is normally not necessary to make changes to the generated
  // code, but the routine can be extended by the user if needed. The return
  // value is currently not used.

  return kTRUE;
}

//============================================================

void wdpsel::Begin(TTree * /*tree*/)
{
  // The Begin() function is called at the start of the query.  When running
  // with PROOF Begin() is only called on the client.  The tree argument is
  // deprecated (on PROOF 0 is passed).

  TString option = GetOption();
  if( option == "" ) {
    option="wdpsel.root";
  }
  
  fOut = new TFile(option,"RECREATE");
  fPQ  = new TTree("PulseQ","Pulse Quantities");

  pq   = new PulseQuantity_t();
  psd1 = new PulsePSD1_t();
  fPQ->Branch("pq", pq,
              "Base/D:Swing/D:Peak/D:PeakH/D:Ppre/D:Ppost/D:FWHM/D:Trigger/D:Tpre/D:Tpost/D:Q/D:Qtot/D:Qpre/D:Qpost/D");
  fPQ->Branch("psd1", psd1, "PSD1/D:Q1/D:Q2/D");

  // init
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
}

void wdpsel::SlaveBegin(TTree * /*tree*/)
{
  // The SlaveBegin() function is called after the Begin() function.  When
  // running with PROOF SlaveBegin() is called on each slave server.  The tree
  // argument is deprecated (on PROOF 0 is passed).

  TString option = GetOption();

}

Bool_t wdpsel::Process(Long64_t entry)
{
  // The Process() function is called for each entry in the tree (or possibly
  // keyed object in the case of PROOF) to be processed. The entry argument
  // specifies which entry in the currently loaded tree is to be processed.  It
  // can be passed to either wdpsel::GetEntry() or TBranch::GetEntry() to read
  // either all or the required parts of the data. When processing keyed objects
  // with PROOF, the object is already loaded and is available via the fObject
  // pointer.
  //
  // This function should contain the "body" of the analysis. It can contain
  // simple or elaborate selection criteria, run algorithms on the data of the
  // event and typically fill histograms.
  //
  // The processing can be stopped by calling Abort().
  //
  // Use fStatus to set the return value of TTree::Process().
  //
  // The return value is currently not used.

  fChain->GetTree()->GetEntry(entry);

  fpulse.nlen = Wave_nLen;
  fpulse.data = Wave_WAVE;
  
  if( ! get_quantity(&pq, &fpulse, &p_pulse) ||
      ! get_psd1(&psd1, &fpulse, &p_pulse, &p_psd1) )
    return kFALSE;

  fPQ->Fill();
  
  return kTRUE;
}

void wdpsel::SlaveTerminate()
{
  // The SlaveTerminate() function is called after all entries or objects have
  // been processed. When running with PROOF SlaveTerminate() is called on each
  // slave server.

}

void wdpsel::Terminate()
{
  // The Terminate() function is the last function to be called during a
  // query. It always runs on the client, it can be used to present the results
  // graphically or save the results to file.

  fPQ->Write();
  fOut->Close();
  fPQ = NULL;
  fOut = NULL;
}

