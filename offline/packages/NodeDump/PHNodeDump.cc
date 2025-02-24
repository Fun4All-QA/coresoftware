#include "PHNodeDump.h"
#include "DumpObject.h"

#include "DumpBbcVertexMap.h"
#include "DumpCaloTriggerInfo.h"
#include "DumpCentralityInfo.h"
#include "DumpEventHeader.h"
#include "DumpGlobalVertexMap.h"
#include "DumpInttDeadMap.h"
#include "DumpJetMap.h"
#include "DumpParticleFlowElementContainer.h"
#include "DumpPHFieldConfig.h"
#include "DumpPHG4BlockCellGeomContainer.h"
#include "DumpPHG4BlockGeomContainer.h"
#include "DumpPHG4CellContainer.h"
#include "DumpPHG4CylinderCellContainer.h"
#include "DumpPHG4CylinderCellGeomContainer.h"
#include "DumpPHG4CylinderGeomContainer.h"
#include "DumpPHG4HitContainer.h"
#include "DumpPHG4InEvent.h"
#include "DumpPHG4ParticleSvtxMap.h"
#include "DumpPHG4ScintillatorSlatContainer.h"
#include "DumpPHG4TruthInfoContainer.h"
#include "DumpPHHepMCGenEventMap.h"
#include "DumpPdbParameterMap.h"
#include "DumpPdbParameterMapContainer.h"
#include "DumpRawClusterContainer.h"
#include "DumpRawTowerContainer.h"
#include "DumpRawTowerGeomContainer.h"
#include "DumpRunHeader.h"
#include "DumpSvtxPHG4ParticleMap.h"
#include "DumpSvtxTrackMap.h"
#include "DumpSvtxVertexMap.h"
#include "DumpSyncObject.h"
#include "DumpTowerBackground.h"
#include "DumpTpcSeedTrackMap.h"
#include "DumpTrkrClusterContainer.h"
#include "DumpTrkrClusterCrossingAssoc.h"
#include "DumpTrkrClusterHitAssoc.h"
#include "DumpTrkrHitSetContainer.h"
#include "DumpTrkrHitTruthAssoc.h"
#include "DumpVariableArray.h"

#include <phool/PHIODataNode.h>
#include <phool/PHNode.h>
#include <phool/getClass.h>
#include <phool/phool.h>

#include <ffaobjects/RunHeader.h>

#include <TObject.h>

#include <iostream>
#include <string>
#include <utility>

using namespace std;

PHNodeDump::PHNodeDump()
  : runnumber(-9999)
  , evtsequence(-9999)
  , fp_precision(-1)
  , outdir("./")
{
}

PHNodeDump::~PHNodeDump()
{
  ignore.clear();
  exclusive.clear();
  while (dumpthis.begin() != dumpthis.end())
  {
    delete dumpthis.begin()->second;
    dumpthis.erase(dumpthis.begin());
  }
  return;
}

int PHNodeDump::AddIgnore(const string &name)
{
  if (ignore.find(name) != ignore.end())
  {
    cout << PHWHERE << " "
         << name << "already in ignore list" << endl;
    return -1;
  }
  ignore.insert(name);
  return 0;
}

int PHNodeDump::Select(const string &name)
{
  if (exclusive.find(name) != exclusive.end())
  {
    cout << PHWHERE << " "
         << name << "already in exclusive list" << endl;
    return -1;
  }
  exclusive.insert(name);
  return 0;
}

int PHNodeDump::GetGlobalVars(PHCompositeNode *topNode)
{
  RunHeader *runheader = findNode::getClass<RunHeader>(topNode, "RunHeader");
  if (runheader)
  {
    runnumber = runheader->get_RunNumber();
  }
  return 0;
}

void PHNodeDump::perform(PHNode *node)
{
  map<string, DumpObject *>::iterator iter;
  if (node->getType() == "PHIODataNode")
  {
    string NodeName = node->getName();
    iter = dumpthis.find(NodeName);
    if (iter == dumpthis.end())
    {
      cout << "Adding Dump Object for " << NodeName << endl;
      AddDumpObject(NodeName, node);
      iter = dumpthis.find(NodeName);  // update iterator
    }

    if (iter != dumpthis.end())
    {
      iter->second->process_event(node);
    }
    else
    {
      //           for (iter = dumpthis.begin(); iter != dumpthis.end(); iter++)
      //             {
      //               cout << "registered: " << iter->second->Name() << endl;
      //             }
      cout << "Something went wrong with adding Dump Object for " << NodeName
           << ", it should exist !! Trying to create it again" << endl;
      AddDumpObject(NodeName, node);
    }
  }
  return;
}

