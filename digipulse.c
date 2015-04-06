#include "digipulse.h"

#include <stdlib.h>

//============================================================

bool cast_data( uint32_t **d1, int **d2, double **d3, ExtremeVal_t *ev,
                const PulseForm_t *pulse, ParaPulse_t *parap )
{
  int  isig = 1;

  // temp variablea
  int  idx, idx_count;
  int  iv, iv_ref;
  int  iv_base, iv_swing;
  
  // pulse polarity
  if(parap->polar == p_m) {
    isig = -1;
  }

  (*d1) = (uint32_t *)calloc(pulse->nlen, sizeof(uint32_t));
  (*d2) = (int *)     calloc(pulse->nlen, sizeof(int));
  (*d3) = (double *)  calloc(pulse->nlen, sizeof(double));

  if( ((*d1)==NULL) || ((*d2)==NULL) || ((*d3)==NULL) ) {
    return false;
  }
  
  for(idx=0; idx<pulse->nlen; idx++) {
    switch( pulse->ndigi ) {
    case 1:
      *((*d1)+idx) = *((uint8_t *) (pulse->data) + idx);
      break;
    case 2:
      *((*d1)+idx) = *((uint16_t *) (pulse->data) + idx);
      break;
    case 4:
    default:
      *((*d1)+idx) = *((uint32_t *) (pulse->data) + idx);
    break;
    }
  }
  
  // set default fluctuation to 5 mV
  if( parap->fSwing == 0 ) parap->fSwing = 2;
  iv_swing = (int) (parap->fSwing * parap->fVResolution);
  
  // find the baseline if needed, usually
  if( parap->bAutoBase ) {
    // search the first 10 bins (not last?)
    iv_ref = (*d1)[0];
    iv = idx_count = 0;
    for(idx = 0; idx < 10; idx++) {
      if( ((int)(*d1)[idx] - iv_ref) <= iv_swing ) {
        iv += (*d1)[idx];
        idx_count ++;
      }
    }
    // convert
    iv_base = iv / idx_count;
    parap->fBase = (double) (iv) / idx_count / parap->fVResolution;
  } else {
    // baseline: convert from mV to integer
    iv_base = (int) ((parap->fBase - parap->fDCoffset) * parap->fVResolution);
  }

  // Calculate the diff data according to baseline
  ev->i_min = ev->i_max = (*d2)[0] = isig * ((*d1)[0]-iv_base);
  ev->idx_min = ev->idx_max = 0;
  for( idx=1; idx < pulse->nlen; idx++) {
    (*d2)[idx] = isig * ((*d1)[idx] - iv_base);
    (*d3)[idx] = (double) (*d2)[idx];
    if( (*d2)[idx] > ev->i_max ) { ev->i_max = (*d2)[idx]; ev->idx_max = idx; }
    if( (*d2)[idx] < ev->i_min ) { ev->i_min = (*d2)[idx]; ev->idx_min = idx; }
  }
  
  return true;
}

//======================================================================

