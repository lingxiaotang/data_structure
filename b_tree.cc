#include<iostream>
#include<array>
#include<algorithm>
#include"b_tree.h"
#include<vector>
#include<iterator>



int main(int argc, char const *argv[])
{
    BTree<int,120> bTree;
    for(int i=1;i<=12000;i++)
    {
        bTree.insert(i);
        std::cout<<"finished "<<i<<std::endl;
    }
    
    for(int i=12000;i>=1;i--)
    {
        if(!bTree.contains(i))
            std::cout<<"doesn't contain "<<i<<std::endl;
    }


    for(int i=12000;i>=1;i--){
        bTree.remove(i);
        if(bTree.contains(i))
            std::cout<<"contains "<<i<<std::endl;
    }
    return 0;
}
