#ifndef B_TREE_H
#define B_TREE_H
#include <array>
#include <algorithm>
#include <type_traits>
#include <iostream>
#include <cassert>

template <typename T, int N, typename pred = std::less<T>>
class BTree{
private:
    static constexpr int Rank=N+1;
    class BTreeNode{
    public:
        //数据数组
        std::array<T, N> datas;
        //存放子树的指针
        std::array<typename std::add_pointer<BTreeNode>::type, N + 1> datasPtr;

        //指向父节点
        typename std::add_pointer<BTreeNode>::type parent;
        //当前节点中数据的数量
        int dataNum;

        //默认构造函数
        BTreeNode() : datas{}, datasPtr{}, parent{nullptr}, dataNum{0} {}

        void insertData(int pos,const T& data){
            for(int i=dataNum;i>pos;--i){
                datas[i]=datas[i-1];
            }
            ++dataNum;
            datas[pos]=data;
        }

        T removeData(int pos){
            const T val=datas[pos];
            for(int i=pos;i<dataNum-1;++i){
                datas[i]=datas[i+1];
            }
            --dataNum;
            return val;
        }

        void insertPointer(int pos,typename std::add_pointer<BTreeNode>::type ptr){
            for(int i=N;i>pos;--i){
                datasPtr[i]=datasPtr[i-1];
            }
            datasPtr[pos]=ptr;
        }

        typename std::add_pointer<BTreeNode>::type removePointer(int pos){
            typename std::add_pointer<BTreeNode>::type ptr=datasPtr[pos];
            for(int i=pos;i<N;++i){
                datasPtr[i]=datasPtr[i+1];
            }
            return ptr;
        }

        int findData(const T&val){
            for(int i=0;i<dataNum;++i){
                if(std::equal_to<T>()(datas[i],val))
                    return i;
            }
            return N;
        }

        int findPtr(typename std::add_pointer<BTreeNode>::type ptr){
            for(int i=0;i<N+1;i++){
                if(std::equal_to<typename std::add_pointer<BTreeNode>::type>()(datasPtr[i],ptr))
                    return i;
            }
            return N+1;
        }

    };

    //根节点
    typename std::add_pointer<BTreeNode>::type root;
    //离上一次查找过程最近的节点
    typename std::add_pointer<BTreeNode>::type hot;

    void slideElemsRight(typename std::array<T, N>::iterator beg, typename std::array<T, N>::iterator end){
        for (auto iter = end; iter >= beg; --iter)
            *iter = *(iter - 1);
    }

    void slidePtrsRight(typename std::array<typename std::add_pointer<BTreeNode>::type, N + 1>::iterator beg,
                        typename std::array<typename std::add_pointer<BTreeNode>::type, N + 1>::iterator end){
        for (auto iter = end; iter >= beg; --iter)
            *iter = *(iter - 1);
    }

    void solveOverFlow(typename std::add_pointer<BTreeNode>::type curTree){
        if (curTree->dataNum < N)
            return;
        int pivot = N / 2;
        T &val = curTree->datas[pivot];
        typename std::add_pointer<BTreeNode>::type parent = curTree->parent != nullptr ? curTree->parent : new BTreeNode();
        //根节点
        if (curTree->parent == nullptr)
            root = parent;
        typename std::add_pointer<BTreeNode>::type lchild = curTree;
        lchild->dataNum = N / 2;
        lchild->parent = parent;

        typename std::add_pointer<BTreeNode>::type rchild = new BTreeNode();
        rchild->dataNum = N - lchild->dataNum - 1;
        rchild->parent = lchild->parent;

        //将分裂后的节点的右半部分的数据拷贝到右子树中
        std::copy(lchild->datas.begin() + pivot + 1, lchild->datas.begin() + N, rchild->datas.begin());

        //处理子树的指针,如果子树不是叶子节点,将其右半部分的子树的父母设为右子树,如果是叶子节点则什么也不用做
        if (lchild->datasPtr[0] != nullptr){
            for (auto beg = pivot + 1; beg < N+1; ++beg)
                lchild->datasPtr[beg]->parent = rchild;
            //将指针拷贝到右子树中
            std::copy(lchild->datasPtr.begin() + pivot + 1, lchild->datasPtr.begin() + N + 1, rchild->datasPtr.begin());
        }

        //原来就有的节点
        if (parent->dataNum != 0){
            //将分裂出的中间节点插入父节点中
            //找位置
            auto curIter = std::find_if(parent->datas.begin(), parent->datas.begin() + parent->dataNum, [&val](const T &val_) { return !pred()(val_, val); });
            //将值插入进去
            slideElemsRight(curIter, parent->datas.begin() + parent->dataNum);
            ++parent->dataNum;
            *curIter = val;
        }
        //新分配的根节点
        else{
            ++parent->dataNum;
            parent->datas[0]=val;
        }
        

        //新分配的根节点
        if (parent->datasPtr[0] == nullptr){
            parent->datasPtr[0] = lchild;
            parent->datasPtr[1] = rchild;
        }
        else{
            //调整指向子孩子的指针
            auto iterPtr = std::find(parent->datasPtr.begin(), parent->datasPtr.end(), curTree);
            //here
            slidePtrsRight(iterPtr, parent->datasPtr.begin() + parent->dataNum );
            *iterPtr = lchild;
            *(iterPtr + 1) = rchild;
        }

        solveOverFlow(parent);
    }

