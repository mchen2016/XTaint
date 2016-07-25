#include "xt_flag.h"
#include "xt_propagate.h"
#include "xt_util.h"

#include <algorithm>
#include <cassert>
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

Propagate::Propagate(){}

std::unordered_set<Node, NodeHash> Propagate::searchAvalanche(vector<string> &log)
{
    std::unordered_set<Node, NodeHash> propagate_res;
    vector<Rec> v_rec;
    NodePropagate s;
    bool isFound = false;

    v_rec = initRec(log);

    s.id = 0;
    s.isSrc = true;
    s.n.flag = "34";
    s.n.addr = "bffff753";
    s.n.val = "34";
    s.n.i_addr = std::stoul(s.n.addr, nullptr, 16);
    s.n.sz = 8;

    int i = 0;
    for(vector<Rec>::iterator it = v_rec.begin(); it != v_rec.end(); ++it){
        if(!(*it).isMark){
            if(s.n.flag == (*it).regular.src.flag &&
               s.n.addr == (*it).regular.src.addr &&
               s.n.val == (*it).regular.src.val){
                isFound = true;
                break;
            }
        }
        i++;
    } // end for

    if(isFound){
        s.pos = i;
        propagate_res = bfs_old(s, v_rec);
    }
    return propagate_res;
}

vector<Rec> Propagate::initRec(vector<string> &log)
{
    vector<Rec> v_rec;
    vector<string> v_log, v_single_rec;

    Rec rec;
    Node src, dst;

    for(vector<string>::iterator it = log.begin(); it != log.end(); ++it){
        v_single_rec = XT_Util::split( (*it).c_str(), '\t');
        if(XT_Util::isMarkRecord(v_single_rec[0]) ){
            rec.isMark = true;
            rec.regular = initMarkRecord(v_single_rec);
        }else{
            rec.isMark = false;
            rec.regular = initRegularRecord(v_single_rec);
        }
        v_rec.push_back(rec);
    }
    return v_rec;
} 

// UNFINISHED !!!
unordered_set<Node, NodeHash> Propagate::bfs(NodePropagate &s, vector<Rec> &r)
{
    queue<NodePropagate> q_propagate;
    unordered_set<Node, NodeHash> res;

    NodePropagate currNode, nextNode;
    struct Rec currRec;
    int numHit;
    bool isValidPropagate, isSameInsn;

    q_propagate.push(s);
    while(!q_propagate.empty() ){
        currNode = q_propagate.front();
        q_propagate.pop();

        // if a source node
        if(currNode.isSrc){
            unsigned int i = currNode.pos;
            // can't be a mark
            assert( r[i].isMark == false);
            nextNode = propagate_dst(currNode, r);
            q_propagate.push(nextNode);

            // if it is store to buffer operation, save to propagate result
            Node node = nextNode.n;
            if(XT_Util::equal_mark(node.flag, flag::TCG_QEMU_ST) )
                insert_propagate_result(node, res);
        } else { // if a dst node
            // find valid propagation from dst -> src for afterwards records
            numHit = 0;
            isSameInsn = true;  // assume belongs to same insn at first
            vector<Rec>::size_type i = currNode.pos + 1;
            for(; i != r.size(); i++) {
                isValidPropagate = false;
                currRec = r[i];

                // if cross insn boundary
                if(isSameInsn)
                    if(currRec.isMark && 
                        XT_Util::equal_mark(currRec.regular.src.flag, flag::XT_INSN_ADDR) )
                        isSameInsn = false;

                if(!currRec.isMark){
                    isValidPropagate = is_valid_propagate(currNode, currRec, r);

                    if(isValidPropagate){
                        nextNode = propagte_src(r, i);
                        // is it a load opreration? If so, then it is a memory buffer

                    } // end isValidPropagate
                }
            } // end of for loop
        }
    } // end of while loop
    return res;
}

