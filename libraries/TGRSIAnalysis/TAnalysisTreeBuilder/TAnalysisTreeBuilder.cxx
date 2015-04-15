
#include "Globals.h"

#include <TVirtualIndex.h>
#include <TTreeIndex.h>
#include <TSystem.h>

#include "TAnalysisTreeBuilder.h"
#include "TGRSIOptions.h"

#include <TMessage.h>
#include <TSocket.h>
#include <TServerSocket.h>
#include <TMonitor.h>
#include <TStopwatch.h>

#include <TMemFile.h>
#include <TFileMerger.h>

#include <TGRSIOptions.h>

//#include "TSharcData.h"



enum StatusKind {
   kStartConnection = 0,
   kProtocol        = 1,
   kProtocolVersion = 1
};




std::mutex TEventQueue::m_event;
std::mutex TWriteQueue::m_write;

bool TEventQueue::elock = false;
std::queue<std::vector<TFragment>*> TEventQueue::fEventQueue;
TEventQueue *TEventQueue::fPtrToQue = 0;

TEventQueue::TEventQueue() { }

TEventQueue::~TEventQueue() { }

TEventQueue *TEventQueue::Get() {
   if(!fPtrToQue)
      fPtrToQue = new TEventQueue;
   return fPtrToQue;
}

void TEventQueue::Add(std::vector<TFragment> *event) {
   //Thread-safe method for adding events to the event queue. 
   m_event.lock();
   fEventQueue.push(event);
   m_event.unlock();
   //UnsetLock();
   return;
}

std::vector<TFragment> *TEventQueue::PopEntry() {
   //Thread-safe method for taking an event out of the event queue
   std::vector<TFragment> *temp;
   m_event.lock();
   temp = fEventQueue.front();
   fEventQueue.pop();
   m_event.unlock();
   return temp;
}

int TEventQueue::Size() {
   //Thread-safe method for checking the size of the event queue
   int temp;
   m_event.lock();
   temp = fEventQueue.size();
   m_event.unlock();
   return temp;
}

bool TWriteQueue::wlock = false;
std::queue<std::map<const char*, TGRSIDetector*>*> TWriteQueue::fWriteQueue;
TWriteQueue *TWriteQueue::fPtrToQue = 0;

TWriteQueue::TWriteQueue() { }

TWriteQueue::~TWriteQueue() { }

TWriteQueue *TWriteQueue::Get() {
   //Returns a pointer to the write queue
   if(!fPtrToQue)
      fPtrToQue = new TWriteQueue;
   return fPtrToQue;
}

void TWriteQueue::Add(std::map<const char*, TGRSIDetector*> *event) {
   //Thread-safe method for adding to the event queue
   m_write.lock();
   fWriteQueue.push(event);
   m_write.unlock();
   return;
}

std::map<const char*, TGRSIDetector*> *TWriteQueue::PopEntry() {
   //Thread safe method for taking an event out of the write queue
   std::map<const char*, TGRSIDetector*> *temp;
   m_write.lock();
   temp = fWriteQueue.front();
   fWriteQueue.pop();
   m_write.unlock();
   return temp;
}

int TWriteQueue::Size() {
   //Thread-safe method for checking the size of the event queue
   int temp;
   m_write.lock();
   temp = fWriteQueue.size();
   m_write.unlock();
   return temp;
}

///************************************************///
///************************************************///
///************************************************///
///************************************************///
///************************************************///
///************************************************///
///************************************************///
///************************************************///


ClassImp(TAnalysisTreeBuilder)


std::mutex TAnalysisTreeBuilder::m_analysisout;
bool TAnalysisTreeBuilder::fServerRunning = true;


void TAnalysisTreeBuilder::AnalysisOutPlusPlus() {
  m_analysisout.lock();
  fAnalysisOut++;
  m_analysisout.unlock();
}

int TAnalysisTreeBuilder::AnalysisOut() {
  m_analysisout.lock();
  int tmp = fAnalysisOut;
  m_analysisout.unlock();
  return tmp;
}

//This sets the minimum amount of memory that root can hold a tree in.
const size_t TAnalysisTreeBuilder::MEM_SIZE = (size_t)1024*(size_t)1024*(size_t)1024*(size_t)8; // 8 GB

TAnalysisTreeBuilder* TAnalysisTreeBuilder::fAnalysisTreeBuilder = 0;

//Reset the statistics of the analysis tree builder
long TAnalysisTreeBuilder::fEntries     = 0;
int  TAnalysisTreeBuilder::fFragmentsIn = 0;
int  TAnalysisTreeBuilder::fAnalysisIn  = 0;
int  TAnalysisTreeBuilder::fAnalysisOut = 0;


TAnalysisTreeBuilder* TAnalysisTreeBuilder::Get() {
   //Returns an instance of the singleton AnalysisTreeBuilder
   if(fAnalysisTreeBuilder == 0)
      fAnalysisTreeBuilder = new TAnalysisTreeBuilder;
   return fAnalysisTreeBuilder;
}

TAnalysisTreeBuilder::TAnalysisTreeBuilder() {
   //The Default ctor for the analysis tree builder
   fFragmentChain = 0;
   fCurrentFragTree = 0;
   fCurrentFragFile = 0;
//   fCurrentAnalysisTree = 0;
//   fCurrentAnalysisFile = 0;
   fCurrentRunInfo = 0;

   fCurrentFragPtr = 0;
/*
   tigress = 0;//new TTigress;
   sharc = 0;//new TSharc;
   triFoil = 0;//new TTriFoil;
   //rf->Clear();
   csm = 0;//new TCSM;
   //spice->Clear(); s3->Clear();
   //tip->Clear();

   griffin = 0;//new TGriffin;
   sceptar = 0;//new TSceptar;
   paces   = 0;//new TPaces;
   descant = 0;//new TDescant;
   //dante->Clear();
   //zerodegree->Clear();
*/
}


TAnalysisTreeBuilder::~TAnalysisTreeBuilder() { }

