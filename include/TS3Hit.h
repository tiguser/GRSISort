#ifndef TS3HIT_H
#define TS3HIT_H

#include "Globals.h"

#include <cstdio>
#include "TFragment.h"
#include "TChannel.h"
#include "TGRSIDetectorHit.h" 

class TS3Hit : public TGRSIDetectorHit {
   public:
    TS3Hit();
    TS3Hit(TFragment &);
    virtual ~TS3Hit();
	 TS3Hit(const TS3Hit&);

 //   Double_t GetEnergy()       {  return energy;  }
 //   Int_t    GetCharge()       {  return charge;  }
 //   Long_t   GetTimeStamp()    {  return ts;      }
 //   Int_t    GetTime()         {  return time;    }
 //   Short_t  GetDetectorNumber()     { return detectornumber;  }
    Double_t GetLed()          {  return led;     }
    Short_t  GetRingNumber()   { return ring;   }
    Short_t  GetSectorNumber() { return sector; }
 //   Double_t GetCFD()          { return cfd;    }

  public:
    void Copy(TObject&) const;        //!
    void Print(Option_t *opt="") const;
    void Clear(Option_t *opt="");

    void SetRingNumber(Short_t rn)     { ring = rn;   }
    void SetSectorNumber(Short_t sn)   { sector = sn; }
//    void SetDetectorNumber(Short_t dn) { detectornumber = dn; }
//    void SetPosition(TVector3 &vec)    { fposition = vec; }
    void SetVariables(TFragment &frag) { //SetEnergy(frag.GetEnergy());
// 					 SetAddress(frag.ChannelAddress);						
//                                          SetCfd(frag.GetCfd());
//                                          SetCharge(frag.GetCharge());
//                                          SetTimeStamp(frag.GetTimeStamp()); 
                                         //SetTime(frag.GetZCross());
					 led    = frag.GetLed(); }
 

  private:
    //TVector3 position;
    Double_t    led;
    Short_t  ring;   //front
    Short_t  sector; //back
//    Short_t  detectornumber;
//   Double_t energy;
//    Double_t cfd;
//    Int_t    charge;
//    Long_t   ts;
//    Int_t    time;

  ClassDef(TS3Hit,4);

};

#endif
