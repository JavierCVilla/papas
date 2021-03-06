#ifndef ClicFIELD_H
#define ClicFIELD_H

#include "papas/detectors/Field.h"

namespace papas {

// Forward declaration
class VolumeCylinder;
/// Clic specific implementation of Detector Field element
///
class ClicField : public Field {
public:
  /** Constructor
   * @param[in] magnitude field strength
   * @param[in] radius field cylinder radius
   * @param[in] z field cylinder z
   * @param[in] x0 field  material x0
   * @param[in] lambdaI field material lambdaI
   */
  ClicField(double magnitude = 2., double radius = 3.5, double z = 4.8);

private:
};

}  // end namespace papas
#endif
