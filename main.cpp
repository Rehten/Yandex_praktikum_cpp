#include <cassert>
#include <iostream>
#include <memory>

template<typename T>
struct TreeNode;

template<typename T>
using TreeNodePtr = std::unique_ptr<TreeNode<T>>;

template<typename T>
struct TreeNode
{
  TreeNode(T val, TreeNodePtr<T> &&left, TreeNodePtr<T> &&right)
    : value(std::move(val)), left(std::move(left)), right(std::move(right))
  {
  }

  T value;
  TreeNodePtr<T> left;
  TreeNodePtr<T> right;
  TreeNode *parent = nullptr;
};

template<typename T>
bool CheckTreeProperty(const TreeNode<T> *node) noexcept
{
  if (!node->left.get() || !node->right.get())
  {
    return (
      (node->left.get() == nullptr || node->left->value <= node->value)
      &&
      (node->right.get() == nullptr || node->right->value >= node->value)
    );
  }

  bool is_left_and_right_are_balanced(
    (node->left.get() == nullptr || node->left->value <= node->value)
    &&
    (node->right.get() == nullptr || node->right->value >= node->value)
  );

  if (!is_left_and_right_are_balanced)
  {
    return false;
  }

  return CheckTreeProperty(node->left.get()) && CheckTreeProperty(node->right.get());
}

template<typename T>
TreeNode<T> *begin(TreeNode<T> *node) noexcept
{
  if (node->left.get() == nullptr)
  {
    return nullptr;
  }

  while (node->left.get())
  {
    node = node->left.get();
  }

  return node;
}

template<typename T>
TreeNode<T> *next(TreeNode<T> *node) noexcept
{
  if (node->right.get())
  {
    node = node->right.get();

    while (node->left.get())
    {
      node = node->left.get();
    }

    return node;
  }
  else if (node->parent && node->parent->value > node->value)
  {
    return node->parent;
  }
  else
  {
    auto node_copy = node;

    while (node_copy->parent && node_copy->parent->value < node->value)
    {
      node_copy = node_copy->parent;
    }

    return node_copy->parent;
  }
}

// функция создаёт новый узел с заданным значением и потомками
TreeNodePtr<int> N(int val, TreeNodePtr<int> &&left = {}, TreeNodePtr<int> &&right = {})
{
  TreeNode<int> *rslt = new TreeNode(val, std::move(left), std::move(right));

  if (rslt->left)
  {
    rslt->left->parent = rslt;
  }

  if (rslt->right)
  {
    rslt->right->parent = rslt;
  }
  return std::unique_ptr<TreeNode<int>>(rslt);
}

int main()
{
  using namespace std;
  using T = TreeNode<int>;
  auto root1 = N(6, N(4, N(3), N(5)), N(7));
  assert(CheckTreeProperty(root1.get()));

  T *iter = begin(root1.get());
  while (iter)
  {
    cout << iter->value << " "s;
    iter = next(iter);
  }
  cout << endl;

  auto root2 = N(6, N(4, N(3), N(5)), N(7, N(8)));
  assert(!CheckTreeProperty(root2.get()));

  // Функция DeleteTree не нужна. Узлы дерева будут рекурсивно удалены
  // благодаря деструкторам unique_ptr
}