unordered_set<Node, NodeHash> Propagate::bfs_old(NodePropagate &s, vector<Rec> &v_rec)
{
    unordered_set<Node, NodeHash> res_buffer;
    vector<NodePropagate> v_propagate_buffer;
    queue<NodePropagate> q_propagate;

    NodePropagate currNode, nextNode;
    struct Rec currRec;
    int numHit;
    bool isValidPropagate, isSameInsn;

    v_propagate_buffer.push_back(s);
    while(!v_propagate_buffer.empty() ){
    L_Q_PROPAGATE:
        // non buffer propagation
        while(!q_propagate.empty() ){
            numHit = 0;
            isSameInsn = true;

            currNode = q_propagate.front();

            // if a source node
            if(currNode.isSrc){
                unsigned int i = currNode.pos;
                // can't be a mark
                assert( v_rec[i].isMark == false);
                nextNode = propagate_dst(currNode, v_rec);
                q_propagate.push(nextNode);
                numHit++;

                // if it is store to buffer operation, save to propagate buffer result
                Node node = nextNode.n;
                if(XT_Util::equal_mark(node.flag, flag::TCG_QEMU_ST) )
                    insert_propagate_result(node, res_buffer);
            }
            // if a dst node
            // find valid propagation from dst -> src for afterwards records
            else{
                numHit = 0;
                isSameInsn = true;  // assume belongs to same insn at first
                vector<Rec>::size_type i = currNode.pos + 1;
                for(; i != v_rec.size(); i++) {
                    isValidPropagate = false;
                    currRec = v_rec[i];

                    // if cross insn boundary
                    if(isSameInsn)
                        if(currRec.isMark && 
                            XT_Util::equal_mark(currRec.regular.src.flag, flag::XT_INSN_ADDR) )
                            isSameInsn = false;

                    if(!currRec.isMark){
                        isValidPropagate = is_valid_propagate(currNode, currRec, v_rec);

                        if(isValidPropagate){
                            nextNode = propagte_src(v_rec, i);
                            // is it a load opreration? If so, then it is a memory buffer
                            if(XT_Util::equal_mark(nextNode.n.flag, flag::TCG_QEMU_LD) ){
                                insert_buffer_node(nextNode, v_propagate_buffer, numHit);

                                // also save to propagate result!!!
                                Node node = nextNode.n;
                                insert_propagate_result(node, res_buffer);
                            } else{ // if not a buffer node
                                if(is_save_to_q_propagate(isSameInsn, numHit) ){
                                    q_propagate.push(nextNode);
                                    numHit++;
                                }
                            }
                        } // end isValidPropagate
                    } // end isMark

                    // if not belong to same instruction and
                    // already have hit, and
                    // not a memory buffer
                    // can break the loop
                    if(!isSameInsn && 
                        numHit >= 1 && 
                        !XT_Util::equal_mark(currNode.n.flag, flag::TCG_QEMU_ST) )
                            break;
                } // end of for loop
            } // end dst node case
            q_propagate.pop();  
        } // end while q_propagate

        if(!v_propagate_buffer.empty() ){
            currNode = v_propagate_buffer[0];
            v_propagate_buffer.erase(v_propagate_buffer.begin() );

            // memory buffer only contains buffer nodes are as src
            if(currNode.isSrc){
                nextNode = propagate_dst(currNode, v_rec);
                q_propagate.push(nextNode);
                numHit++;
            }
        }

        if(!q_propagate.empty() )
            goto L_Q_PROPAGATE;
    } // end while v_propagate_buffer

    return res_buffer;
}

inline NodePropagate Propagate::propagate_dst(NodePropagate &s, vector<Rec> &r)
{
    NodePropagate d;
    unsigned int i = s.pos;

    d.isSrc = false;
    d.pos = s.pos;
    d.id = s.id + 1;

    d.n.flag = r[i].regular.dst.flag;
    d.n.addr = r[i].regular.dst.addr;
    d.n.val = r[i].regular.dst.val;
    d.n.i_addr = r[i].regular.dst.i_addr;
    d.n.sz = r[i].regular.dst.sz;

    return d;
}

inline NodePropagate Propagate::propagte_src(std::vector<Rec> &v_rec, int i)
{
    NodePropagate s;

    s.isSrc = true;
    s.pos = i;
    s.id = i * 2;

    s.n.flag = v_rec[i].regular.src.flag;
    s.n.addr = v_rec[i].regular.src.addr;
    s.n.val = v_rec[i].regular.src.val;
    s.n.i_addr = v_rec[i].regular.src.i_addr;
    s.n.sz = v_rec[i].regular.src.sz;

    return s;
}

inline void Propagate::insert_propagate_result(Node &n, std::unordered_set<Node, NodeHash> &res)
{
    unordered_set<Node, NodeHash>::const_iterator got = res.find(n);
    // if not in the propagate result
    if(got == res.end() )
        res.insert(n);
}