void TAnalysisTreeBuilder::StartMakeAnalysisTree(int argc, char** argv) {
   //Sets up a fragment chain from the list of fragment files sent to
   //grsisort. This fragment chain then gets sorted by timestamp.
   if(argc==1) {
      SetUpFragmentChain(TGRSIOptions::GetInputRoot());
   } else {
      return;
   }
   SortFragmentChain();   

}


void TAnalysisTreeBuilder::InitChannels() {
   //Initializes the channels from a cal file on the command line when 
   //grsisort is started. If no cal file is input on the command line
   //grsisort attempts to read the calibration from the fragment tree
   //if it exists.
   
   if(!fCurrentFragTree)
      return;

   //Delete channels from memory incase there is something in there still
   TChannel::DeleteAllChannels(); 
   //Try to read the calibration data from the fragment tree
	TChannel::ReadCalFromTree(fCurrentFragTree);

   //If we find an input cal file, we overwrite what the tree calibration is with that cal file
   if(!TGRSIOptions::GetInputCal().empty()) {
      for(int x = 0;x<TGRSIOptions::GetInputCal().size();x++) {
         TChannel::ReadCalFile(TGRSIOptions::GetInputCal().at(x).c_str());
      }
   }
   printf("AnalysisTreeBuilder:  read in %i TChannels.\n", TChannel::GetNumberOfChannels());
}  



void TAnalysisTreeBuilder::SetUpFragmentChain(std::vector<std::string> infiles) {
   //Makes a TChain of all of the fragments input on the command line. This may not work as
   //desired for the Griffin DAQ. In this case histograms should be summed during post
   //processing.
   TChain *chain = new TChain("FragmentTree");
   for(int x=0;x<infiles.size();x++) 
      chain->Add(infiles.at(x).c_str());
   SetUpFragmentChain(chain);
   
}

void TAnalysisTreeBuilder::SetUpFragmentChain(TChain *chain) {
   //Sets the fFragment chain from the TChain chain.
   if(fFragmentChain)
      delete fFragmentChain;
   fFragmentChain = chain;
   fFragmentChain->CanDeleteRefs(true); 
}

void TAnalysisTreeBuilder::SortFragmentChain() {
   //Sorts the fragment chain by tree number.
   if(!fFragmentChain)
      return;
   
   int ntrees = fFragmentChain->GetNtrees();
   int nChainEntries = fFragmentChain->GetEntries();
   int treeNumber, lastTreeNumber = -1;

   printf("Found %i trees with %i total fragments.\n",ntrees,nChainEntries);

   for(int i=0;i<nChainEntries;i++) {
      fFragmentChain->LoadTree(i);
      treeNumber = fFragmentChain->GetTreeNumber();
      if(treeNumber != lastTreeNumber) {
         if(lastTreeNumber == -1) {
            printf(DYELLOW "Sorting tree[%d]: %s" RESET_COLOR "\n",treeNumber+1,fFragmentChain->GetTree()->GetCurrentFile()->GetName());
         } else {
            printf(DYELLOW "Changing to tree[%d/%d]: %s" RESET_COLOR "\n",treeNumber+1,ntrees,fFragmentChain->GetTree()->GetCurrentFile()->GetName());
            printf(DYELLOW "    Switched from tree[%d] at chain entry number %i" RESET_COLOR "\n",lastTreeNumber,i);
         }
         lastTreeNumber = treeNumber;
      }  else {
         continue;
      }
      fCurrentFragTree = fFragmentChain->GetTree();
      int nentries = fCurrentFragTree->GetEntries();
      SetupFragmentTree();
      
      fFragmentsIn = 0;
      fAnalysisIn  = 0;
      fAnalysisOut = 0;

      fSortFragmentDone = false;
      fPrintStatus = true;
      fProcessThread = new std::thread(&TAnalysisTreeBuilder::ProcessEvent,this);
//      fWriteThread = new std::thread(&TAnalysisTreeBuilder::WriteAnalysisTree,this);
      fStatusThread = new std::thread(&TAnalysisTreeBuilder::Status,this);

      //SetupOutFile();
      SetupOutServer();
      //SetupAnalysisTree();
      SetupAnalysisClients();

      if(fCurrentRunInfo->Griffin())
         SortFragmentTreeByTimeStamp();
      else
         SortFragmentTree();

      fSortFragmentDone = true;

      fProcessThread->join();
      fWriteThread->join();
      fPrintStatus = false;
      
      CloseAnalysisFile();
      printf("\n");
      i+=(nentries);//-10);
      fCurrentFragFile->Close("R");
   }
   printf("Finished chain sort.\n");
   return;
}

void TAnalysisTreeBuilder::SortFragmentTree() {
   //Sorts the fragment tree based on the TreeIndex major name.
   //It then puts the fragment into the event Q.
   long major_min = (long) fCurrentFragTree->GetMinimum(fCurrentFragTree->GetTreeIndex()->GetMajorName());
   if(major_min<0)
      major_min = 0;
   long major_max = (long) fCurrentFragTree->GetMaximum(fCurrentFragTree->GetTreeIndex()->GetMajorName());

   TTreeIndex *index = (TTreeIndex*)fCurrentFragTree->GetTreeIndex();
   fEntries = index->GetN();

   for(int j=major_min;j<=major_max;j++) {
      std::vector<TFragment> *event = new std::vector<TFragment>;
      int fragno = 1;
      while(fCurrentFragTree->GetEntryWithIndex(j,fragno++) != -1) {
         fFragmentsIn++;
         event->push_back(*fCurrentFragPtr);
      }
      if(event->empty()) {
         delete event;
      } else {
         TEventQueue::Get()->Add(event);
      }
   }
   printf("\n");
   fCurrentFragTree->DropBranchFromCache(fCurrentFragTree->GetBranch("TFragment"),true);
   fCurrentFragTree->SetCacheSize(0);
   return;
}


