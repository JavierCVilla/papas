//
//  Created by Alice Robson on 09/11/15.
//
//
#include <iostream> //TODOAJR remove
#include "Simulator.h"
#include "CMS.h"
#include "Identifier.h"
#include "enums.h"

using fastsim::enumLayer;

Simulator::Simulator(const BaseDetector& d)
: m_detector(d), m_propStraight(), m_propHelix(d.field()->getMagnitude())
{
  //TODO think about sizing
  m_ECALClusters.reserve(1000);
  m_HCALClusters.reserve(1000);
  m_smearedECALClusters.reserve(1000);
  m_smearedHCALClusters.reserve(1000);
  m_particles.reserve(1000);
  m_tracks.reserve(1000);
  m_smearedTracks.reserve(1000);
  m_nodes.reserve(1000);
  
}

void  Simulator::simulatePhoton(PFParticle& ptc)
{
  auto ecal_sp=m_detector.ECAL();
  
  //propagate as far as the ECAL inner cylinder
  propagate(ptc, ecal_sp->volumeCylinder().Inner());
  
  //make a cluster
  long ecalID = addECALCluster(ptc); //add the position into this
  
  //smear the cluster
  addSmearedCluster(ecalID);
}

void  Simulator::simulateHadron(PFParticle& ptc)
{
  auto ecal_sp=m_detector.ECAL();
  auto hcal_sp=m_detector.HCAL();
  auto field_sp=m_detector.field();
  double frac_ecal =0.; //TODO ask Colin
  
  //make a track
  const Track& track=addTrack(ptc);
  addSmearedTrack(track);
  
  //propagate to the inner ECAL cyclinder
  propagate(ptc, ecal_sp->volumeCylinder().Inner());
  double path_length=ecal_sp->getMaterial().pathLength(ptc.isElectroMagnetic());
  
  //TO MATCH PYTHON$
  //path_length=0.273579537605;
  
  
  if (path_length < std::numeric_limits<double>::max()) {
    
    /// ecal path length can be infinite in case the ecal
    /// has lambda_I = 0 (fully transparent to hadrons)
    std::shared_ptr<Path> path = ptc.path();
    double time_ecal_inner = path->timeAtZ(path->namedPoint("_ECALin").Z());
    double deltat = path->deltaT(path_length);
    double time_decay = time_ecal_inner + deltat;
    TVector3 point_decay = path->pointAtTime(time_decay);
    path->addPoint("ecal_decay", point_decay);
    if (ecal_sp->volumeCylinder().Contains(point_decay))
    {
      //TODO reinstate after testingdouble frac_ecal = randomgen::randomUniform(0., 0.7).next(); // could also have a static member number generator
      frac_ecal=0.35;
      long ecalID = addECALCluster(ptc, track.id(), frac_ecal);
      //For now, using the hcal resolution and acceptance for hadronic cluster
      //in the ECAL. That's not a bug!
      addSmearedCluster(ecalID); // TODO to be revised by COlin
                                 //name of call needs to be improved
    }
  }
  
  //now move into HCAL
  propagate(ptc, hcal_sp->volumeCylinder().Inner());
  //TODO ask colin if the parent is the track or the ecalclust
  long hcalID = addHCALCluster(ptc, track.id(), 1-frac_ecal);
  addSmearedCluster(hcalID);
  
  
}

void  Simulator::propagate(PFParticle& ptc, const SurfaceCylinder & sc)
{
  bool is_neutral= fabs(ptc.charge())<0.5; //TODO ask colin why not zero
  if (is_neutral)
    m_propStraight.propagateOne(ptc, sc);
  else
    m_propHelix.propagateOne(ptc, sc);
  
}


const Cluster& Simulator::cluster(long clusterID) const{
  
  auto layer=Identifier::layer(clusterID);
  if (layer==fastsim::enumLayer::ECAL)
    return m_ECALClusters.at(clusterID);
  else
    return m_HCALClusters.at(clusterID);
  
}


PFParticle& Simulator::addParticle( int pdgid, TLorentzVector tlv, TVector3 vertex)
{
  double field = m_detector.field()->getMagnitude();
  long uniqueid = Identifier::makeParticleID(fastsim::enumSource::SIMULATION);
  m_particles.emplace(uniqueid,PFParticle{uniqueid, pdgid, tlv, vertex, field});
  addNode(uniqueid); //add node to graph
  return m_particles[uniqueid];
}