    void solveUnderflow(typename std::add_pointer<BTreeNode>::type curTree){
        if(curTree->dataNum>=((Rank+1)/2-1)){
            return;
        }

        //处理根节点
        if(!curTree->parent){
            if(curTree->dataNum==0){
                root=curTree->datasPtr[0];
                delete curTree;
                if(root)
                    root->parent=nullptr;
                return;
            }
            return;
        }

        typename std::add_pointer<BTreeNode>::type parent=curTree->parent;
        int ptr_index=parent->findPtr(curTree);

        //存在左兄弟
        if(ptr_index>0){
            typename std::add_pointer<BTreeNode>::type lsb=parent->datasPtr[ptr_index-1];
            //左兄弟中有足够的关键码
            if(lsb->dataNum>=(Rank+1)/2){
                //将父节点中的对应数据挪到当前节点
                curTree->insertData(0,parent->removeData(ptr_index-1));
                //将左节点的对应数据挪到父节点
                parent->insertData(ptr_index-1,lsb->removeData(lsb->dataNum-1));
                //调整子树指针

                curTree->insertPointer(0,lsb->removePointer(lsb->dataNum+1));
                if(curTree->datasPtr[0]){
                    curTree->datasPtr[0]->parent=curTree;
                }
            }
        }else if(ptr_index<Rank&&curTree->dataNum<((Rank+1)/2-1)){ //存在右兄弟
            typename std::add_pointer<BTreeNode>::type rsb=parent->datasPtr[ptr_index+1];
            //右兄弟中右足够的关键码
            if(rsb->dataNum>=(Rank+1)/2){
                //将父节点中的对应数据挪到当前节点
                curTree->insertData(curTree->dataNum,parent->removeData(ptr_index));
                //将右节点中的数据挪到父节点
                parent->insertData(ptr_index,rsb->removeData(0));

                //调整子树指针
                curTree->insertPointer(curTree->dataNum,rsb->removePointer(0));
                if(curTree->datasPtr[curTree->dataNum]){
                    curTree->datasPtr[curTree->dataNum]->parent=curTree->datasPtr[curTree->dataNum];
                }
            }
        }
        
        if(curTree->dataNum<((Rank+1)/2-1)){//左右节点均没有可借的数据项，只能合并节点
            if(ptr_index>0){//合并左节点
                typename std::add_pointer<BTreeNode>::type lsb=parent->datasPtr[ptr_index-1];
                lsb->insertData(lsb->dataNum,parent->removeData(ptr_index-1));
                parent->removePointer(ptr_index);
                while (curTree->dataNum){
                    lsb->insertPointer(lsb->dataNum,curTree->removePointer(0));
                    if(lsb->datasPtr[lsb->dataNum])
                        lsb->datasPtr[lsb->dataNum]->parent=lsb;
                    lsb->insertData(lsb->dataNum,curTree->removeData(0));
                }
                lsb->insertPointer(lsb->dataNum,curTree->removePointer(0));
                if(lsb->datasPtr[lsb->dataNum])
                    lsb->datasPtr[lsb->dataNum]->parent=lsb;
                delete curTree;
            }else if(ptr_index<Rank){ //合并右节点
                typename std::add_pointer<BTreeNode>::type rsb=parent->datasPtr[ptr_index+1];
                curTree->insertData(curTree->dataNum,parent->removeData(ptr_index));
                parent->removePointer(ptr_index+1);
                while (rsb->dataNum){
                    curTree->insertPointer(curTree->dataNum,rsb->removePointer(0));
                    if(curTree->datasPtr[curTree->dataNum])
                        curTree->datasPtr[curTree->dataNum]->parent=curTree;
                    curTree->insertData(curTree->dataNum,rsb->removeData(0));
                }
                curTree->insertPointer(curTree->dataNum,rsb->removePointer(0));
                if(curTree->datasPtr[curTree->dataNum])
                    curTree->datasPtr[curTree->dataNum]->parent=curTree;
                delete rsb;
            }
        }
        solveUnderflow(parent);
    }

public:
    BTree() : root{new BTreeNode()} {}

    bool contains(const T &val){
        typename std::array<T, N - 1>::iterator curIter;
        typename std::add_pointer<BTreeNode>::type curNode = root;
        hot = curNode;
        while (true){
            if (curNode&&curNode->dataNum != 0){
                curIter = std::find_if(curNode->datas.begin(), curNode->datas.begin() + curNode->dataNum,
                                       [&val](const T &val_) { return !pred()(val_, val); });
                if (std::equal_to<T>()(val, *curIter)&&curIter-curNode->datas.begin()<curNode->dataNum)
                    return true;
                int index = curIter - curNode->datas.begin();
                curNode = curNode->datasPtr[index];
                if (curNode == nullptr)
                    break;
                hot = curNode;
            }
            else
                break;
        }
        return false;
    }

    bool insert(const T &val){
        if (contains(val))
            return false;
        auto curIter = std::find_if(hot->datas.begin(), hot->datas.begin() + hot->dataNum, [&val](const T &val_) { return !pred()(val_, val); });
        //将值插入进去
        slideElemsRight(curIter, hot->datas.begin() + hot->dataNum);
        ++hot->dataNum;
        *curIter = val;
        //不用上溢
        if (hot->dataNum < N)
            return true;
        solveOverFlow(hot);
        return true;
    }

    bool remove(const T &val){
        if(!contains(val)){
            return false;
        }
        int index=hot->findData(val);
        if(hot->datasPtr[0]){
            typename std::add_pointer<BTreeNode>::type cur_iter=hot->datasPtr[index+1];
            while(cur_iter->datasPtr[0]){
                cur_iter=cur_iter->datasPtr[0];
            }
            hot->datas[index]=cur_iter->datas[0];
            cur_iter->removeData(0);
            solveUnderflow(cur_iter);
        }else{
            hot->removeData(index);
            solveUnderflow(hot);
        }
        return true;
    }
};

#endif