void TAnalysisTreeBuilder::SortFragmentTreeByTimeStamp() {
   //Suprisingly, sorts the fragment tree by times stamps.
   //It then takes the sorted fragments and puts them into the eventQ.

   TFragment *currentFrag = 0;

   //Find the TFragment cranch of the tree
   fCurrentFragTree->SetBranchAddress("TFragment",&currentFrag);

   //Get the tree index of the fragment tree
   TTreeIndex *index = (TTreeIndex*)fCurrentFragTree->GetTreeIndex();

   fEntries = index->GetN();
   Long64_t *indexvalues = index->GetIndex();
   //Set the major index to be sorted over as the high bits of the time stamp
   int major_max = fCurrentFragTree->GetMaximum("TimeStampHigh");

   //Read in the first fragment from the fragment tree
   fCurrentFragTree->GetEntry(indexvalues[0]);
   long firstTimeStamp = currentFrag->GetTimeStamp();
   int  firstDetectorType = currentFrag->DetectorType;

   //We set the buildevent flag to false by default. When the time gate closes we change this to true
   //to tell the code to build the event and send it to be written to the analysis tree.
   bool buildevent = false;
   std::vector<TFragment> *event = new std::vector<TFragment>;//(1,*currentFrag);
   event->push_back(*currentFrag);

   fFragmentsIn++; //Increment the number of fragments that have been read
   
   //loop over all of the fragments in the tree 
   for(int x=1;x<fEntries;x++) {
      if(fCurrentFragTree->GetEntry(indexvalues[x]) == -1 ) {  //major,minor) == -1) {
         //GetEntry Reads all branches of entry and returns total number of bytes read.
         //This means that if this if statement passes, we have had an I/O error.
         printf(DRED "FIRE!!!" RESET_COLOR  "\n");
         continue;
      }
      if(indexvalues[x] == indexvalues[x-1]) {
         //If the last fragment index equals the current fragment index, you have a real fire
         //this is likely caused by using a root version where the method we are employing here
         //just so happens to not work. It is unlikely that there is a different reason for why
         //this is happening.
         printf(DRED "REAL FIRE!!! x = %i, indexvalues[x] = %lld, indexvalues[x-1] = %lld" RESET_COLOR  "\n", x, indexvalues[x], indexvalues[x-1]);
         continue;
      }
      fFragmentsIn++;//Now that we have read a new entry, we need to increment our counter

      //if we've already seen this channel we add the event to the queue
      //if(channelSeen.count(oldFrag->ChannelNumber) == 1) {
         //we might want to create an error statement here!!!
         //TEventQueue::Get()->Add(event);
         //event = new std::vector<TFragment>;
         //channelSeen.clear();
      //}
      //event->push_back(*oldFrag);
      //channelSeen.insert(oldFrag->ChannelNumber);
      //printf("\ntime diff = %ld\n",currentFrag->GetTimeStamp() - firstTimeStamp);
      
      //We first compare the current fragment's time stamp to the time stamp of the fragment that opened
      //up the time gate. We will eventually make this a walking gate rather than a static gate so that
      //we can process the high-rate experiments better where the time-random background becomes more of an
      //issue.
      long timediff = currentFrag->GetTimeStamp() - firstTimeStamp;

      //We now set the "allowed" time windows for different detector streams to be called a coincidence
      //The way this is done right now is not correct and should be changed in the near future.
      int currentDetectorType = currentFrag->DetectorType;
      //if((firstDetectorType == 1 && currentDetectorType == 2) || (firstDetectorType == 2 && currentDetectorType == 1) ) {
      //   if(timediff < 100 || timediff > 200)
      //      buildevent =true;
      //} else if((firstDetectorType == 1 && currentDetectorType == 5) || (firstDetectorType == 5 && currentDetectorType == 1)) {
      //   if(timediff < 0 || timediff > 100)
      //      buildevent =true;
      //} else if((firstDetectorType == 1 && currentDetectorType == 6) || (firstDetectorType == 6 && currentDetectorType == 1)) {
      //   if(timediff < 0 || timediff > 200)
      //      buildevent =true;
      //} else if((firstDetectorType == 2 && currentDetectorType == 5) || (firstDetectorType == 5 && currentDetectorType == 2)) {
      //   if(timediff < 100 || timediff > 200)
      //      buildevent =true;
      //} else if((firstDetectorType == 2 && currentDetectorType == 6) || (firstDetectorType == 6 && currentDetectorType == 2)) {
      //   if(timediff < 0 || timediff > 100)
      //      buildevent =true;
      //} else if((firstDetectorType == 5 && currentDetectorType == 6) || (firstDetectorType == 6 && currentDetectorType == 5)) {
      //   if(timediff < 0 || timediff > 200)
      //      buildevent =true;
      //} else if((firstDetectorType == currentDetectorType)) {
      //   if(timediff < 0 || timediff > 100)
      //      buildevent =true;
      //} else   {
      //   if(timediff < 0 || timediff > 200)
      //      buildevent =true;
      //}

      //We check to see if the coincidence window is shut. If it is we start building the event out of the old fragments
      //and send it to the event Q for processing.
      //if(buildevent) {
//      if(abs(currentFrag->GetTimeStamp() - firstTimeStamp) > 200) {  // 2 micro-sec.
      if((currentFrag->GetTimeStamp() - firstTimeStamp) > 200) {  // 2 micro-sec.
         //printf("Adding %ld fragments to queue\n",event->size());
         //if(event->size() > 1) {
            //for(int i = 0; i < event->size(); ++i) {
               //event->at(i).Print();
            //}
         //}
         //Add the event to the event Q and put the new fragment which is not part of the event into the start of the
         //next event.
         TEventQueue::Get()->Add(event);
         //Create a new event and push back the first fragment
         event = new std::vector<TFragment>;//(1,*currentFrag);
         event->push_back(*currentFrag);
         //channelSeen.clear();
         
         //reset the build event flag and all of the properties of the fragment that opens up the time gate
         firstDetectorType = currentFrag->DetectorType;
         firstTimeStamp = currentFrag->GetTimeStamp();
         buildevent = false;
      } else {
         //If we aren't ready to "build" the event, we fill the current event with the new fragment
         event->push_back(*currentFrag);
      }
   }
   //in case we have fragments left after all of the fragments have been processed, we add them to the queue now
   if(event->size() > 0) {
      TEventQueue::Get()->Add(event);
   }
   
   //Drop the TFragmentBranch from the Cache so we aren't still holding it in memory
   fCurrentFragTree->DropBranchFromCache(fCurrentFragTree->GetBranch("TFragment"),true);
   fCurrentFragTree->SetCacheSize(0);
   return;
}




