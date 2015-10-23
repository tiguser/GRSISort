
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

//TTigressHit::TTigressHit(const TTigressHit& rhs)  {  
//  Clear();
//  TGRSIDetectorHit::CopyFragment(rhs);
//}

TTigressHit::TTigressHit(const TFragment& frag) : TGRSIDetectorHit(frag) {
  //Clear();
  //  	
}
void TTigressHit::Clear(Option_t *opt)  {
  TGRSIDetectorHit::Clear(opt);
  first_segment = 0;
  first_segment_charge = 0.0;
  is_crys_set     = false;
  fCrystal         = -1;
  
  segments.clear();
  bgos.clear();
}

void TTigressHit::Copy(TObject &rhs) const {
  TGRSIDetectorHit::Copy((TGRSIDetectorHit&)rhs);
  for(int i=0;i<segments.size();i++) {
    TGRSIDetectorHit temp;
    segments.at(i).Copy(temp);
    ((TTigressHit&)rhs).segments.push_back(temp); 
  }
  for(int i=0;i<bgos.size();i++) {
    TGRSIDetectorHit temp;
    bgos.at(i).Copy(temp);
    ((TTigressHit&)rhs).bgos.push_back(temp); 
  }
  ((TTigressHit&)rhs).is_crys_set = is_crys_set;
  ((TTigressHit&)rhs).fCrystal    = fCrystal;

}

double TTigressHit::GetDoppler(double beta,TVector3 *vec) { 
  bool madevec = false;
  if(vec==0) {
    vec = &beam;
  }
  double gamma = 1/(sqrt(1-pow(beta,2)));
  return this->GetEnergy()*gamma*(1-beta*TMath::Cos(this->GetPosition().Angle(*vec)));
}

void TTigressHit::Print(Option_t *opt)  const {
}

void TTigressHit::CheckFirstHit(int charge,int segment)  {
  if(abs(charge) > first_segment_charge)  {
     first_segment = segment;
  }
  return;        
}

void TTigressHit::SumHit(TTigressHit &hit)  {
  if(this == &hit)  {
//    lasthit = position;
    lastpos = std::make_tuple(GetDetector(),GetCrystal(),GetInitialHit());
    return;
  }
  this->SetEnergy(GetEnergy() + hit.GetEnergy());
  this->lasthit = hit.GetPosition();
  this->lastpos = std::make_tuple(hit.GetDetector(),hit.GetCrystal(),hit.GetInitialHit());
}


TVector3 TTigressHit::GetPosition(Double_t dist) { return TTigress::GetPosition(GetDetector(),GetCrystal(),GetInitialHit(),dist); }



int TTigressHit::GetCrystal() const {
   if(is_crys_set)
      return fCrystal;
   is_crys_set = true;
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
