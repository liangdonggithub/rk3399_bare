#ifndef RBTREE_H
#define RBTREE_H

#include "tools/types.h"

typedef int Type;

// 红黑树的节点
typedef struct RBTreeNode{
    struct RBTreeNode *left;    // 左孩子
    struct RBTreeNode *right;    // 右孩子
    struct RBTreeNode *parent;    // 父结点
    u64                node_attr;
}Node, *RBTree;

// 红黑树的根
typedef struct rb_root{
    Node *node;
}RBRoot;

typedef s8  (*insert_cmp_cb)(Node*,Node*);
typedef s8  (*search_cmp_cb)(Node*,void*);

void rbnode_zero(RBTree node);
// 销毁红黑树
void destroy_rbtree(RBRoot *root);
// 将结点插入到红黑树中。插入成功，返回0；失败返回-1。
int insert_rbtree(RBRoot *root, Node* pnode, insert_cmp_cb cmp_cb);
// 删除结点(key为节点的值)
void delete_rbtree(RBRoot *root, Node* node);
// 前序遍历"红黑树"
void preorder_rbtree(RBRoot *root);
// 中序遍历"红黑树"
void inorder_rbtree(RBRoot *root);
// 后序遍历"红黑树"
void postorder_rbtree(RBRoot *root);

// (递归实现)查找"红黑树"中键值为key的节点。找到的话，返回0；否则，返回-1。
Node* rbtree_search(RBRoot *root, void* param, search_cmp_cb cmp_cb);
// (非递归实现)查找"红黑树"中键值为key的节点。找到的话，返回0；否则，返回-1。
Node* iterative_rbtree_search(RBRoot *root, void* param, search_cmp_cb cmp_cb);

Node* rbtree_smallest_bigger(RBRoot *root, void* param, search_cmp_cb cmp_cb);

Node* rbtree_biggest_smaller(RBRoot *root, void* param, search_cmp_cb cmp_cb);

u8    rbtree_empty(RBRoot* root);

// 返回最小结点的值(将值保存到val中)。找到的话，返回0；否则返回-1。
Node* rbtree_minimum(RBRoot *root);
// 返回最大结点的值(将值保存到val中)。找到的话，返回0；否则返回-1。
Node* rbtree_maximum(RBRoot *root);

// 打印红黑树
void print_rbtree(RBRoot *root);

#define RED                     0
#define BLACK                   1

#define GET_RBT_ATTR_COLOR(x)   (x & 0x1)
#define SET_RBT_ATTR_COLOR(x,c) do { x &= ~0x1UL; x |= (c & 0x1); } while(0)



#endif