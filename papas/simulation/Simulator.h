#ifndef Simulator_h
#define Simulator_h

#include "papas/datatypes/Cluster.h"
#include "papas/datatypes/Definitions.h"
#include "papas/datatypes/DefinitionsCollections.h"
#include "papas/datatypes/Particle.h"
#include "papas/datatypes/Track.h"
#include "papas/graphtools/DefinitionsNodes.h"

// forward declarations
class TVector3;
class TLorentzVector;

namespace papas {
// forward declarations
class HelixPropagator;
class StraightLinePropagator;
class Propagator;
class Event;
class Detector;
class DetectorElement;
class SurfaceCylinder;

/** Simulator class simulates particles and keeps a collection of the Particles and resulting Clusters, Tracks and
 of the relations (history) between them
 The Tracks and Clusters are created and then smeared (and stored separately). Clusters are separated into
 Ecal and Hcal collections.
 Simulator supports simulation of neutral and charged Hadrons and Photons;
 */
class Simulator {

public:
  /** Constructor
   @param[in] event papas Event structure (TODO double check its needed)
   @param[in] detector Detector
   @param[in] ecalClusters structure into which new Ecals are stored
   @param[in] hcalClusters structure into which new Hcals are stored
   @param[in] smearedEcalClusters structure into which new smearedEcals are stored
   @param[in] smearedHcalClusters structure into which new smeared Hcals are stored
   @param[in] tracks structure into which new tracks are stored
   @param[in] smearedtracks structure into which new  smearedtracks are stored
   @param[in] (papas) particles to which simulation information will be added
   @param[in] history structure into which new history can be added, may be empty at start
  */
  Simulator(const Event& event, char particleSubtype, const Detector& detector, Clusters& ecalClusters,
            Clusters& hcalClusters, Clusters& smearedEcalClusters, Clusters& smearedHcalClusters, Tracks& tracks,
            Tracks& smearedtracks, Nodes& history);

  /** Simulate a particle, to produce tracks, smearedtracks, clusters, smearedclusters and path info
   @param[in] ptc the particle to be simulated
  */
  void simulateParticle(const Particle& ptc);

  /* Find the cluster with the specified identifier
   @param[in] clusterId the identifier of the cluster
   */
  const Cluster& cluster(Identifier clusterId) const;
  const Clusters& ecalClusters() const { return m_ecalClusters; }                ///<return Ecal clusters collection
  const Clusters& hcalClusters() const { return m_hcalClusters; }                ///<return Hcal clusters collection
  const Clusters& smearedEcalClusters() const { return m_smearedEcalClusters; }  ///<return smeared Ecal clusters coll.
  const Clusters& smearedHcalClusters() const { return m_smearedHcalClusters; }  ///<return smeared Hcal clusters coll.
  const Tracks& tracks() const { return m_tracks; }                              ///<return tracks collection
  const Tracks& smearedTracks() const { return m_smearedTracks; }                ///<return smeared tracks collection
  Nodes& history() { return m_history; }  /// return a reference to history nodes collection
  // const Particles& particles() const { return m_particles; }  ///< Return particles collection
  void clear();  ///< Clear all the collections of clusters, particles, tracks

  /**
   Smears a Cluster
   @param[in] cluster the cluster that is to be smeared
   @param[in] detectorLayer the layer to be used for smearing. Note this is not always the same as the layer to which
   the cluster belongs
   @return the smeared Cluster (moved)
   */
  Cluster smearCluster(const Cluster& cluster,
                       papas::Layer detectorLayer = papas::Layer::kNone);  ///<randomise cluster energy

  /** TODO move to gun example
   Makes a new Particle using random uniform distribution for theta, phi (-pi to +pi), energy
   @param[in] int pdgid: particle id (eg 22 for a photon)
   @param[in] double charge: charge of particle eg -1
   @param[in] double thetamin: minimum theta (uniform distribution between mintheta maxtheta)
   @param[in] double thetamax: maximum theta
   @param[in] double ptmin: minimum pt (uniform distribution between minpt maxpt)
   @param[in] double ptmax: maximum pt
   @param[in] const TVector3& vertex: start point of particle
   @return Particle& the newly created particle
   */
  // move this somewhere else
  Particle& addGunParticle(int pdgid, double charge, double thetamin, double thetamax, double ptmin, double ptmax,
                           const TVector3& vertex);  // TODO move elsewhere

private:
  void simulatePhoton(const Particle& ptc);    ///< Simulates cluster from a Photon
  void simulateHadron(const Particle& ptc);    ///< Simulates clusters and track from a Hadron
  void simulateNeutrino(const Particle& ptc);  ///< Simulates a neutrino
  void simulateElectron(const Particle& ptc);  ///< Simulates an electron (no smearing)
  void simulateMuon(const Particle& ptc);      ///< Simulates a muon(no smearing)