void TAnalysisTreeBuilder::SetupFragmentTree() {
   //Set up the fragment Tree to be sorted on time stamps or trigger Id's. This also reads the the run info out of the fragment tree.
   fCurrentFragFile = fCurrentFragTree->GetCurrentFile();
   fCurrentRunInfo  = (TGRSIRunInfo*)fCurrentFragFile->Get("TGRSIRunInfo");
   //if(fCurrentRunInfo) {
   //   TGRSIRunInfo::SetInfoFromFile(fCurrentRunInfo);
      fCurrentRunInfo->Print();
   //}

   //Intialize the TChannel Information
   InitChannels();

   //Set the sorting to be done on the timestamps if the fragment contains Griffin fragments
   if(fCurrentRunInfo->Griffin()) {
      fCurrentRunInfo->SetMajorIndex("TimeStampHigh");
      fCurrentRunInfo->SetMinorIndex("TimeStampLow");
   }

   //Check to see if the fragment tree already has an index set. If not build the index based on timestamps if it is a Griffin 
   //fragment. If it is not Griffin build based on the trigger Id.
   if(!fCurrentFragTree->GetTreeIndex()) {
      if(fCurrentRunInfo->MajorIndex().length()>0) {
         printf(DBLUE "Tree Index not found, building index on %s/%s..." RESET_COLOR,
                        fCurrentRunInfo->MajorIndex().c_str(),fCurrentRunInfo->MinorIndex().c_str());  fflush(stdout); 
         if(fCurrentRunInfo->MinorIndex().length()>0) {
            fCurrentFragTree->BuildIndex(fCurrentRunInfo->MajorIndex().c_str(),fCurrentRunInfo->MinorIndex().c_str());
         } else {
            fCurrentFragTree->BuildIndex(fCurrentRunInfo->MajorIndex().c_str());
         }
      } else {
         printf(DBLUE "Tree Index not found, building index on TriggerId/FragmentId..." RESET_COLOR);  fflush(stdout);   
         fCurrentFragTree->BuildIndex("TriggerId","FragmentId");
      }
      printf(DBLUE " done!" RESET_COLOR "\n");
   }

   //Set the branch to point at the Fragment Tree.
   TBranch *branch = fCurrentFragTree->GetBranch("TFragment");
   //Make the fCurrentFragPtr point at the Fragment Tree.
   branch->SetAddress(&fCurrentFragPtr);
   //Load the fragment tree into the MEM_SIZE (described in the header and above) into memory
   fCurrentFragTree->LoadBaskets(MEM_SIZE);   
   printf(DRED "\t MEM_SIZE = %zd" RESET_COLOR  "\n", MEM_SIZE);
   return;
}

void TAnalysisTreeBuilder::SetupOutServer() {
   //Sets up the anaysistree.root file to write the created events into
   if(!fCurrentRunInfo)
      return;
   std::string outfilename;
   if(fCurrentRunInfo->SubRunNumber() == -1)
     outfilename = Form("analysis%05i.root",fCurrentRunInfo->RunNumber());
   else
     outfilename = Form("analysis%05i_%03i.root",fCurrentRunInfo->RunNumber(),fCurrentRunInfo->SubRunNumber());
   //We add the output analysis file to the "input root files" in case we want to do something with that file after we finish 
   //sorting it.   In otherwords, adding it hear opens it automatically in interactive mode once the sort is over.
   TGRSIOptions::AddInputRootFile(outfilename);

   fCurrentAnalysisFileName = outfilename;

   fFillServerThread = new std::thread(&TAnalysisTreeBuilder::FillServer,this);

   //if(fCurrentAnalysisFile)
   //   delete fCurrentAnalysisFile;
   //fCurrentAnalysisFile = new TFile(outfilename.c_str(),"recreate");
   //fCurrentAnalysisFile->SetCompressionSettings(1);
   printf("started output server feeding file: %s\n",fCurrentAnalysisFileName.c_str());
   return;
}

void TAnalysisTreeBuilder::FillServer() {
  TServerSocket *socketServer = new TServerSocket(9099,true);
  TMonitor *monitor = new TMonitor;
  monitor->Add(socketServer);

  unsigned int numberofclients=0;
  TMemFile *transient = 0;

  TFileMerger merger(false,false);
  merger.SetPrintLevel(0);

  while(fServerRunning) {
    TMessage *message;
    TSocket  *socket;
    socket = monitor->Select();
    if(socket->IsA() == TServerSocket::Class()) {
      if(numberofclients > TGRSIOptions::GetNumberOfWriteThreads()) {
        printf("currently only excepting %i writing threads.\n", TGRSIOptions::GetNumberOfWriteThreads() );
        monitor->Remove(socket);
        socketServer->Close();
      } else {
         TSocket *client = ((TServerSocket*)socket)->Accept();
         client->Send(numberofclients++,kStartConnection);
         client->Send(kProtocolVersion,kProtocol);
         monitor->Add(client);
         printf("Accepted %i connections\n",numberofclients);
      }
      continue;
    }
 
    socket->Recv(message);
    if(message==NULL) {
      printf("[recv] clent message null\n");
      continue;
    }
    switch(message->What()) {
       case kMESS_STRING: {
          char str[64];
          message->ReadString(str,64);
          printf("Client %i: '%s\n'",numberofclients-1,str);
          monitor->Remove(socket);
          //
          socket->Close();
          numberofclients--;
          if(monitor->GetActive()==0 || numberofclients==0) {
            printf("no active clients, closing server\n");
            delete message;
            return;
          }
       }
       break;
       case kMESS_ANY: {
          Long64_t length;
          TString filename;
          int clientId;
          message->ReadInt(clientId);
          message->ReadTString(filename);
          message->ReadLong64(length);
          delete transient;
          transient = new TMemFile(filename,message->Buffer()+message->Length(),length,"recreate");
          message->SetBufferOffset(message->Length()+length);
          if(!merger.OutputFile(fCurrentAnalysisFileName.c_str(),"update")) {
             printf("server fail to open outfile.\n");
             return ;
          }
          if(!merger.AddAdoptFile(transient)) {
             printf("server failed to adopt file.\n");
             return ;
          }
          if(!merger.PartialMerge(TFileMerger::kAllIncremental)) {
             printf("server failed to partial merge.\n");
             return ;
          }
          transient = NULL;
       }
       break;
       case kMESS_OBJECT: 
          printf("server recieved object of class %s\n",message->GetClass()->GetName());
          break;
       default:
          printf("server recieved unexpected message.\n");
          break;
      };
     delete message;    
   }
  return;
}

