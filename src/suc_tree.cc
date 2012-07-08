#include "suc_tree.h"

suc_tree::suc_tree(const bitVector& bv): bv_(bv) {
  size_ = bv.getSize();
  num_blocks_ = (size_ - 1) / block_size + 1;
  tree_depth_ = ceil(log(num_blocks_) / log(tree_arity));
  num_nodes_ = 0;
  for (int d = 0, k = 1; d < tree_depth_; ++d, k *= tree_arity) {
    num_nodes_ += k;
  }
  start_node_offset_ = 1;
  for (int d = 0, k = 1; d < tree_depth_ - 1; ++d, k *= tree_arity) {
    start_node_offset_ += k;
  }
  dbg("size, num_blocks, tree_depth, num_nodes : %u, %d, %d, %d\n",
      size_, num_blocks_, tree_depth_, num_nodes_);
  // set excess vector and leaves of RMM tree
  vector<int> excess(size_);
  leaves_.resize(num_blocks_);
  int prev = 0;
  for (int b = 0; b < num_blocks_; ++b) {
    dbg("\nblock : %d\n", b);
    int local = prev;
    int local_min = prev + 1;
    int local_max = prev - 1;
    for (int i = b * block_size; (i < (b + 1) * block_size) && (i < size_); ++i) {
      local += (bv[i] == 0) ? -1 : 1;
      excess[i] = local;
      local_min = min(local_min, local);
      local_max = max(local_max, local);
      dbg("%d ", excess[i]);
    }
    leaves_[b] = leaf(prev, local_min, local_max);
    prev = local;
  }

  // set internal nodes of RMM tree
  nodes_.resize(num_nodes_);
  for (int sb = num_nodes_ / tree_arity; sb < num_nodes_; ++sb) {
    dbg("sb : %d\n", sb);
    int start = min(tree_arity * sb + 1 - num_nodes_, num_blocks_);
    int end = min(start + tree_arity, num_blocks_);
    dbg("start, end : %d, %d\n", start, end);
    if (start == num_blocks_) {
      nodes_[sb] = node(0, 0);
    } else {
      int min = min_element(leaves_.begin() + start, leaves_.begin() + end, leaf::compare_min)->get_min();
      int max = max_element(leaves_.begin() + start, leaves_.begin() + end, leaf::compare_max)->get_max();
      dbg("leaf min, max : %d, %d\n", min, max);
      nodes_[sb] = node(min, max);
    }
  }
  for (int depth = tree_depth_ - 2; depth >= 0; --depth) {
    int m_begin = 0;
    for (int d = 0; d < depth; m_begin = m_begin * tree_arity + 1, ++d);
    dbg("depth, m_begin : %d, %d\n", depth, m_begin);
    for (int m = m_begin; m <= m_begin * tree_arity; ++m) {
      dbg("m : %d\n", m);
      int start = tree_arity * m + 1;
      int end = start + tree_arity;
      dbg("start, end : %d, %d\n", start, end);
      int min = min_element(nodes_.begin() + start, nodes_.begin() + end, node::compare_min)->get_min();
      int max = max_element(nodes_.begin() + start, nodes_.begin() + end, node::compare_max)->get_max();
      dbg("min, max : %d, %d\n", min, max);
      nodes_[m] = node(min, max);
    }
  }
}

void suc_tree::print_bits() {
  for (int b = 0; b < num_blocks_; ++b) {
    for (int u = 0; u < block_size / lookup_unit_size; ++u) {
      for (int i = 0; i < lookup_unit_size; ++i) {
        uint j = block_size * b + lookup_unit_size * u + i;
        if (j < size_) printf("%d", bv_[j]);
      }
      printf(" ");
    }
    printf("\n");
  }
}

void suc_tree::print_nodes() {
  printf("leaves :\n");
  for (vector<leaf>::iterator it = leaves_.begin(); it != leaves_.end(); ++it) {
    printf("(%d, %d, %d)\n", it->get_offset(), it->get_min(), it->get_max());
  }
  printf("nodes :\n");
  for (vector<node>::iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
    printf("(%d, %d)\n", it->get_min(), it->get_max());
  }
}

uint suc_tree::inspect(uint i) {
  return bv_[i];
}

bool suc_tree::isleaf(uint i) {
  return inspect(i + 1) == 0;
}

int suc_tree::block_sum(uint i, uint j) {
  int x = 0;
  for(uint k = i; k <= j; ++k) {
    x += (bv_[k] == 1) ? 1 : -1;
  }
  return x;
}

string bits_to_string(uint i) {
  std::string str;
  for (int j = 0; j < lookup_unit_size; ++j) {
    str += (i & 1) ? "1" : "0";
    i = i >> 1;
  }
  return str;
}