  /**
   Determines if a smeared Cluster is detectable
   @param[in] smearedCluster for which we need to determine if it is accepted
   @param[in] acceptLayer detector layer used for acceptance NB not always the same layer as the cluster
   @param[in] accept if true then the cluster will be accepted
   @return boolean true/false
   */
  bool acceptSmearedCluster(const Cluster& smearedCluster, papas::Layer acceptLayer = papas::Layer::kNone,
                            bool accept = false) const;

  /**
   Makes a new Ecal Cluster, stored it in the Ecals collection and updates the history
   @param[in] ptc The parent particle
   @param[in] parentId identifier for parent (used for history)
   @return the stored Cluster (nb this is not the same as the original smearedCluster which has been moved)
   */
  Cluster makeAndStoreEcalCluster(const Particle& ptc, double fraction, double csize, char subtype);
  Cluster makeAndStoreHcalCluster(const Particle& ptc, double fraction, double csize, char subtype);

  /**
   Moves the smearedCluster into the smeared Ecals collection and updates the history
   @param[in] smearedCluster cluster to be stored
   @param[in] parentId identifier for parent (used for history)
   @return the stored Cluster (nb this is not the same as the original smearedCluster which has been moved)
   */
  const Cluster& storeSmearedEcalCluster(Cluster&& smearedCluster, Identifier parentId);

  /**
   Moves the smearedCluster into the smeared Hcals collection and updates the history
   @param[in] smearedCluster cluster to be stored
   @param[in] parentId identifier for parent (used for history)
   @return the stored Cluster (nb this is not the same as the original smearedCluster which has been moved)
   */
  const Cluster& storeSmearedHcalCluster(Cluster&& smearedCluster, Identifier parentId);

  /**
   Makes a track based on particle properties and stored into tracks collection, updating history
   @param[in] ptc particle from which to construct track
   @return the new stored Track
   */
  const Track& makeAndStoreTrack(const Particle& ptc);

  /**
   Smears a track by randomisation of the energy of a track
   @param[in] track the unsmeared track
   @param[in] resolution the standard deviation of the randomisation
   @return the smeared track
   */
  Track smearTrack(const Track& track, double resolution) const;
  /**
   Determines if a smearedtrack is detected
   @param[in] smearedtrack the smeared track
   @param[in] accept. If set to true the track is always accepted
   @return true if track is detected, false otherwise
   */
  bool acceptSmearedTrack(const Track& smearedtrack, bool accept = false) const;

  /**
   Determines if an electron smearedtrack is detected
   @param[in] smearedtrack the smeared track
   @param[in] accept. If set to true the  track is always accepted
   @return true if track is detected, false otherwise
   */
  bool acceptElectronSmearedTrack(const Track& smearedTrack, bool accept = false) const;

  /**
   Determines if a muon smearedtrack is detected
   @param[in] smearedtrack the smeared track
   @param[in] accept. If set to true the track is always accepted
   @return true if track is detected, false otherwise
   */
  bool acceptMuonSmearedTrack(const Track& smearedTrack, bool accept = false) const;

  /**
   Stores a smearedtrack and updates the history
   @param[in] smearedtrack the smeared track which will be moved intot he smeared track store
   @param[in] parentid used to update history
   */
  void storeSmearedTrack(Track&& smearedtrack, Identifier parentid);

  /** Return shared ptr to the appropriate propagator (straightline or helix)
   @param[in] charge charge of particle
   */
  std::shared_ptr<const Propagator> propagator(double charge) const;

  /**
   Returns the detector element associated with a particular layer
   @param[in] layer an enumeration describing which layer eg Layer::kEcal
   @return shared pointer to the detector element
  */
  std::shared_ptr<const DetectorElement> elem(papas::Layer layer) const;

  const Event& m_event;             ///< Event (for consistent Algorithm arguments)?
  const Detector& m_detector;       ///< the Detector
  Clusters& m_ecalClusters;         ///< ecal clusters (prior to smearing)
  Clusters& m_hcalClusters;         ///< hcal clusters (prior to smearing)
  Clusters& m_smearedEcalClusters;  ///< smeared ecal clusters
  Clusters& m_smearedHcalClusters;  ///< smeared hcal clusters
  Tracks& m_tracks;                 ///< tracks
  Tracks& m_smearedTracks;          ///< smeared tracks
  Nodes& m_history;                 ///< Records relationships of everything that is simulated

  std::shared_ptr<StraightLinePropagator> m_propStraight;  ///<used to determine the path of uncharged particles
  std::shared_ptr<HelixPropagator> m_propHelix;            ///<used to determine the path of charged particles
};

}  // end namespace papas

#endif /* Simulator_h */
