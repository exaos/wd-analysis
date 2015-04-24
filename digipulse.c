
#include "digipulse.h"

//============================================================

// cast data array and prepare for analysis

bool dp_cast_data( uint32_t **d_orig, int **d_usig, 
                   int *nlen,  PulsePortrait_t *ppt,
                   const PulseForm_t *pulse, ParaPulse_t *para_p )
{
  int  isig = 1;
  PulsePoint_t  pp_min, pp_max;

  // temp variables
  int  idx, idx_count;
  int  i_start, i_end;
  int  iv, iv_ref, iv_base, iv_swing;

  //----------> Check parameters

  // set default fluctuation to 2
  if( para_p->fSwing == 0 ) para_p->fSwing = 2;
  
  //----------> software gate

  // check if software gate properly set
  
  if( para_p->bGate && ((para_p->fGate).end > 0) ) {

    // calculate index: (start,end)
    i_start = (int)( para_p->fBinResolution * (para_p->fGate).start );
    i_end   = (int)( para_p->fBinResolution * (para_p->fGate).end );
    
    if( i_end > pulse->nlen ) { i_end = pulse->nlen;   }
    if( i_start > i_end )     { i_start = 0;   }
    
    *nlen = i_end - i_start;
    
  } else {
    
    i_start = 0;
    i_end   = pulse->nlen;
    *nlen   = pulse->nlen;
    
  }

  //----------> data copy

  // memory initialization
  (*d_orig) = (uint32_t *) calloc( (*nlen), sizeof(uint32_t) );
  (*d_usig) = (int *)      calloc( (*nlen), sizeof(int) );
  if( ((*d_orig) == NULL) || ((*d_usig) == NULL) ) {
    fprintf(stderr, "dp_cast_data: Failed to initialized memory!\n");
    return false;
  }

  // copy original data
  for(idx=0; idx<(*nlen); idx++) {
    switch( pulse->ndigi ) {
    case 1:
      *((*d_orig)+idx) = *((uint8_t  *)(pulse->data) + idx + i_start);
      break;
    case 2:
      *((*d_orig)+idx) = *((uint16_t *)(pulse->data) + idx + i_start);
      break;
    case 4:
    default:
      *((*d_orig)+idx) = *((uint32_t *)(pulse->data) + idx + i_start);
    break;
    }
  }
  
  //----------> first cut

  // cut for original signal
  
  if( (para_p->fp_ocut != NULL) && !(*para_p->fp_ocut)(nlen, (*d_orig)) ) {
    // printf("Original cut: false!");
    return false;
  }

  //----------> baseline
  
  iv_swing = (int) (para_p->fSwing * para_p->fVResolution);
  
  // find the baseline if needed, usually
  
  if( para_p->bAutoBase ) {
    
    // search the first 10 bins (not last?)
    iv_ref = (*d_orig)[0];
    iv = idx_count = 0;
    for(idx = 0; idx < 20; idx++) {
      if( ((int)(*d_orig)[idx] - iv_ref) <= iv_swing ) {
        iv += (*d_orig)[idx];
        idx_count ++;
      }
    }
    
    // convert
    iv_base = iv / idx_count;
    para_p->fBase = (double) (iv) / idx_count / para_p->fVResolution;
    
  } else {
    
    // baseline: convert from mV to integer
    iv_base = (int) ((para_p->fBase - para_p->fDCoffset) * para_p->fVResolution);
    
  }

  //----------> unify

  // Calculate the diff data according to baseline and
  // flip the pulse if it is negative
  
  // pulse polarity
  if( para_p->polar == p_m ) {  isig = -1;  }
  
  pp_min.i = pp_max.i = 0;
  pp_min.v = pp_max.v = (*d_usig)[0] = isig * ((*d_orig)[0] - iv_base);
  
  for( idx=1; idx < (*nlen); idx++) {
    (*d_usig)[idx] = isig * ((*d_orig)[idx] - iv_base);
    if( (*d_usig)[idx] > pp_max.v ) {
      pp_max.i = idx;
      pp_max.v = (*d_usig)[idx];
    }
    if( (*d_usig)[idx] < pp_min.v ) {
      pp_min.i = idx;
      pp_min.v = (*d_usig)[idx];
    }
  }

  //----------> Pulse Portrait

  if( ppt == NULL ) { return false;  }

  // base, swing
  ppt->fBase  = para_p->fBase ;
  ppt->fSwing = para_p->fSwing ;

  // peak, peakH
  ppt->fPeak  = pp_max.i * para_p->fBinResolution ;
  ppt->fPeakH = pp_max.v * para_p->fVResolution ;

  // trigger
  ppt->fTrigger = (pulse->nlen * para_p->fTrigger - i_start) * para_p->fBinResolution ;
  
  //----------> last cut

  // cut for unified signal
  
  if( (para_p->fp_ucut != NULL) && !(*para_p->fp_ucut)(nlen, (*d_usig)) ) {
    // printf("Unified cut: false!");
    return false;
  }

  //----------> end
  return true;
}