void TAnalysisTreeBuilder::SetupAnalysisClients() { 
   //Sets up the analysis tree by creating branches of the available detector systems. The available detector systems
   //are set in the RunInfo of the fragment tree. When the analysis tree sorting begins, an output is created on screen
   //that tells you which detector systems were found in the RunInfo.

   for(int x=0;x<TGRSIOptions::GetNumberOfWriteThreads();x++) {
     printf( "\n    i am here x = %i\n",x);
     fFillClientThreads[x] = new std::thread(&TAnalysisTreeBuilder::FillClient,this);
   }

   //Set new branches in the analysis tree if the run info in the fragment tree says the detectors are in the data stream
   return;  
}

void TAnalysisTreeBuilder::FillClient() {

   TSocket *socket = new TSocket("localhost",9099);
   if(!socket->IsValid()) {
      printf("client could not connect on localhost:9099\n");
      delete socket;
      return;
   }
   int clientnumber,version,kind;
   socket->Recv(clientnumber,kind);
   if(kind!=kStartConnection) {
     printf("[connect] unexpected server message: %i\n",kind);
     delete socket;
     return;
   }
   socket->Recv(version,kind);
   if(kind!=kProtocolVersion) {
      printf("[version] unexpected server message: %i\n",kind);
      delete socket;
      return;
   }
  
   printf("client[%i] connected to server.\n",clientnumber);

   float messafeLength           = 0;
   float compressedMessageLength = 0;
  
   TMemFile *file = new TMemFile(Form("MergeClient_%02i.root",clientnumber),"recreate");
   file->Print();

   if(!fCurrentRunInfo)
      return;
   TTree fClientAnalysisTree("AnalysisTree","AnalysisTree");
   TGRSIRunInfo *info = fCurrentRunInfo;

   TTigress *tigress = 0;//new TTigress;
   TSharc   *sharc   = 0;//new TSharc;
   TTriFoil *triFoil = 0;//new TTriFoil;
   //rf->Clear();
   TCSM     *csm     = 0;//new TCSM;
   //spice->Clear(); s3->Clear();
   //tip->Clear();

   TGriffin *griffin = 0;//new TGriffin;
   TSceptar *sceptar = 0;//new TSceptar;
   TPaces   *paces   = 0;//new TPaces;
   TDescant *descant = 0;//new TDescant;
   //dante->Clear();
   //zerodegree->Clear();
   if(info->Tigress())   { fClientAnalysisTree.Branch("TTigress","TTigress",&tigress);}//, basketSize); } 
   if(info->Sharc())     { fClientAnalysisTree.Branch("TSharc","TSharc",&sharc); }//, basketSize); } 
   if(info->TriFoil())   { fClientAnalysisTree.Branch("TTriFoil","TTriFoil",&triFoil);}//, basketSize); } 
   //if(info->Rf())        { tree->Bronch("TRf","TRf",&rf); }//, basketSize); } 
   if(info->CSM())       { fClientAnalysisTree.Branch("TCSM","TCSM",&csm); }//, basketSize); } 
   //if(info->Spice())     { tree->Bronch("TSpice","TSpice",&spice); }//, basketSize); 
   //tree->SetBronch("TS3","TS3",&s3); }//, basketSize); } 
   //if(info->Tip())       { tree->Bronch("TTip","TTip",&tip); }//, basketSize); } 

   if(info->Griffin())   { fClientAnalysisTree.Branch("TGriffin","TGriffin",&griffin);} 
   if(info->Sceptar())   { fClientAnalysisTree.Branch("TSceptar","TSceptar",&sceptar);} 
   if(info->Paces())     { fClientAnalysisTree.Branch("TPaces","TPaces",&paces);}//, basketSize); } 
   if(info->Descant())   { fClientAnalysisTree.Branch("TDescant","TDescant",&descant);}//, basketSize);
   //if(info->Dante())     { tree->Bronch("TDante","TDante",&dante); }//, basketSize); } 
   //if(info->ZeroDegree()){ tree->Bronch("TZeroDegree","TZeroDegree",&zerodegree); }//, basketSize); } 

   TMessage::EnableSchemaEvolutionForAll(true);
   TMessage message(kMESS_OBJECT);

   int NumberOfFills=0;
   while(TWriteQueue::Size() > 0 || TEventQueue::Size() > 0 || !fSortFragmentDone) {
      if(TWriteQueue::Size() == 0) {
         std::this_thread::sleep_for(std::chrono::milliseconds(100));
         continue;
      }
      std::map<const char*, TGRSIDetector*> *detectors = TWriteQueue::PopEntry();
  
      //fAnalysisOut++;
      AnalysisOutPlusPlus();
      //FillAnalysisTree(fClientAnalysisTree,detectors);

      for(auto det = detectors->begin(); det != detectors->end(); det++) {
         if(strcmp(det->first,"TI") == 0) {
            tigress = (TTigress*) det->second;
         } else if(strcmp(det->first,"SH") == 0) {
            sharc = (TSharc*) det->second;
         } else if(strcmp(det->first,"Tr") == 0) {
            triFoil = (TTriFoil*) det->second;
            //*rf = *((TRf*) det->second);
         } else if(strcmp(det->first,"CS") == 0) {
            csm = (TCSM*) det->second;
         //} else if(strcmp(det->second->IsA()->GetName(),"TSpice") == 0) {
            //*spice = *((TSpice*) det->second);
         //} else if(strcmp(det->second->IsA()->GetName(),"TTip") == 0) {
            //*tip = *((TTip*) det->second);
         } else if(strcmp(det->first,"GR") == 0) {
            griffin = (TGriffin*) det->second;
         } else if(strcmp(det->first,"SE") == 0) {
            sceptar = (TSceptar*) det->second;
         } else if(strcmp(det->first,"DS") == 0) {
            descant = (TDescant*) det->second;
         } else if(strcmp(det->first,"PA") == 0) {
            paces = (TPaces*) det->second;
         } 
      }
 
      fClientAnalysisTree.Fill();

      if(info->Tigress())   { tigress->Clear();} //
      if(info->Sharc())     { sharc->Clear(); }  //
      if(info->TriFoil())   { triFoil->Clear();} //
      //if(info->Rf())                           //
      if(info->CSM())       { csm->Clear(); }    //
      //if(info->Spice())     ice);              //
      //tree->SetBronch("TS3",
      //if(info->Tip())                          //

      if(info->Griffin())   { griffin->Clear();} 
      if(info->Sceptar())   { sceptar->Clear();} 
      if(info->Paces())     { paces->Clear();}   //
      if(info->Descant())   { descant->Clear();} //

      for(auto det = detectors->begin(); det != detectors->end(); det++) {
         delete det->second;
         det->second = 0;
      }
      delete detectors;
      detectors = 0;


      //ok, now the tree has been filed.  Next we check the how many times
      //the tree has been filled and update if it is large enough, send
      //it to the server.
      if((NumberOfFills&10000)==0) {
        file->Write();
        message.Reset(kMESS_ANY);
        message.WriteInt(clientnumber);
        message.WriteTString(file->GetName());
        message.WriteLong64(file->GetEND());
        file->CopyTo(message);
        socket->Send(message);
        file->ResetAfterMerge(0); 
      }
   }
   if(fClientAnalysisTree.GetEntries()>0) {
     file->Write();
     message.Reset(kMESS_ANY);
     message.WriteInt(clientnumber);
     message.WriteTString(file->GetName());
     message.WriteLong64(file->GetEND());
     file->CopyTo(message);
     socket->Send(message);
     file->ResetAfterMerge(0); 
   }

  socket->Send("Finished");
  printf("client exiting\n");
  socket->Close();
  return ;
}





