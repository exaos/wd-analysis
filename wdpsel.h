//////////////////////////////////////////////////////////
// This class has been automatically generated on Wed Mar 18 15:52:42 2015 by
// ROOT version 5.34/00 from TTree wdPulse/WaveDump pulses found on file:
// s1.root
//////////////////////////////////////////////////////////

#ifndef wdpsel_h
#define wdpsel_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>

// Header file for the classes stored in the TTree if any.

// Utilities for digital waveforms
#include "digipulse.hpp"
  
// Fixed size dimensions of array or collections stored in the TTree if any.

class wdpsel : public TSelector {
 public :
  TTree          *fChain;   //!pointer to the analyzed TTree or TChain

  // Declaration of leaf types
  UInt_t          Wave_nLen;
  UInt_t          Wave_nBoardID;
  UInt_t          Wave_nChannel;
  UInt_t          Wave_nEventID;
  UInt_t          Wave_nPattern;
  UInt_t          Wave_nTimeStamp;
  UInt_t          Wave_nDAC;
  UShort_t        Wave_WAVE[4096];   //[nLen]

  // List of branches
  TBranch        *b_Wave;   // -- the branch
  
 wdpsel(TTree * /* tree */ =0) : fChain(0) { };
  virtual ~wdpsel() { }
  virtual Int_t   Version() const { return 2; }
  virtual void    Begin(TTree *tree);
  virtual void    SlaveBegin(TTree *tree);
  virtual void    Init(TTree *tree);
  virtual Bool_t  Notify();
  virtual Bool_t  Process(Long64_t entry);
  virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) {
    return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
  virtual void    SetOption(const char *option) { fOption = option; }
  virtual void    SetObject(TObject *obj) { fObject = obj; }
  virtual void    SetInputList(TList *input) { fInput = input; }
  virtual TList  *GetOutputList() const { return fOutput; }
  virtual void    SlaveTerminate();
  virtual void    Terminate();

  // data output
  PulseForm_t      fpulse;
  ParaPulse_t      p_pulse;
  ParaPSD1_t       p_psd1;
  PulseQuantity_t *pq;
  PulsePSD1_t     *psd1;
  
  TTree    *fPQ;  // Physical quantites
  TFile    *fOut; // Output file
 
  // Make this class visible to ROOT -- CINT
  ClassDef(wdpsel,0);
};

#endif