// dst -> src propagation rules:
//      1. records belong to same insn, can have multiple hits
//      2. records beyond insn, can only have one hit
// if the dst node is a store operation, then if
//      dst.addr == current record src.addr
//      consider valid
// else otherwise
//      case 1 - dst.addr == current record src.addr
inline bool Propagate::is_valid_propagate(NodePropagate &currNode, 
                                                                    Rec &currRec, 
                                                                    vector<Rec> &v_rec)
{
    bool isValidPropagate, isStore; 

    isValidPropagate = false;
    if(XT_Util::equal_mark(currNode.n.flag, flag::TCG_QEMU_ST) )
        isStore = true;
    else
        isStore = false;

    // is the dst node a store operation, indicating node is a memory buffer
    if(isStore){
        if(currNode.n.addr == currRec.regular.src.addr)
            isValidPropagate = true;
    }else{
        // case 1
        // dst node.addr == current node src.addr
        if(currNode.n.addr == currRec.regular.src.addr){
            // if vals are also same
            if(currNode.n.val == currRec.regular.src.val)
                isValidPropagate = true;
            else if(currNode.n.val.find(currRec.regular.src.val) != string::npos || 
                        currRec.regular.src.val.find(currNode.n.val) != string::npos)
                isValidPropagate = true;
            // specail case: tcg add
            else if(XT_Util::equal_mark(currRec.regular.src.flag, flag::TCG_ADD) )
                isValidPropagate = true;
            // special case: if current node next node is a tcg xor
            else if(XT_Util::equal_mark(v_rec[currNode.pos + 1].regular.src.flag, flag::TCG_XOR) )
                isValidPropagate = true;
        }
        // case 2
        // load pointer: current node val is same with current record's addr
        else if(currNode.n.val == currRec.regular.src.addr && 
                    XT_Util::equal_mark(currNode.n.flag, flag::TCG_QEMU_LD) )
            isValidPropagate = true;
    }

    return isValidPropagate;
}

bool Propagate::compare_buffer_node(const NodePropagate &a, const NodePropagate &b)
{
    return a.id < b.id;
}

void Propagate::insert_buffer_node(NodePropagate &node, 
                                                          vector<NodePropagate> &v_propagate_buf, 
                                                          int &numHit)
{
    bool hasNode = false;
    for(vector<NodePropagate>::iterator it = v_propagate_buf.begin(); 
          it != v_propagate_buf.end(); ++it){
        if((*it).id == node.id)
            hasNode = true;
    }

    if(!hasNode){
        v_propagate_buf.push_back(node);
        sort(v_propagate_buf.begin(), v_propagate_buf.end(), compare_buffer_node);
        numHit++;
    }
}

// determines if it needs to save to the q_propagate given 
//      1). flag of is in same instruction 
//      2). number of valid propagations hits
inline bool Propagate::is_save_to_q_propagate(bool isSameInsn, int &numHit)
{
    bool isSave = false;

    if(isSameInsn)
        isSave = true;
    else{
        if(numHit < 1)
            isSave = true;
    }
    return isSave;
}

inline RegularRec Propagate::initMarkRecord(vector<string> &singleRec)
{
    RegularRec mark;

    mark.src.flag = singleRec[0];
    mark.src.addr = singleRec[1];
    mark. src.val = singleRec[2];
    mark.src.i_addr = 0;
    mark.src.sz = 0;

    return mark;
}

inline RegularRec Propagate::initRegularRecord(vector<string> &singleRec)
{
    RegularRec reg;

    reg.src.flag = singleRec[0];
    reg.src.addr = singleRec[1];
    reg.src.val = singleRec[2];
    reg.src.i_addr = 0;
    reg.src.sz = 0;

    reg.dst.flag = singleRec[3];
    reg.dst.addr = singleRec[4];
    reg.dst.val = singleRec[5];
    reg.dst.i_addr = 0;
    reg.dst.sz = 0;

    if(XT_Util::equal_mark(singleRec[0], flag::TCG_QEMU_LD) ){
        reg.src.i_addr = std::stoul(singleRec[1], nullptr, 16);
        reg.src.sz = std::stoul(singleRec[6], nullptr, 10);
    } else if(XT_Util::equal_mark(singleRec[0], flag::TCG_QEMU_ST) ) {
        reg.dst.i_addr = std::stoul(singleRec[4], nullptr, 16);
        reg.dst.sz = std::stoul(singleRec[6], nullptr, 10);
    }

    return reg;
}