/*
void TAnalysisTreeBuilder::ClearActiveAnalysisTreeBranches() {
   //Clears the current analysis tree branches.
   if(!fCurrentAnalysisFile || !fCurrentRunInfo)
      return;
   TGRSIRunInfo *info = fCurrentRunInfo;
   TTree *tree = fCurrentAnalysisTree;

   if(info->Tigress())   { tigress->Clear(); }
   if(info->Sharc())     { sharc->Clear(); }
   if(info->TriFoil())   { triFoil->Clear(); }
   //if(info->Rf())        { rf->Clear(); } 
   if(info->CSM())       { csm->Clear(); }
   //if(info->Spice())     { spice->Clear(); s3->Clear(); } 
   //if(info->Tip())       { tip->Clear(); } 
//printf("clearing griffin 0x08%x\n",griffin);
//griffin->Print();
   if(info->Griffin())   { griffin->Clear(); }
   if(info->Sceptar())   { sceptar->Clear(); }
   if(info->Paces())     { paces->Clear(); } 
   //if(info->Dante())     { dante->Clear(); } 
   //if(info->ZeroDegree()){ zerodegree->Clear(); } 
   if(info->Descant())   { descant->Clear();}
   //printf("ClearActiveAnalysisTreeBranches done\n");
}
*/
/*
void TAnalysisTreeBuilder::ResetActiveAnalysisTreeBranches(TTree *tree) {
   //Clears the current analysis tree branches.
   if(strcmp(tree->GetName(),"AnalysisTree") || !fCurrentRunInfo)
      return;
   TGRSIRunInfo *info = fCurrentRunInfo;
   //TTree *tree = fCurrentAnalysisTree;

   if(info->Tigress())   { tigress = 0; }//->Clear(); }
   if(info->Sharc())     { sharc = 0; }//->Clear(); }
   if(info->TriFoil())   { triFoil = 0; }//->Clear(); }
   //if(info->Rf())        { rf->Clear(); } 
   if(info->CSM())       { csm = 0; }//->Clear(); }
   //if(info->Spice())     { spice->Clear(); s3->Clear(); } 
   //if(info->Tip())       { tip->Clear(); } 
//printf("clearing griffin 0x08%x\n",griffin);
//griffin->Print();
   if(info->Griffin())   { griffin = 0; }//->Clear(); }
   if(info->Sceptar())   { sceptar = 0; }//->Clear(); }
   if(info->Paces())     { paces = 0; }//->Clear(); } 
   //if(info->Dante())     { dante->Clear(); } 
   //if(info->ZeroDegree()){ zerodegree->Clear(); } 
   if(info->Descant())   { descant = 0; }//->Clear();}
   //printf("ClearActiveAnalysisTreeBranches done\n");
}
*/








void TAnalysisTreeBuilder::BuildActiveAnalysisTreeBranches(std::map<const char*, TGRSIDetector*> *detectors) {
   //Build the hits in each of the detectors.
   if( !fCurrentRunInfo)
      return;
   TGRSIRunInfo *info = fCurrentRunInfo;

   for(auto det = detectors->begin(); det != detectors->end(); det++) {
      det->second->BuildHits();
   }
}

