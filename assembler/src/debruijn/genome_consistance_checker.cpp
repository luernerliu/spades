//
// Created by lab42 on 9/1/15.
//

#include "genome_consistance_checker.hpp"
#include "debruijn_graph.hpp"
#include <algorithm>

namespace debruijn_graph {
using omnigraph::MappingRange;
using namespace std;

//gap or overlap size. WITHOUT SIGN!
size_t gap(const Range &a, const Range &b) {
    return max(a.end_pos, b.start_pos) - min (a.end_pos, b.start_pos);
}
bool GenomeConsistenceChecker::consequent(const Range &mr1, const Range &mr2) {
    if (mr1.end_pos > mr2.start_pos + absolute_max_gap_)
        return false;
    if (mr1.end_pos + absolute_max_gap_ < mr2.start_pos)
        return false;
    return true;

}
bool GenomeConsistenceChecker::consequent(const MappingRange &mr1, const MappingRange &mr2) {
    //do not want to think about handling gaps near 0 position.
    if (mr2.initial_range.start_pos <= absolute_max_gap_ && mr1.initial_range.end_pos + absolute_max_gap_ >= genome_.size() )
        return true;
    if (!consequent(mr1.initial_range, mr2.initial_range) || !consequent(mr1.mapped_range, mr2.mapped_range))
        return false;
    size_t initial_gap = gap(mr1.initial_range, mr2.initial_range);
    size_t mapped_gap = gap(mr1.mapped_range, mr2.mapped_range);
    size_t max_gap = max(initial_gap, mapped_gap);
    if (1.* max_gap > relative_max_gap_* max (min(mr1.initial_range.size(), mr1.mapped_range.size()), min(mr2.initial_range.size(), mr2.mapped_range.size())))
        return false;
    return true;
}
/*
set<MappingRange> GenomeConsistenceChecker::FillPositionGaps(const set<MappingRange> &info, size_t gap) {
    set<MappingRange> result;
    auto cur = info.begin();
    while(cur != info.end()) {
        MappingRange new_range = *cur;
        ++cur;
        while(cur != info.end() && consequent(new_range, *cur, gap)) {
            new_range = new_range.Merge(*cur);
            ++cur;
        }
        result.insert(new_range);
    }
    return result;
}

void GenomeConsistenceChecker::GenomeConsistenceChecker::Merge(set<MappingRange> &ranges, set<MappingRange> &to_merge, int shift) {
    for(set<MappingRange>::iterator it = to_merge.begin(); it != to_merge.end(); ++it) {
        ranges.insert(genome_mapping_.EraseAndExtract(ranges, it->Shift(shift)));
    }
}

bool GenomeConsistenceChecker::IsConsistentWithGenomeStrand(const vector<EdgeId> &path, const string &strand) const {
    size_t len = graph_.length(path[0]);
    for (size_t i = 1; i < path.size(); i++) {
        Merge(res, .GetEdgePositions(path[i], strand));
        len += graph_.length(path[i]);
    }
    FillPositionGaps(res, len);
    if (res.size() > 0) {
        for (size_t i = 0; i < res.size(); i++) {
            size_t m_len = res[i].initial_range.size();
            if (abs(int(res[i].initial_range.size()) - int(len)) < max(1.0 * max_gap_, 0.07 * len))
                return true;
        }
    }
    return false;
}

bool GenomeConsistenceChecker::IsConsistentWithGenome(vector<EdgeId> path) const {
    if (path.size() == 0)
        return false;
    for (size_t i = 0; i + 1 < path.size(); i++) {
        if (graph_.EdgeStart(path[i + 1]) != graph_.EdgeEnd(path[i]))
            return false;
    }
    return IsConsistentWithGenomeStrand(path, "0") || IsConsistentWithGenomeStrand(path, "1");
}

*/
PathScore GenomeConsistenceChecker::CountMisassemblies(BidirectionalPath &path) const {
    PathScore straight = CountMisassembliesWithStrand(path, "0");
    PathScore reverse = CountMisassembliesWithStrand(path, "1");
    size_t total_length = path.LengthAt(0);
//TODO: constant;
    if (total_length > std::max(straight.mapped_length, reverse.mapped_length) * 2) {
        INFO("mapped less than half of the path, skipping");
        return PathScore();
    } else {
        if (straight.mapped_length > reverse.mapped_length) {
            return straight;
        } else {
            return reverse;
        }
    }
}

void GenomeConsistenceChecker::SpellGenome() const {
    vector<pair<EdgeId, MappingRange> > to_sort;
    for(auto e: storage_) {
        if (excluded_unique_.find(e) == excluded_unique_.end() ) {
            set<MappingRange> mappings = gp_.edge_pos.GetEdgePositions(e, "fxd0");
            if (mappings.size() > 1) {
                INFO("edge " << e << "smth strange");
            } else if (mappings.size() == 0) {
                continue;
            } else {
                to_sort.push_back(make_pair(e, *mappings.begin()));
            }
        }
    }
    sort(to_sort.begin(), to_sort.end(), [](const pair<EdgeId, MappingRange> & a, const pair<EdgeId, MappingRange> & b) -> bool
    {
        return a.second.initial_range.start_pos < b.second.initial_range.start_pos;
    }
    );
    size_t count = 0;
    for(auto p: to_sort) {
        INFO("edge " << gp_.g.int_id(p.first) << " length "<< gp_.g.length(p.first) << " coverage " << gp_.g.coverage(p.first) << " mapped to " << p.second.mapped_range.start_pos << " - " << p.second.mapped_range.end_pos << " init_range " << p.second.initial_range.start_pos << " - " << p.second.initial_range.end_pos );
        genome_spelled_[p.first] = count;
        count++;
    }
}

PathScore GenomeConsistenceChecker::CountMisassembliesWithStrand(BidirectionalPath &path, string strand) const {
    return debruijn_graph::PathScore();
}
void GenomeConsistenceChecker::RefillPos() {
    RefillPos("0");
    RefillPos("1");
}


void GenomeConsistenceChecker::RefillPos(const string &strand) {
    for (auto e: storage_) {
        RefillPos(strand, e);
    }
}

void GenomeConsistenceChecker::RefillPos(const string &strand, const EdgeId &e) {

    set<MappingRange> old_mappings = gp_.edge_pos.GetEdgePositions(e, strand);
    TRACE("old mappings sz " << old_mappings.size() );
    size_t total_mapped = 0;
    for (auto mp:old_mappings) {
        total_mapped += mp.initial_range.size();
    }
    if (total_mapped * 1. > gp_.g.length(e) * 1.5) {
       INFO ("Edge " << gp_.g.int_id(e) << "is not unique, excluding");
       excluded_unique_.insert(e);
       return;
    }
//TODO: support non-unique edges;
    if (total_mapped * 1. < gp_.g.length(e) * 0.5) {
        DEBUG ("Edge " << gp_.g.int_id(e) << "is not mapped on strand "<< strand <<", not used");
        //excluded_unique_.insert(e);
        return;
    }
    TRACE(total_mapped << " " << gp_.g.length(e));
    string new_strand = "fxd" + strand;
    vector<MappingRange> res;
    vector<MappingRange> to_process (old_mappings.begin(), old_mappings.end());
    sort(to_process.begin(), to_process.end(), [](const MappingRange & a, const MappingRange & b) -> bool
    {
        return a.mapped_range.start_pos < b.mapped_range.start_pos;
    } );
    size_t sz = to_process.size();
//max weight path in orgraph of mappings
    TRACE("constructing mapping graph");
    TRACE(sz << " vertices");
    vector<vector<size_t>> consecutive_mappings(sz);
    for(size_t i = 0; i < sz; i++) {
        for (size_t j = i + 1; j < sz; j++) {
            if (consequent(to_process[i], to_process[j])) {
                consecutive_mappings[i].push_back(j);
            } else {
                if (to_process[j].mapped_range.start_pos > to_process[i].mapped_range.end_pos + absolute_max_gap_) {
                    break;
                }
            }
        }
    }
    TRACE("graph constructed");
    vector<size_t> scores(sz);
    vector<int> prev(sz);
    for(size_t i = 0; i < sz; i++) {
        scores[i] = to_process[i].initial_range.size();
        prev[i] = -1;
    }
    for(size_t i = 0; i < sz; i++) {
        for (size_t j = 0; j < consecutive_mappings[i].size(); j++) {
            TRACE(consecutive_mappings[i][j]);
            if (scores[consecutive_mappings[i][j]] < scores[i] + to_process[consecutive_mappings[i][j]].initial_range.size()) {
                scores[consecutive_mappings[i][j]] = scores[i] + to_process[consecutive_mappings[i][j]].initial_range.size();
                prev[consecutive_mappings[i][j]] = i;
            }
        }
    }
    size_t cur_max = 0;
    size_t cur_i = 0;

    vector<size_t> used_mappings;
    for(size_t i = 0; i < sz; i++) {
        if (scores[i] > cur_max) {
            cur_max = scores[i];
            cur_i = i;
        }
    }
    while (cur_i != -1) {
        used_mappings.push_back(cur_i);
        cur_i = prev[cur_i];
    }
    reverse(used_mappings.begin(), used_mappings.end());


    cur_i = 0;
    MappingRange new_mapping;
    new_mapping = to_process[used_mappings[cur_i]];
    size_t used_mapped = new_mapping.initial_range.size();
    TRACE ("Edge " << gp_.g.int_id(e) << " length "<< gp_.g.length(e));
    TRACE ("new_mapping mp_range "<< new_mapping.mapped_range.start_pos << " - " << new_mapping.mapped_range.end_pos
         << " init_range " << new_mapping.initial_range.start_pos << " - " << new_mapping.initial_range.end_pos );
    while (cur_i  < used_mappings.size() - 1) {
        cur_i ++;
        used_mapped += to_process[used_mappings[cur_i]].initial_range.size();
        new_mapping = new_mapping.Merge(to_process[used_mappings[cur_i]]);
        TRACE("new_mapping mp_range "<< new_mapping.mapped_range.start_pos << " - " << new_mapping.mapped_range.end_pos
             << " init_range " << new_mapping.initial_range.start_pos << " - " << new_mapping.initial_range.end_pos );


    }
    if (total_mapped - used_mapped >= 0.1 * gp_.g.length(e)) {
        INFO ("Edge " << gp_.g.int_id(e) << " length "<< gp_.g.length(e)  << "is potentially misassembled! mappings: ");
        for (auto mp:old_mappings) {
            INFO("mp_range "<< mp.mapped_range.start_pos << " - " << mp.mapped_range.end_pos << " init_range " << mp.initial_range.start_pos << " - " << mp.initial_range.end_pos );
        }

    }
    gp_.edge_pos.AddEdgePosition(e, new_strand, new_mapping);


}



}