int suc_tree::fwd_lookup(uint bits, int d) {
  if (d < -lookup_unit_size || d > lookup_unit_size) {
    return lookup_table[0][bits] & 0xf0;
  } else {
/*    int sum = 0;
    int is_found = 0;
    int found_index = 0;
    int popcnt = 0;
    for (int j = 0; j < lookup_unit_size; ++j) {
      int b = bits >> j & 1;
      sum += b ? 1 : -1;
      if (!is_found && sum == d) {
        is_found = 1;
        found_index = j;
      }
      popcnt += b;
    }
    return (popcnt << 4) + (found_index << 1) + is_found;
  } */
    return lookup_table[d + lookup_unit_size][bits];
  }
}

uint suc_tree::fwd_search_inblock(uint i, int d) {
  uint bits_index = i / bitsize;
  uint bits_rem = i % bitsize;
  uint bits = bv_.getBlock(bits_index);
  int unit = bits_rem / lookup_unit_size + 1;
  dbg("b_ind, b_rem, bits, unit : %d, %d, %s, %d\n", bits_index, bits_rem, bits_to_string(bits).c_str(), unit);
  // in unit remain
  int sum = 0;
  for (uint j = bits_rem; j < unit * lookup_unit_size; ++j) {
    dbg("j : %d\n", j);
    sum += (bits >> j & 1 == 1) ? 1 : -1;
    if (sum == d) {
      uint ret = bits_index * bitsize + j;
      return (ret < size_) ? ret : UINT_MAX;
    }
  }
  d -= sum;

  // unit main
  for (uint b = bits_index; b <= bits_index || b * bitsize < block_size; ++b) {
    bits = bv_.getBlock(b);
    for (int u = (b == bits_index) ? unit : 0; u < bitsize / lookup_unit_size; ++u) {
      int res = fwd_lookup(bits >> (u * lookup_unit_size) & 0xff, d);
      if (res & 1) {
        uint ret =  b * bitsize + u * lookup_unit_size + (res >> 1 & 0x7);
        return (ret < size_) ? ret : UINT_MAX;
      } else {
        d -= ((res >> 4) * 2 - lookup_unit_size);
      }
    }
  }

  // not found
  return UINT_MAX;
}

int suc_tree::tree_search(int node, int d) {
  int depth = tree_depth_ - 1;
  while (true) { // upward
    for (; node % tree_arity != 0 && !nodes_[node].includes(d); ++node);
    if (node % tree_arity == 0 && !nodes_[node].includes(d)) {
      node /= tree_arity;
      --depth;
      if (depth < 0) return -1;
      dbg("up node, depth : %d, %d\n", node, depth);
    } else {
      break;
    }
  }
  while (true) { // downward
    for (; node % tree_arity != 0 && !nodes_[node].includes(d); ++node);
    if (depth == tree_depth_ - 1) {
      return node;
    } else {
      ++depth;
      node = node * tree_arity + 1;
      dbg("down node, depth : %d, %d\n", node, depth);
    }
  }
  return -1;
}

uint suc_tree::fwd_search(uint i, int d) {
  int start_block = i / block_size;
  uint edge_index = (start_block + 1) * block_size - 1;
  // in block search
  dbg("phase1\n");
  uint ret = fwd_search_inblock(i, d);
  if (ret != UINT_MAX) return ret;

  // in leaves search
  dbg("phase2\n");
  int edge_block = (start_block / tree_arity + 1) * tree_arity - 1;
  int new_d = d + leaves_[start_block + 1].get_offset() - block_sum(i, edge_index);
  dbg("edge_block, new_d : %d, %d\n", edge_block, new_d);
  for (int b = start_block + 1; b <= edge_block; ++b) {
    if (leaves_[b].includes(new_d)) return fwd_search_inblock(b * block_size, new_d - leaves_[b].get_offset());
  }
  // tree search
  dbg("phase3\n");
  int start_node = start_node_offset_ + edge_block / tree_arity;
  int tree_search_return = tree_search(start_node, new_d);
  if (tree_search_return == -1) return UINT_MAX;
  int block = tree_search_return * tree_arity + 1 - num_nodes_;
  dbg("start_node, block : %d, %d\n", start_node, block);
  for (int b = block; b < block + tree_arity; ++b) {
    if (leaves_[b].includes(new_d)) return fwd_search_inblock(b * block_size, new_d - leaves_[b].get_offset());
  }

  return UINT_MAX;
}

uint suc_tree::find_close(uint i) {
  return fwd_search(i, 0);
}

bool suc_tree::is_ancestor(uint i, uint j) {
  return i <= j && find_close(j) <= find_close(i);
}

uint suc_tree::fwd_search_naive(uint i, int d) {
  int x = 0;
  for (uint j = i; j < size_; ++j) {
    x += (bv_[j] == 1) ? 1 : -1;
    if (d == x) return j;
  }
  return UINT_MAX;
}
