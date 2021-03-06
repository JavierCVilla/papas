#ifndef helixpropagator_h
#define helixpropagator_h

#include "papas/simulation/Propagator.h"

namespace papas {
class Particle;
class Field;
class SurfaceCylinder;
class HelixPropagator : public Propagator {
  /** Class to determine where the (helix) path of a charged particle crosses the detector cylinders
  */
public:
  /** Constructor
  */
  HelixPropagator(std::shared_ptr<const Field> field);
  /**
   Propagate charged particle along Helix to the selected cylinder and store the point
   where the particle crosses the cylinder
   @param[in] ptc particle that is to be propagated
   @param[in] cyl cylinder to which the particle is to be propagated.
   @param[in] field magnitude of magnetic field
   */
  virtual void propagateOne(const Particle& ptc, const SurfaceCylinder& cyl) const override;

  /** Sets the particle path to a helix
   @param[in] ptc particle that is to be propagated
   */
  virtual void setPath(Particle& ptc) const override;
};
}  // end namespace papas

#endif
