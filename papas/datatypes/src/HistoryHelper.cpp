#include "papas/datatypes/HistoryHelper.h"

#include "papas/datatypes/Event.h"

namespace papas {

HistoryHelper::HistoryHelper(const Event& event) : m_event(event) {}

Ids HistoryHelper::linkedIds(Identifier id, DAG::enumVisitType direction) const {
  const auto& history = m_event.history();
  const auto& startnode = history.at(id);
  DAG::BFSRecurseVisitor<PFNode> bfs;
  const auto& nodes = bfs.traverseNodes(startnode, direction);
  Ids ids;
  for (const auto& node : nodes) {
    ids.insert(node->value());
  }
  return ids;
}

Ids HistoryHelper::linkedIds(Identifier id, const std::string& typeAndSubtype, DAG::enumVisitType direction) const {
  auto ids = linkedIds(id, direction);
  auto fids = filteredIds(ids, typeAndSubtype);
  return fids;
}

Ids HistoryHelper::filteredIds(Ids ids, const IdCoder::ItemType type, const IdCoder::SubType subtype) const {
  Ids matchedIds;
  for (auto id : ids) {
    if (IdCoder::type(id) == type && IdCoder::subtype(id) == subtype) {
      matchedIds.insert(id);
    }
  }
  return matchedIds;
}

Ids HistoryHelper::filteredIds(Ids ids, const std::string& typeAndSubtype) const {
  Ids matchedIds;
  for (auto id : ids) {
    if (IdCoder::typeAndSubtype(id) == typeAndSubtype) {
      matchedIds.insert(id);
    }
  }
  return matchedIds;
}

}  // end namespace papas