PFParticle& Simulator::addParticle( int pdgid, double theta, double phi, double energy, TVector3 vertex)
{
  TLorentzVector tlv = makeTLorentzVector(pdgid, theta, phi, energy);
  return addParticle( pdgid, tlv, vertex);
}



long Simulator::addECALCluster(PFParticle& ptc,long parentid,double fraction, double csize)
{
  Cluster cluster = makeCluster(ptc, parentid, fastsim::enumLayer::ECAL, fraction, csize);
  m_ECALClusters.emplace(cluster.id(), std::move(cluster));
  return cluster.id();
}

long Simulator::addHCALCluster(PFParticle& ptc,long parentid,double fraction, double csize)
{
  Cluster cluster = makeCluster(ptc, parentid, fastsim::enumLayer::HCAL, fraction, csize);
  m_HCALClusters.emplace(cluster.id(), std::move(cluster));
  return cluster.id();
}


Cluster Simulator::makeCluster(PFParticle& ptc, long parentid,fastsim::enumLayer layer, double fraction, double csize)
{
  if (!parentid) {
    parentid=ptc.id();
  }
  long clusterid = Identifier::makeClusterID(layer, fastsim::enumSubtype::RAW);
  double energy = ptc.p4().E() * fraction;
  
  //TODO change string to ENUM
  std::string cylname = m_detector.calorimeter(layer)->volumeCylinder().InnerName();
  TVector3 pos = ptc.pathPosition(cylname); //assume path already set in particle
  
  if (csize == 0) { //or could make the defalt value -1?? check Colin
    csize = m_detector.calorimeter(layer)->clusterSize(ptc);
  }
  
  const Cluster& cluster{energy, pos, csize, clusterid};
  addNode(clusterid, parentid); //a track may be the parent of a cluster
  return cluster; //check this defaults OK
}


long Simulator::addSmearedCluster(long parentClusterID)
{
  
  Cluster smeared=makeSmearedCluster(parentClusterID);
  auto layer = Identifier::layer(parentClusterID);
  
  if (m_detector.calorimeter(layer)->acceptance(smeared)) {
    addNode(smeared.id(), parentClusterID);
    if(layer ==fastsim::enumLayer::ECAL) {
      m_smearedECALClusters.emplace(smeared.id(), std::move(smeared));
    }
    else
      m_smearedHCALClusters.emplace(smeared.id(), std::move(smeared));
    return smeared.id();
  }
  else {
    //cluster.erase(smeared.id());
    return parentClusterID;
  }
}


Cluster Simulator::makeSmearedCluster( long parentClusterID) //, double energyresolution )
{
  //create a new id
  auto layer=Identifier::layer(parentClusterID);
  long newclusterid = Identifier::makeClusterID(layer, fastsim::enumSubtype::SMEARED);
  const Cluster& parent = cluster(parentClusterID);
  
  std::shared_ptr<const Calorimeter> sp_calorimeter = m_detector.calorimeter(layer);
  //double energyresolution = sp_calorimeter->energyResolution(parent.energy());
  //TODO double energy = parent.energy() * fastsim::randomGaussian(1, energyresolution).next();
  //double energy = parent.energy() * randomgen::RandNormal(1, energyresolution).next();
  double energy = parent.energy() * 0.95;
  
  Cluster cluster = Cluster{ energy, parent.position(), parent.size(), newclusterid};
  return cluster;
}

const Track& Simulator::addTrack(PFParticle& ptc)
{
  long trackid = Identifier::makeTrackID();
  m_tracks.emplace(trackid, Track{ ptc.p3(), ptc.charge(), ptc.path(), trackid});
  addNode(trackid, ptc.id());
  return m_tracks.at(trackid); //check this defaults OK
}


long Simulator::addSmearedTrack( const Track& track, bool accept) {
  long smearedTrackID = Identifier::makeTrackID(fastsim::enumSubtype::SMEARED);
  //double ptResolution = m_detector.tracker()->ptResolution(track);
  //TODO after testing double scale_factor = randomgen::RandNormal(1, ptResolution).next()
  double scale_factor = 1.1;
  Track smeared = Track{ track.p3() * scale_factor, track.charge(), track.path(), smearedTrackID};
  
  if (m_detector.tracker()->acceptance(smeared) || accept ) {
    addNode(smearedTrackID, track.id());
    m_smearedTracks.emplace(smearedTrackID, std::move(smeared));
  }
  return smearedTrackID;
}


