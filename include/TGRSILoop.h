#ifndef TGRSILOOP_H
#define TGRSILOOP_H

#include <cstdio>
#include <string>
#include <fstream>

#ifndef __CINT__
#include <thread>
#endif

#include "TGRSIint.h"
#include "TMidasFile.h"
#include "TMidasEvent.h"
#include "TXMLOdb.h"
#include "TChannel.h"
#include "Globals.h"

#include "TObject.h"


class TGRSILoop : public TObject {
   
   public:
      static TGRSILoop *Get();
      virtual ~TGRSILoop();

   private:
      static TGRSILoop *fTGRSILoop;
      static bool suppress_error;
      TGRSILoop();

   public:
      void Initialize();
      void BeginRun(int transition,int runnumber, int time);
      void EndRun(int transition,int runnumber, int time);
      void Finalize();

   private:
      bool fTestMode;
      bool fOffline;   

      bool fIamTigress;
      bool fIamGriffin;

      TXMLOdb *fOdb;

      int fFragsReadFromMidas;
      int fFragsSentToTree;
		int fBadFragsSentToTree;
		int fDeadtimeScalersSentToTree;
		int fRateScalersSentToTree;
 
   #ifndef __CINT__
      std::thread *fMidasThread;
      std::thread *fFillTreeThread;
      std::thread *fFillScalerThread;
   #endif

   public:
      bool fMidasThreadRunning;
      bool fFillTreeThreadRunning;
      bool fFillScalerThreadRunning;
      
      void SetSuppressError(bool temp = true) { suppress_error = temp; } 
      bool IsOnline()   { return !fOffline;  }
      bool IsOffline()  { return fOffline;   }

      bool SortMidas();
      void ProcessMidasFile(TMidasFile*);
      void FillFragmentTree(TMidasFile*);
      void FillScalerTree();
      bool ProcessMidasEvent(TMidasEvent*,TMidasFile *mfile=0);
      bool ProcessTIGRESS(uint32_t *ptr,int &dsize,TMidasEvent *mevent=0,TMidasFile *mfile=0); 
      bool ProcessGRIFFIN(uint32_t *ptr,int &dsize,int bank,TMidasEvent *mevent=0,TMidasFile *mfile=0); 
      bool Process8PI(uint32_t stream,uint32_t *ptr,int &dsize,TMidasEvent *mevent=0,TMidasFile *mfile=0);
      bool ProcessEPICS(float *ptr,int &dsize,TMidasEvent *mevent=0,TMidasFile *mfile=0);
      //bool ProcessEPICS(double *ptr,int &dsize,TMidasEvent *mevent=0,TMidasFile *mfile=0);

      void SetFileOdb(char *data,int size);
      void SetTIGOdb();
      void SetGRIFFOdb();

      int GetFragsSentToTree()    { return fFragsSentToTree; }
      int GetFragsReadFromMidas() { return fFragsReadFromMidas; }

      static bool GetSuppressError() { return suppress_error; } 

      void Clear(Option_t *opt="");
      void Print(Option_t *opt="") const;

   ClassDef(TGRSILoop,0)
};

#endif
