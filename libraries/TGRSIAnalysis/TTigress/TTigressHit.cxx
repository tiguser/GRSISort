
#include "TTigress.h"
#include "TTigressHit.h"
#include <TClass.h>

ClassImp(TTigressHit)

TTigressHit::TTigressHit()  {  
  Clear();
}

TTigressHit::~TTigressHit()  {  }

TTigressHit::TTigressHit(const TTigressHit& rhs)  {  
   (rhs).Copy(*this);
}


void TTigressHit::Clear(Option_t *opt)  {
  TGRSIDetectorHit::Clear(opt);
  first_segment = 0;
  first_segment_charge = 0.0;
  is_crys_set     = false;
  crystal         = -1;

  segment.clear();
  bgo.clear();
}

void TTigressHit::Copy(TTigressHit &rhs) const {
  TGRSIDetectorHit::Copy((TGRSIDetectorHit&)rhs);
  for(int i=0;i<segments.size();i++) {
    TGRSIDectorHit temp;
    segments.at(i).Copy(temp);
    rhs.segments.push_back(temp); 
  }
  for(int i=0;i<bgos.size();i++) {
    TGRSIDectorHit temp;
    bgos.at(i).Copy(temp);
    rhs.bgos.push_back(temp); 
  }
}


void TTigressHit::Print(Option_t *opt)  const {
}

void TTigressHit::CheckFirstHit(int charge,int segment)  {
  if(abs(charge) > first_segment_charge)  {
     first_segment = segment;
  }
  return;        
}

void TTigressHit::SumHit(TTigressHit *hit)  {
  if(this == hit)  {
//    lasthit = position;
    lastpos = std::make_tuple(GetDetector(),GetCrystal(),GetInitialHit());
    return;
  }
  this->core.SetEnergy(this->GetEnergy() + hit->GetEnergy());
  this->lasthit = hit->GetPosition();
  this->lastpos = std::make_tuple(hit->GetDetector(),hit->GetCrystal(),hit->GetInitialHit());
}


TVector3 TTigressHit::GetPosition(Double_t dist) { return TTigress::GetPosition(GetDetector(),GetCrystal(),GetInitialHit(),dist); }



int TTigressHit::GetCrystal() {
   if(is_crys_set)
      return crystal;

   TChannel *chan = GetChannel();
   if(!chan)
      return -1;
   MNEMONIC mnemonic;
   ParseMNEMONIC(chan->GetChannelName(),&mnemonic);
   char color = mnemonic.arraysubposition[0];
   switch(color) {
      case 'B':
         return 0;
      case 'G':
         return 1;
      case 'R':
         return 2;
      case 'W':
         return 3;  
   };
   return -1;  
}