int PHNodeDump::CloseOutputFiles()
{
  map<string, DumpObject *>::iterator iter;
  for (iter = dumpthis.begin(); iter != dumpthis.end(); ++iter)
  {
    iter->second->CloseOutputFile();
  }
  return 0;
}

int PHNodeDump::AddDumpObject(const string &NodeName, PHNode *node)
{
  DumpObject *newdump;
  string newnode = NodeName;
  if (!exclusive.empty())
  {
    if (exclusive.find(NodeName) == exclusive.end())
    {
      cout << "Exclusive find: Ignoring " << NodeName << endl;
      newdump = new DumpObject(NodeName);
      newdump->NoOutput();
      goto initdump;
    }
  }
  if (ignore.find(NodeName) != ignore.end())
  {
    cout << "Ignoring " << NodeName << endl;
    newdump = new DumpObject(NodeName);
  }
  else
  {
    if (node->getType() == "PHIODataNode")
    {
      // need a static cast since only from DST these guys are of type PHIODataNode<TObject*>
      // when created they are normally  PHIODataNode<PHObject*> but can be anything else as well
      TObject *tmp = (TObject *) (static_cast<PHIODataNode<TObject> *>(node))->getData();
      if (tmp->InheritsFrom("BbcVertexMap"))
      {
        newdump = new DumpBbcVertexMap(NodeName);
      }
      else if (tmp->InheritsFrom("CaloTriggerInfo"))
      {
        newdump = new DumpCaloTriggerInfo(NodeName);
      }
      else if (tmp->InheritsFrom("CentralityInfo"))
      {
        newdump = new DumpCentralityInfo(NodeName);
      }
      else if (tmp->InheritsFrom("EventHeader"))
      {
        newdump = new DumpEventHeader(NodeName);
      }
      else if (tmp->InheritsFrom("GlobalVertexMap"))
      {
        newdump = new DumpGlobalVertexMap(NodeName);
      }
      else if (tmp->InheritsFrom("InttDeadMap"))
      {
        newdump = new DumpInttDeadMap(NodeName);
      }
      else if (tmp->InheritsFrom("JetMap"))
      {
        newdump = new DumpJetMap(NodeName);
      }
      else if (tmp->InheritsFrom("ParticleFlowElementContainer"))
      {
        newdump = new DumpParticleFlowElementContainer(NodeName);
      }
      else if (tmp->InheritsFrom("PdbParameterMap"))
      {
        newdump = new DumpPdbParameterMap(NodeName);
      }
      else if (tmp->InheritsFrom("PdbParameterMapContainer"))
      {
        newdump = new DumpPdbParameterMapContainer(NodeName);
      }
      else if (tmp->InheritsFrom("PHFieldConfig"))
      {
        newdump = new DumpPHFieldConfig(NodeName);
      }
      else if (tmp->InheritsFrom("PHG4BlockGeomContainer"))
      {
        newdump = new DumpPHG4BlockGeomContainer(NodeName);
      }
      else if (tmp->InheritsFrom("PHG4BlockCellGeomContainer"))
      {
        newdump = new DumpPHG4BlockCellGeomContainer(NodeName);
      }
      else if (tmp->InheritsFrom("PHG4CellContainer"))
      {
        newdump = new DumpPHG4CellContainer(NodeName);
      }
      else if (tmp->InheritsFrom("PHG4CylinderCellContainer"))
      {
        newdump = new DumpPHG4CylinderCellContainer(NodeName);
      }
      else if (tmp->InheritsFrom("PHG4CylinderGeomContainer"))
      {
        newdump = new DumpPHG4CylinderGeomContainer(NodeName);
      }
      else if (tmp->InheritsFrom("PHG4CylinderCellGeomContainer"))
      {
        newdump = new DumpPHG4CylinderCellGeomContainer(NodeName);
      }
      else if (tmp->InheritsFrom("PHG4HitContainer"))
      {
        newdump = new DumpPHG4HitContainer(NodeName);
      }
      else if (tmp->InheritsFrom("PHG4InEvent"))
      {
        newdump = new DumpPHG4InEvent(NodeName);
      }
      else if (tmp->InheritsFrom("PHG4ParticleSvtxMap"))
      {
        newdump = new DumpPHG4ParticleSvtxMap(NodeName);
      }
      else if (tmp->InheritsFrom("PHG4ScintillatorSlatContainer"))
      {
        newdump = new DumpPHG4ScintillatorSlatContainer(NodeName);
      }
      else if (tmp->InheritsFrom("PHG4TruthInfoContainer"))
      {
        newdump = new DumpPHG4TruthInfoContainer(NodeName);
      }
      else if (tmp->InheritsFrom("PHHepMCGenEventMap"))
      {
        newdump = new DumpPHHepMCGenEventMap(NodeName);
      }
      else if (tmp->InheritsFrom("RawClusterContainer"))
      {
        newdump = new DumpRawClusterContainer(NodeName);
      }
      else if (tmp->InheritsFrom("RawTowerContainer"))
      {
        newdump = new DumpRawTowerContainer(NodeName);
      }
      else if (tmp->InheritsFrom("RawTowerGeomContainer"))
      {
        newdump = new DumpRawTowerGeomContainer(NodeName);
      }
      else if (tmp->InheritsFrom("RunHeader"))
      {
        newdump = new DumpRunHeader(NodeName);
      }
      else if (tmp->InheritsFrom("SvtxPHG4ParticleMap"))
      {
        newdump = new DumpSvtxPHG4ParticleMap(NodeName);
      }
      else if (tmp->InheritsFrom("SvtxTrackMap"))
      {
        newdump = new DumpSvtxTrackMap(NodeName);
      }
      else if (tmp->InheritsFrom("SvtxVertexMap"))
      {
        newdump = new DumpSvtxVertexMap(NodeName);
      }
      else if (tmp->InheritsFrom("SyncObject"))
      {
        newdump = new DumpSyncObject(NodeName);
      }
      else if (tmp->InheritsFrom("TowerBackground"))
      {
        newdump = new DumpTowerBackground(NodeName);
      }
      else if (tmp->InheritsFrom("TpcSeedTrackMap"))
      {
        newdump = new DumpTpcSeedTrackMap(NodeName);
      }
      else if (tmp->InheritsFrom("TrkrClusterContainer"))
      {
        newdump = new DumpTrkrClusterContainer(NodeName);
      }
      else if (tmp->InheritsFrom("TrkrClusterCrossingAssoc"))
      {
        newdump = new DumpTrkrClusterCrossingAssoc(NodeName);
      }
      else if (tmp->InheritsFrom("TrkrClusterHitAssoc"))
      {
        newdump = new DumpTrkrClusterHitAssoc(NodeName);
      }
      else if (tmp->InheritsFrom("TrkrHitSetContainer"))
      {
        newdump = new DumpTrkrHitSetContainer(NodeName);
      }
      else if (tmp->InheritsFrom("TrkrHitTruthAssoc"))
      {
        newdump = new DumpTrkrHitTruthAssoc(NodeName);
      }
      else if (tmp->InheritsFrom("VariableArray"))
      {
        newdump = new DumpVariableArray(NodeName);
      }
      else
      {
        cout << "Registering Dummy for " << NodeName
             << ", Class: " << tmp->ClassName() << endl;
        newdump = new DumpObject(NodeName);
      }
    }
    else
    {
      cout << "ignoring PHDataNode: " << NodeName << endl;
      newdump = new DumpObject(NodeName);
    }
  }

initdump:
  newdump->SetParentNodeDump(this);
  newdump->SetOutDir(outdir);
  newdump->SetPrecision(fp_precision);
  newdump->Init();
  dumpthis[newnode] = newdump;
  return 0;
}

int PHNodeDump::SetOutDir(const string &dirname)
{
  outdir = dirname;
  return 0;
}