// cast 8-bit pulse data
bool dp_cast_data_8( uint32_t **d_orig, int **d_usig,
                     int *nlen, PulsePortrait_t *ppt,
                     const PulseForm8_t *pulse, ParaPulse_t *para_p )
{
  PulseForm_t p8;
  p8.ndigi = 1;
  p8.nlen = pulse->nlen;
  p8.data = pulse->data;
  return dp_cast_data( d_orig, d_usig, nlen, ppt, &p8, para_p );
}

// cast 16-bit pulse data
bool dp_cast_data_16( uint32_t **d_orig, int **d_usig,
                      int *nlen, PulsePortrait_t *ppt,
                      const PulseForm16_t *pulse, ParaPulse_t *para_p )
{
  PulseForm_t p16;
  p16.ndigi = 2;
  p16.nlen = pulse->nlen;
  p16.data = pulse->data;
  return dp_cast_data( d_orig, d_usig, nlen, ppt, &p16, para_p );
}

//============================================================

// get extended portraits of the pulse

bool dp_get_portrait_ext( PulsePortraitExt_t *ppet, int *nlen, const int *d_sig,
                          PulsePortrait_t *ppt, ParaPulse_t *para_p )
{
  int   iv, iv_peak, iv_thres;
  int   idx, idx_peak, idx_r1, idx_r2;
  
  // initialize PulsePortraitExt
  
  if( ppet == NULL ) {
    ppet = (PulsePortraitExt_t *) calloc(1, sizeof(PulsePortraitExt_t));
  }

  //----------> threshold line
  
  if( para_p->fThreshold < para_p->fSwing ) {
    para_p->fThreshold = para_p->fSwing;
  }
  iv_thres = (int) (para_p->fThreshold * para_p->fVResolution);
  
  //----------> i_pre, i_post
  
  idx_peak = (int) (ppt->fPeak / para_p->fBinResolution);
  
  // find signal point begin to above threshold before the peak
  idx_r1 = 0;
  for(idx = 1; idx < idx_peak; idx++) {
    if( d_sig[idx] > iv_thres ) {
      idx_r1 = idx;
      break;
    }
  }
  
  // find signal point begin to below threshold after the peak
  idx_r2 = idx_peak;
  for(idx = idx_peak+1; idx < *nlen; idx++) {
    if( d_sig[idx] < iv_thres ) {
      idx_r2 = idx;
      break;
    }
  }

  // fPpre, fPost
  
  ppet->fPpre  = para_p->fBinResolution * (idx_peak - idx_r1);
  ppet->fPpost = para_p->fBinResolution * (idx_r2 - idx_peak);
  
  //----------> Trigger: i_trig
  
  ppet->fTpre  = ppt->fTrigger - para_p->fBinResolution * idx_r1;
  ppet->fTpost = para_p->fBinResolution * idx_r2 - ppt->fTrigger;
  
  //----------> FWHM
  
  iv_peak = (int) (ppt->fPeak / para_p->fVResolution);
  iv = iv_peak/2;
  
  idx_r1 = 0;
  for(idx = 1; idx < idx_peak; idx++) {
    idx_r1 = idx;
    if( (iv - d_sig[idx]) < 2 ) break;
  }

  idx_r2 = idx_peak;
  for(idx = idx_peak+1; idx < *nlen; idx++) {
    idx_r2 = idx;
    if( (d_sig[idx] - iv) < 2 ) break;
  }

  ppet->fFWHM = (idx_r2 - idx_r1) * para_p->fBinResolution;
  
  //----------> end
  return true;
}

