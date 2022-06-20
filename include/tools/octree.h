#ifndef OCTREE_H
#define OCTREE_H

struct octree_item {
    struct octree_item* sub_items[8];
};

#endif