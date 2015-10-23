#ifndef TIGRESSHIT_H
#define TIGRESSHIT_H

#include <cstdio>
#include <cmath>
#ifndef __CINT__
#include <tuple>
#endif

#include "TFragment.h"
#include "TChannel.h"

#include "TMath.h"
#include "TVector3.h"
#include "TClonesArray.h"

#include "TGRSIDetectorHit.h"


class TTigressHit : public TGRSIDetectorHit {
  public:
    TTigressHit();
    TTigressHit(const TFragment&);
    TTigressHit(const TTigressHit&);
    virtual ~TTigressHit();

  private:
    UShort_t first_segment;        
    Int_t    first_segment_charge;   //!
    mutable Bool_t   is_crys_set;            //!
    UShort_t fCrystal;                //!

    //TGRSIDetectorHit core;  this!
    std::vector<TGRSIDetectorHit> segments;
    std::vector<TGRSIDetectorHit> bgos;
  
    //need to do sudo tracking to build addback.
    TVector3 lasthit;                //!
    #ifndef __CINT__
    std::tuple<int,int,int> lastpos; //!
    #endif

  public:
    void SetHit() {}
    /////////////////////////    /////////////////////////////////////
    void SetCore(TGRSIDetectorHit &core)      { Copy(core); }               //!
    void AddSegment(TGRSIDetectorHit &seg)    { segments.push_back(seg);  } //!
    void AddBGO(TGRSIDetectorHit &bgo)        { bgos.push_back(bgo);  }     //!

    /////////////////////////    /////////////////////////////////////
    inline int GetInitialHit()           {  return first_segment;  }      //!
  
    TVector3 GetPosition(Double_t dist=110.); 

    double GetDoppler(double beta,TVector3 *vec=0); 

    inline int GetSegmentMultiplicity() { return segments.size(); } //!
    inline int GetBGOMultiplicity()     { return bgos.size();     } //!
    inline TGRSIDetectorHit &GetSegment(const int &i)       { return segments.at(i);}
    inline TGRSIDetectorHit &GetBGO(const int &i)           { return bgos.at(i);  }  
    inline TGRSIDetectorHit &GetCore()                      { return *this;  } 

    void CheckFirstHit(int charge,int segment);                    //!
    int  GetCrystal() const;

//    static bool Compare(TTigressHit lhs, TTigressHit rhs);        //!     { return (lhs.GetDetectorNumber() < rhs.GetDetectorNumber()); }
//    static bool CompareEnergy(TTigressHit lhs, TTigressHit rhs);  //!     { return (lhs.GetDetectorNumber() < rhs.GetDetectorNumber()); }
    
    void SumHit(TTigressHit&);                           //!
    #ifndef __CINT__
    inline std::tuple<int,int,int> GetLastPosition() {return lastpos;} //!
    #endif                         

  public:
    virtual void Clear(Option_t *opt = "");                          //!
    virtual void Copy(TObject&) const;                             //!
    virtual void Print(Option_t *opt = "") const;                    //!

  ClassDef(TTigressHit,3)
};




#endif