//============================================================

// pulse integral

bool dp_get_q( PulseQ_t *pq, int *nlen, const int *d_sig,
               PulsePortrait_t *ppt, ParaPulse_t *para_p )
{
  int   iv_thres;
  int   iv_sum;
  int   idx, idx_peak, idx_r1, idx_r2;

  if( pq == NULL ) {
    pq = (PulseQ_t *) calloc(1, sizeof(PulseQ_t));
  }

  //----------> threshold line
  
  if( para_p->fThreshold < para_p->fSwing ) {
    para_p->fThreshold = para_p->fSwing;
  }
  iv_thres = (int) (para_p->fThreshold * para_p->fVResolution);
  
  //----------> i_pre, i_post

  idx_peak = (int) (ppt->fPeak / para_p->fBinResolution);
  
  // find signal point begin to above threshold before the peak

  idx_r1 = 0;
  for(idx = 1; idx < idx_peak; idx++) {
    if( d_sig[idx] > iv_thres ) {
      idx_r1 = idx;
      break;
    }
  }

  // Qpre
  
  iv_sum = 0;
  for( idx=idx_r1; idx < idx_peak; idx++ ) {
    iv_sum += d_sig[idx];
  }
  pq->fQpre = (double) iv_sum * para_p->fVResolution;
  
  // find signal point begin to below threshold after the peak

  idx_r2 = idx_peak;
  for(idx = idx_peak+1; idx < *nlen; idx++) {
    if( d_sig[idx] < iv_thres ) {
      idx_r2 = idx;
      break;
    }
  }

  // Qpost
  
  iv_sum = 0;
  for( idx = idx_peak; idx < idx_r2; idx++ ) {
    iv_sum += d_sig[idx];
  }
  pq->fQpost = (double) iv_sum * para_p->fVResolution;

  pq->fQ = pq->fQpre + pq->fQpost;

  //----------> Qtot

  iv_sum = 0;
  for( idx = 0; idx < *nlen; idx++ ) {
    iv_sum += d_sig[idx];
  }
  pq->fQtot = (double) iv_sum * para_p->fVResolution;
  
  //----------> end
  return true;
}

//============================================================

// get PSD using ratio of components

bool dp_get_psd_qr( PulsePSD_QR_t *psd, int *nlen, const int *d_sig,
                    PulsePortrait_t *ppt, ParaPulse_t *para_p,
                    ParaPSD_QR_t *para_psd )
{
  int  iv_sum;
  int  idx;
  int  idx_peak = (int) (ppt->fPeak / para_p->fBinResolution);

  // get gate index
  
  int  idx1 = idx_peak + (int) (para_psd->fT1 / para_p->fBinResolution);
  int  idx2 = idx_peak + (int) (para_psd->fT2 / para_p->fBinResolution);
  int  idx3 = idx_peak + (int) (para_psd->fT3 / para_p->fBinResolution);
  int  idx4 = idx_peak + (int) (para_psd->fT4 / para_p->fBinResolution);

  if( idx4 > *nlen ) idx4 = *nlen;
  if( idx3 > idx4 ) idx3 = idx4;
  if( idx1 > idx2 ) idx1 = idx2;
  if( idx3 <= idx2 ) {
    fprintf(stderr, "Error: wrong gate range!\n");
    return false;
  }
  
  // init if needed
  
  if( psd == NULL ) {
    psd = (PulsePSD_QR_t *) calloc(1, sizeof(PulsePSD_QR_t));
  }

  // long gate: T1 -- T4
  
  iv_sum = 0;
  for( idx = idx1; idx < idx4; idx++ ) {
    iv_sum += d_sig[idx];
  }
  psd->fQlong = (double) iv_sum * para_p->fVResolution ;

  // short gate: T2 -- T3
  
  iv_sum = 0;
  for( idx = idx2; idx < idx3; idx++ ) {
    iv_sum += d_sig[idx];
  }
  psd->fQshort = (double) iv_sum * para_p->fVResolution ;

  // tail, psd
  
  psd->fQtail = psd->fQlong - psd->fQshort;
  psd->fPSD = psd->fQtail / psd->fQlong;

  //----------> end
  return true;
}