void Simulator::addNode(long newid, const long parentid)
{
  
  //add the new node into the set of all nodes
  m_nodes[newid] = {newid};
  
  if (parentid) {
    Node& parent = m_nodes[parentid];
    Node& child = m_nodes[newid];
    parent.addChild(child);
  }
}

std::shared_ptr<const DetectorElement> Simulator::elem(fastsim::enumLayer layer) const
{
  return m_detector.element(layer);
}

void Simulator::testing()
{
  
  DAG::BFSVisitor<Node> bfs;
  for ( auto p  : m_particles)
  {
    std::cout<< "Connected to "<<p.first<< std::endl;
    auto res =bfs.traverseUndirected(m_nodes[p.first]);
    for (auto r : res)
      std::cout<< "  "<<r->value()<<": "<<Identifier::dataType(r->value()) <<": " <<Identifier::subType(r->value())<<std::endl;
  }
}

IDs Simulator::linkedECALSmearedClusterIDs(long nodeid) const {
  return getMatchingIDs(nodeid,
                        fastsim::enumDataType::CLUSTER,
                        fastsim::enumLayer::ECAL,
                        fastsim::enumSubtype::SMEARED,
                        fastsim::enumSource::SIMULATION);
  //return ids;
}

IDs  Simulator::linkedRawTrackIDs(long nodeid) const {
  return getMatchingIDs(nodeid,
                        fastsim::enumDataType::TRACK,
                        fastsim::enumLayer::NONE,
                        fastsim::enumSubtype::RAW,
                        fastsim::enumSource::SIMULATION);
}

IDs  Simulator::linkedSmearedTrackIDs(long nodeid) const {
  return getMatchingIDs(nodeid,
                        fastsim::enumDataType::TRACK,
                        fastsim::enumLayer::NONE,
                        fastsim::enumSubtype::SMEARED,
                        fastsim::enumSource::SIMULATION);
}

IDs  Simulator::linkedParticleIDs(long nodeid) const {
  return getMatchingIDs(nodeid,
                        fastsim::enumDataType::PARTICLE,
                        fastsim::enumLayer::NONE,
                        fastsim::enumSubtype::RAW,
                        fastsim::enumSource::SIMULATION);
}

IDs  Simulator::parentParticleIDs(long nodeid) const {
  return getMatchingParentIDs(nodeid,
                              fastsim::enumDataType::PARTICLE,
                              fastsim::enumLayer::NONE,
                              fastsim::enumSubtype::RAW,
                              fastsim::enumSource::SIMULATION);
}

IDs  Simulator::linkedIDs(long nodeid) const {
  DAG::BFSVisitor<Node> bfs;
  IDs foundids;
  foundids.reserve(1000); //TODO how
  auto res = bfs.traverseUndirected(m_nodes.at(nodeid));
  for (auto r : res)
  {
    foundids.push_back(r->value());
  }
  return foundids;
}

IDs Simulator::getMatchingIDs(long nodeid, fastsim::enumDataType datatype, fastsim::enumLayer layer, fastsim::enumSubtype type, fastsim::enumSource source) const
{
  DAG::BFSVisitor<Node> bfs;
  IDs foundids;
  //foundids.reserve(1000); //TODO set sizes sensible.... how
  auto res =bfs.traverseUndirected(m_nodes.at(nodeid));
  for (auto r : res)
  {
    long id=r->value();
    if(Identifier::isUniqueIDMatch(id, datatype,layer, type,source)) {
      foundids.push_back(id);
    }
  }
  return foundids;
}

IDs Simulator::getMatchingParentIDs(long nodeid, fastsim::enumDataType datatype, fastsim::enumLayer layer, fastsim::enumSubtype type,fastsim::enumSource source) const
{
  DAG::BFSVisitor<Node> bfs;
  IDs foundids;
  //foundids.reserve(1000); //TODO set sizes sensible.... how
  auto res =bfs.traverseParents(m_nodes.at(nodeid));
  for (auto r : res)
  {
    long id=r->value();
    if(Identifier::isUniqueIDMatch(id, datatype,layer, type,source)) {
      foundids.push_back(id);
    }
  }
  return foundids;
}