// Get the quantities of pulseform
bool get_quantity( PulseQuantity_t **pq, const PulseForm_t *pulse, ParaPulse_t *parap )
{
  uint32_t     *d_orig;
  int          *d_sig;
  double       *d_sm; // smoothed data
  
  // temp variablea
  int idx, idx1, idx2;
  int idx_trig, idx_peak;
  int idx_th1, idx_th2;
  int iv, iv_ref;
  int iv_base, iv_swing;
  int iv_thres, iv_peak;
  int iv_sum1, iv_sum2;
  double t_win;
  
  ExtremeVal_t ev;
  
  //---------- initialize for calculation ----------
  // cast data array
  if( cast_data(&d_orig, &d_sig, &d_sm, &ev, pulse, parap) == false ) {
    printf("ERROR: Failed to initialize memeory!\n");
    return false;
  }
  idx_peak = ev.idx_max;
  iv_peak  = ev.i_max;
  
  // initialize pq
  if( (*pq) == NULL) {
    printf("Init structure: PulseQuantity\n");
    (*pq) = (PulseQuantity_t *) malloc(sizeof(PulseQuantity_t));
  }

  //============================================================

  // update from pulse parameter
  (*pq)->fBase  = parap->fBase;
  (*pq)->fSwing = parap->fSwing;

  // trigger
  t_win = pulse->nlen * parap->fBinResolution;
  (*pq)->fTpre = (*pq)->fTrigger = parap->fTrigger * t_win;
  idx_trig = (int)((*pq)->fTrigger / parap->fBinResolution);
  (*pq)->fTpost = t_win - (*pq)->fTrigger;
  
  // Get the peak: position and height
  (*pq)->fPeak  = idx_peak * parap->fBinResolution;
  (*pq)->fPeakH = iv_peak * parap->fVResolution;

  // calculate: fPre, fPpost --- CHECKME
  iv_thres = (int) (parap->fThreshold * parap->fVResolution);
  
  idx_th1 = 0;
  for(idx = 0; idx < idx_peak; idx++ ) {
    if( d_sig[idx] > iv_thres ) {
      idx_th1 = idx;
      break;
    }
  }
  (*pq)->fPpre = (idx_peak - idx_th1) * parap->fBinResolution;

  idx_th2 = idx_peak;
  for(idx = idx_peak; idx < pulse->nlen; idx++) {
    if( d_sig[idx] <= iv_thres ) {
      idx_th2 = idx;
      break;
    }
  }
  (*pq)->fPpost = (idx_th2 - idx_peak) * parap->fBinResolution;
  

  // fFWHM
  iv_ref = iv_peak/2;
  idx1 = 0;
  idx2 = idx_peak;
  for(idx = 0; idx < idx_peak; idx++) {
    idx1 = idx;
    if( (iv_ref - d_sig[idx]) < 2 ) break;
  }
  for(idx = idx_peak; idx< pulse->nlen; idx++) {
    idx2 = idx;
    if( (d_sig[idx] - iv_ref) < 2 ) break;
  }
  (*pq)->fFWHM = (idx2 - idx1) * parap->fBinResolution;

  // fQtot
  iv_sum1 = 0;
  for( idx=0; idx < pulse->nlen; idx++) {
    iv_sum1 += d_sig[idx];
  }
  (*pq)->fQtot = (double) (iv_sum1) * parap->fVResolution;
  
  // fQ = fQpre + fQpost
  iv_sum1 = iv_sum2 = 0;
  for(idx = idx_th1; idx < idx_peak; idx++) {
    iv_sum1 += d_sig[idx];
  }
  for(idx = idx_peak; idx < idx_th2; idx++) {
    iv_sum2 += d_sig[idx];
  }
  (*pq)->fQpre = (double)iv_sum1 * parap->fVResolution;
  (*pq)->fQpost= (double)iv_sum2 * parap->fVResolution;
  (*pq)->fQ = (*pq)->fQpre + (*pq)->fQpost;
  
  //============================================================
  return true;
}

//======================================================================

bool get_psd1( PulsePSD1_t **psd1, const PulseForm_t *pulse,
               ParaPulse_t *parap, ParaPSD1_t *ppsd1 )
{
  uint32_t *d_orig;
  int      *d_sig;
  double   *d_sm;
  ExtremeVal_t ev;

  // temp var
  int   idx, idx1, idx2, idx3, idx4;
  int   idx_peak;
  int   iv_sum1, iv_sum2;
  
  // init psd1
  if( (*psd1) == NULL ) {
    printf("Init structure: PulsePSD1\n");
    (*psd1) = (PulsePSD1_t *) malloc( sizeof(PulsePSD1_t) );
  }
  
  // cast data
  if( cast_data( &d_orig, &d_sig, &d_sm, &ev, pulse, parap ) == false ) {
    printf("ERROR: Failed to initialize memeory!\n");
    return false;
  }

  idx_peak = ev.idx_max;
  
  //============================================================
  idx1 = idx_peak + (int) (ppsd1->fT1 * parap->fBinResolution);
  idx2 = idx_peak + (int) (ppsd1->fT2 * parap->fBinResolution);
  idx3 = idx_peak + (int) (ppsd1->fT3 * parap->fBinResolution);
  idx4 = idx_peak + (int) (ppsd1->fT4 * parap->fBinResolution);
  iv_sum1 = iv_sum2 = 0;

  if( idx1 > idx2 || idx2 > idx3 || idx3 > idx4 || idx4 > pulse->nlen )
    return false;
  
  for(idx = idx1; idx < idx4; idx++) {
    iv_sum1 += d_sig[idx];
  }
  
  for(idx = idx2; idx < idx3; idx++) {
    iv_sum2 += d_sig[idx];
  }
  
  (*psd1)->fQ1 = iv_sum1 * parap->fBinResolution * parap->fVResolution;
  (*psd1)->fQ2 = iv_sum2 * parap->fBinResolution * parap->fVResolution;
  (*psd1)->fPSD1 = (*psd1)->fQ2 / (*psd1)->fQ1;
  
  //============================================================
  return true;
}