void TAnalysisTreeBuilder::FillWriteQueue(std::map<const char*, TGRSIDetector*> *detectors) {
   //Fill the write Q with the built hits in each of the detectors.
   fAnalysisIn++;
   TWriteQueue::Get()->Add(detectors);
}
/*
void TAnalysisTreeBuilder::WriteAnalysisTree() {
   //Write the analysis tree to the root file. We do this by filling the analysis tree with each
   //of the built events in the write Q.
   int counter = 0;
   while(TWriteQueue::Size() > 0 || TEventQueue::Size() > 0 || !fSortFragmentDone) {
      if(TWriteQueue::Size() == 0) {
         std::this_thread::sleep_for(std::chrono::milliseconds(100));
         continue;
      }
      std::map<const char*, TGRSIDetector*> *detectors = TWriteQueue::PopEntry();
      fAnalysisOut++;

      FillAnalysisTree(detectors);
   }
}
*/

void TAnalysisTreeBuilder::CloseAnalysisFile() {

   //what for the server to close....
   return ;




   //Safely close the analysis file.
//   if(!fCurrentAnalysisFile)
//      return;

   ///******************************////
   ///******************************////
   // this will be removed and put into a seperate thread later.

//   printf("Writing file %s\n",fCurrentAnalysisFile->GetName());
   ///******************************////
   ///******************************////

//   if(TChannel::GetNumberOfChannels()>0) {
//     TChannel *c = TChannel::GetDefaultChannel();
//     c->Write();
//   }
   //TChannel::DeleteAllChannels();

//   fCurrentAnalysisFile->cd();
//   if(fCurrentAnalysisTree)
//      fCurrentAnalysisTree->Write();
//   fCurrentAnalysisFile->Close();

//   fCurrentAnalysisTree = 0;
//   fCurrentAnalysisFile = 0;
//   return;
}


void TAnalysisTreeBuilder::ProcessEvent() {
   //Process the event. We do this by filling a map of the detectors, and put the hits into the appropriate detector class.
   //This is important because different detector systems will need different experimental data/conditions to be set.
   int counter = 0;
   while(TEventQueue::Size() > 0 || !fSortFragmentDone) {
      if(TEventQueue::Size() == 0) { 
         std::this_thread::sleep_for(std::chrono::milliseconds(100));
         continue; 
      }
      
      //We need to pull the event out of the Event Q
      std::vector<TFragment> *event = TEventQueue::PopEntry();
      MNEMONIC mnemonic;
      std::map<const char*, TGRSIDetector*> *detectors = new std::map<const char*, TGRSIDetector*>;
      for(int i=0;i<event->size();i++) {
      
         TChannel *channel = TChannel::GetChannel(event->at(i).ChannelAddress);
         if(!channel)
            continue;
         ClearMNEMONIC(&mnemonic);
         ParseMNEMONIC(channel->GetChannelName(),&mnemonic);
         
         //We use the MNEMONIC in order to figure out what detector we want to put the set of fragments into
         if(mnemonic.system.compare("TI")==0) {
            if(detectors->find("TI") == detectors->end()) {
               (*detectors)["TI"] = new TTigress;
            }
            (*detectors)["TI"]->FillData(&(event->at(i)),channel,&mnemonic);
         } else if(mnemonic.system.compare("SH")==0) {
            if(detectors->find("SH") == detectors->end()) {
               (*detectors)["SH"] = new TSharc;
            }
            (*detectors)["SH"]->FillData(&(event->at(i)),channel,&mnemonic);
         } else if(mnemonic.system.compare("Tr")==0) {	
            if(detectors->find("Tr") == detectors->end()) {
               (*detectors)["Tr"] = new TTriFoil;
            }
            (*detectors)["Tr"]->FillData(&(event->at(i)),channel,&mnemonic);
         } else if(mnemonic.system.compare("CS")==0) {	
            if(detectors->find("CS") == detectors->end()) {
               (*detectors)["CS"] = new TCSM;
            }
            (*detectors)["CS"]->FillData(&(event->at(i)),channel,&mnemonic);
         } else if(mnemonic.system.compare("GR")==0) {	
            if(detectors->find("GR") == detectors->end()) {
               (*detectors)["GR"] = new TGriffin;
            }
            (*detectors)["GR"]->FillData(&(event->at(i)),channel,&mnemonic);
         } else if(mnemonic.system.compare("SE")==0) {	
            if(detectors->find("SE") == detectors->end()) {
               (*detectors)["SE"] = new TSceptar;
            }
            (*detectors)["SE"]->FillData(&(event->at(i)),channel,&mnemonic);
         } else if(mnemonic.system.compare("PA")==0) {	
            if(detectors->find("PA") == detectors->end()) {
               (*detectors)["PA"] = new TPaces;
            }
            (*detectors)["PA"]->FillData(&(event->at(i)),channel,&mnemonic);
         } else if(mnemonic.system.compare("DS")==0) {	
            if(detectors->find("DS") == detectors->end()) {
               (*detectors)["DS"] = new TDescant;
            }
            (*detectors)["DS"]->FillData(&(event->at(i)),channel,&mnemonic);
         //else if(mnemonic.system.compare("PA")==0) {	
         //	FillData(&(event->at(i)),channel,&mnemonic);
         //} else if(mnemonic.system.compare("DA")==0) {	
         //	FillData(&(event->at(i)),channel,&mnemonic);
         //} else if(mnemonic.system.compare("ZD")==0) {	
         //	FillData(&(event->at(i)),channel,&mnemonic);
         //} else if(mnemonic.system.compare("DE")==0) {	
         //	FillData(&(event->at(i)),channel,&mnemonic);
         }
      }

      if(!detectors->empty()) {
         BuildActiveAnalysisTreeBranches(detectors);
         FillWriteQueue(detectors);
      }

      delete event;
   
   }
}

