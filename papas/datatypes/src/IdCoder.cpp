#include "papas/datatypes/IdCoder.h"

#include <assert.h>
#include <bitset>
#include <cmath>
#include <inttypes.h>
#include <iostream>

#include "papas/utility/StringFormatter.h"

//
// Encode information into an identifier
//
// accessible intro to use of bitwise operations can be found here
// http://stackoverflow.com/questions/6556961/use-of-the-bitwise-operators-to-pack-multiple-values-in-one-int
//

namespace papas {
// max index value is 2** m_bitshift

Identifier IdCoder::makeId(uint32_t index, ItemType type, char subt, float val) {

  if (type == kNone) {
    throw "Id must have a valid type";
  }

  if (index >= pow(2, m_bitshift) - 1) throw "IdCoder: index is too big: too many identifiers";

  // Shift all the parts and join together
  // NB uint64_t is needed to make sure the shift is carried out over 64 bits, otherwise
  // if the m_bitshift is 32 or more the shift is undefined and can return 0

  Identifier typeShift = (uint64_t)type << m_bitshift1;
  Identifier valueShift = (uint64_t)floatToBits(val) << m_bitshift;
  Identifier subtypeShift = (uint64_t) static_cast<int>(tolower(subt)) << m_bitshift2;
  Identifier uid = (uint64_t)subtypeShift | (uint64_t)valueShift | (uint64_t)typeShift | index;

  if (!checkValid(uid, type, subt, val, index)) throw "Error occured constructing identifier";
  return uid;
}

IdCoder::ItemType IdCoder::type(Identifier id) {
  return static_cast<ItemType>((id >> m_bitshift1) & (uint64_t)(pow(2, 3) - 1));
}

char IdCoder::subtype(Identifier id) {
  return static_cast<char>((id >> m_bitshift2) & (uint64_t)(pow(2, m_bitshift1 - m_bitshift2) - 1));
}

float IdCoder::value(Identifier id) {
  // shift to extract the required bits
  int bitvalue = id >> m_bitshift & (uint64_t)(pow(2, m_bitshift2 - m_bitshift) - 1);
  // convert bits back to float
  return bitsToFloat(bitvalue);
}

uint32_t IdCoder::index(Identifier id) { return id & (uint64_t)(pow(2, m_bitshift) - 1); }

uint32_t IdCoder::uniqueId(Identifier id) {
  // For some purposes we want a smaller uniqueid without the value information
  // here we consruct a 32 bit uniqueid out of the index and the type and subtype
  uint32_t bitshift = m_bitshift + m_bitshift1 - m_bitshift2;
  Identifier typeShift = (uint32_t)type(id) << bitshift;
  Identifier subtypeShift = (uint32_t) static_cast<uint32_t>(tolower(subtype(id))) << m_bitshift;
  // binary printout std::cout <<"Index" << std::bitset<32>(IdCoder::index(id)) <<std::endl;
  uint32_t uniqueid = (uint32_t)subtypeShift | (uint32_t)typeShift | (uint32_t)index(id);
  if (!checkUIDValid(id, uniqueid)) throw "unique id part of identifier not valid";
  return uniqueid;
}

char IdCoder::typeLetter(Identifier id) {
  // converts from the identifier type enumeration such as kEcalCluster into a single letter decriptor eg 'e'
  std::string typelist = ".ehtpb....";

  auto index = (uint32_t)type(id);
  if (index < 6)
    return typelist[(uint32_t)type(id)];
  else
    throw "Error in identifier typeLetter";
}

IdCoder::ItemType IdCoder::type(char s) {
  // converts from the a single letter decriptor eg 'e' into the type enumeration such as kEcalCluster
  std::string typelist = ".ehtpb";
  auto found = typelist.find(s);
  if (found == typelist.npos) throw "type not found";
  return (ItemType)found;
}

std::string IdCoder::typeAndSubtype(Identifier id) {
  // produce the two letter type and subtype string such as 'em'
  std::string typeSubType = std::string(1, typeLetter(id)) + std::string(1, subtype(id));
  return typeSubType;
}

std::string IdCoder::pretty(Identifier id) {
  // pretty version of the identifier
  return IdCoder::typeAndSubtype(id) + std::to_string(IdCoder::index(id));
}

papas::Layer IdCoder::layer(Identifier id) {
  if (isEcal(id))
    return papas::Layer::kEcal;
  else if (isHcal(id))
    return papas::Layer::kHcal;
  else if (isTrack(id))
    return papas::Layer::kTracker;
  else
    return papas::Layer::kNone;
}

IdCoder::ItemType IdCoder::type(papas::Layer layer) {
  if (layer == papas::Layer::kEcal)
    return ItemType::kEcalCluster;
  else if (layer == papas::Layer::kHcal)
    return ItemType::kHcalCluster;
  else if (layer == papas::Layer::kTracker)
    return ItemType::kTrack;
  else
    return ItemType::kNone;
}

bool IdCoder::checkValid(Identifier uid, ItemType itype, char subt, float val, uint32_t indx) {
  // verify that it all works, the id should match the items from which it was constructed
  if (index(uid) != indx) return false;
  if (val != 0) {
    if ((fabs(value(uid) - val) >= fabs(val) * 10e-6) | (type(uid) != itype) | (subtype(uid) != subt)) return false;
  }
  return true;
}

bool IdCoder::checkUIDValid(Identifier id, uint32_t uniqueid) {
  uint32_t bitshift = m_bitshift + m_bitshift1 - m_bitshift2;
  // verify that it all works, the uniqueid should match the items from which it was constructed
  ItemType it = static_cast<ItemType>((uniqueid >> bitshift) & (uint32_t)(pow(2, 3) - 1));
  char st = static_cast<char>((uniqueid >> m_bitshift) & (uint64_t)(pow(2, bitshift - m_bitshift) - 1));
  uint32_t idx = ((uniqueid) & (uint32_t)(pow(2, m_bitshift) - 1));
  if (it != type(id) || st != subtype(id) || idx != index(id)) return false;
  return true;
}

uint64_t IdCoder::floatToBits(float value) {
  // CHAR_BIT not known on lxplus assert(CHAR_BIT * sizeof(float) == 32);  // TODO think of somewhere better to put this
  union {
    float value;  // assumes sizeof(float) == sizeof(int)
    int bits;
  } data;
  data.value = value;
  return (uint64_t)(data.bits);
};

float IdCoder::bitsToFloat(uint64_t bits) {
  union {
    float value;  // assumes sizeof(float) == sizeof(int)
    int bits;
  } data;
  data.bits = bits;
  return (data.value);
};

}  // end namespace papas
