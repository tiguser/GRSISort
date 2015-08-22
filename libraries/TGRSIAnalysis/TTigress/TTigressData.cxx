

#include "TTigressData.h"


//ClassImp(TTigressData)

bool TTigressData::fIsSet = false;

TTigressData::TTigressData()  {  
  Clear();
}

TTigressData::~TTigressData()  {  }


void TTigressData::Clear(Option_t *opt)  {

  fCoreSet.clear();

  fIsSet = false;

  fClover_Nbr.clear();
  fCore_Nbr.clear();
  fCore_Frag.clear();

  fSeg_Clover_Nbr.clear();
  fSeg_Core_Nbr.clear();
  fSegment_Nbr.clear();  
  fSegment_Frag.clear();
}

void TTigressData::Print(Option_t *opt) const {
  // not yet written.
  printf("not yet written.\n");
}


