
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

void readtwd(const char *fn)
{
  WaveDump_t *wd = new WaveDump_t();
  
  TFile *f = new TFile(fn);
  
  TTree *T = f->Get("wdPulse");
  T->SetBranchAddress("Wave", &wd.nLen);
  
  ULong64_t N = T->GetEntries();
  for(int i=0; i<10; i++) {
    T->GetEntry(i);
    printf("nLen: %d; nEventID: %d\n", wd->nLen, wd->nPattern);
    for(int j=120; j<130; j++)
      printf(" %d", wd->WAVE[j]);
    printf("\n");
  }
}