void TAnalysisTreeBuilder::Print(Option_t *opt) {
   //Prints information about the Q's and threads in the AnalysisTreeBuilding process
//   if(fCurrentFragFile && fCurrentAnalysisFile)
//      printf(DMAGENTA " %s/%s" RESET_COLOR  "\n",fCurrentFragFile->GetName(),fCurrentAnalysisFile->GetName());
   printf(DMAGENTA " fSortFragmentDone         = %s" RESET_COLOR "\n",fSortFragmentDone ? "true":"false");
   printf(DYELLOW  " TEventQueue::Size()       = %i" RESET_COLOR "\n",TEventQueue::Size());
   printf(DBLUE    " TWriteQueue::Size()       = %i" RESET_COLOR "\n",TWriteQueue::Size());
   printf(DGREEN   " fFragmentsIn/fAnalysisOut = %i / %i" RESET_COLOR "\n",fFragmentsIn,AnalysisOut());  
   printf(GREEN    " std::thread::hardware_concurrency = %u" RESET_COLOR "\n",std::thread::hardware_concurrency());
   printf(DMAGENTA " ==========================================" RESET_COLOR "\n");
   return;
}

void TAnalysisTreeBuilder::Status() {
   //Prints the current status for each of the threads in the TAnalysisTree Building process
   TStopwatch w;
   w.Start();
   bool fragmentsDone = false;
   bool sortingDone = false;
   while(fPrintStatus) {
//      printf(DYELLOW HIDE_CURSOR "%12i / %12ld " RESET_COLOR "/" DBLUE " %12i " RESET_COLOR "/" DCYAN " %12i " RESET_COLOR "/" DRED " %12i " RESET_COLOR "/" DGREEN " %12i " RESET_COLOR
//             "    processed fragments / # of fragments/ # of events / event queue size / write queue size / events written.\t%.1f seconds." SHOW_CURSOR "\r",
//             fFragmentsIn, fEntries, fAnalysisIn, TEventQueue::Size(), TWriteQueue::Size(), fAnalysisOut, w.RealTime());
      if(!sortingDone) {
         printf(DYELLOW HIDE_CURSOR "Fragments: %.1f %%," DBLUE "   %9i built events," DRED "   written: %9i = %.1f %%," 
                DGREEN "   write speed: %9.1f built events/second." RESET_COLOR " %3.1f seconds." SHOW_CURSOR "\r",
               (100.*fFragmentsIn)/fEntries, fAnalysisIn, AnalysisOut(), (100.*AnalysisOut())/fAnalysisIn, AnalysisOut()/w.RealTime(), w.RealTime());
      } else {
         if(AnalysisOut() > 0) {
            printf(DYELLOW HIDE_CURSOR "Fragments: %.1f %%," DBLUE "   %9i built events," DRED "   written: %9i = %.1f %%," 
                   DGREEN "   write speed: %9.1f built events/second." RESET_COLOR "  %.1f seconds, %.1f seconds remaining." SHOW_CURSOR "\r",
                  (100.*fFragmentsIn)/fEntries, fAnalysisIn, AnalysisOut(), (100.*AnalysisOut())/fAnalysisIn, AnalysisOut()/w.RealTime(), w.RealTime(), ((double)(fAnalysisIn-AnalysisOut()))/AnalysisOut()*w.RealTime());
         } else {
            printf(DYELLOW HIDE_CURSOR "Fragments: %.1f %%," DBLUE "   %9i built events," DRED "   written: %9i = %.1f %%," 
                   DGREEN "   write speed: %9.1f built events/second." RESET_COLOR " %3.1f seconds." SHOW_CURSOR "\r",
                  (100.*fFragmentsIn)/fEntries, fAnalysisIn, AnalysisOut(), (100.*AnalysisOut())/fAnalysisIn, AnalysisOut()/w.RealTime(), w.RealTime());
         }
      }
      //we insert a newline (thus preserving the last status), if we just finished getting all fragment, or finished removing fragments from the event queue
      if(!fragmentsDone && fFragmentsIn == fEntries && fEntries != 0) {
         printf("\n");
         fragmentsDone = true;
      }
      if(fragmentsDone && !sortingDone && TEventQueue::Size() == 0) {
         printf("\n");
         sortingDone = true;
      }
      w.Continue(); 
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   }
//   printf(DYELLOW HIDE_CURSOR "%12i / %12ld " RESET_COLOR "/" DBLUE " %12i " RESET_COLOR "/" DCYAN " %12i " RESET_COLOR "/" DRED " %12i " RESET_COLOR "/" DGREEN " %12i " RESET_COLOR
//          "    processed fragments / # of fragments/ # of events / event queue size / write queue size / events written.\t%.1f seconds." SHOW_CURSOR "\n",
//          fFragmentsIn, fEntries, fAnalysisIn, TEventQueue::Size(), TWriteQueue::Size(), fAnalysisOut, w.RealTime());
   if(AnalysisOut() > 0) {
      printf(DYELLOW HIDE_CURSOR "Fragments: %.1f %%," DBLUE "   %9i built events," DRED "   written: %9i = %.1f %%," 
             DGREEN "   write speed: %9.1f built events/second." RESET_COLOR "  %.1f seconds, %.1f seconds remaining." SHOW_CURSOR "\r",
            (100.*fFragmentsIn)/fEntries, fAnalysisIn, AnalysisOut(), (100.*AnalysisOut())/fAnalysisIn, AnalysisOut()/w.RealTime(), w.RealTime(), ((double)(fAnalysisIn-AnalysisOut()))/AnalysisOut()*w.RealTime());
   } else {
      printf(DYELLOW HIDE_CURSOR "Fragments: %.1f %%," DBLUE "   %9i built events," DRED "   written: %9i = %.1f %%," 
             DGREEN "   write speed: %9.1f built events/second." RESET_COLOR " %3.1f seconds." SHOW_CURSOR "\r",
            (100.*fFragmentsIn)/fEntries, fAnalysisIn, AnalysisOut(), (100.*AnalysisOut())/fAnalysisIn, AnalysisOut()/w.RealTime(), w.RealTime());
   }

   return;
}


