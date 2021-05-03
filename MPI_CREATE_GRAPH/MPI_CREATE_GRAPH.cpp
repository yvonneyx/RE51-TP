#include <iostream>
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
//#include "stdfix.h"
using namespace std;

int main( int argc, char *argv[] ) {
    int errs = 0, i, k, neighbourNumber,j;
    int size = 4;
    int topo_type;
    int *index, *edges, *outindex, *outedges, *neighbours;
    //配置MPI的初始环境生成两个通讯器comm1和comm2
    MPI_Comm comm1, comm2;
    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &size );    // 通过size获得进程数

    if (size >= 3) {       // 如果进程数大于等于三才开始生成拓扑图，个人理解是两个节点就不需要用图分析了，连二叉都凑不齐
        index  = new int(size);
        edges = new int(size * 2 );     //C++指针赋值前要分配空间不然会报空指针
        neighbours = new int(size * 2 );
        if (!index || !edges)//异常处理如果指针内存分配失败就不用继续后面的操作了
            {
            cout<<"无法分配"<< 3 *  size<<"  oct的内存空间给index和edges "<<endl;
            MPI_Abort( MPI_COMM_WORLD, 1 );
        }

        //对照之前的表输入index和edges
        index[0]=1;
        index[1]=3;
        index[2]=5;
        index[3]=6;
        edges[0]=1;
        edges[1]=0;
        edges[2]=2;
        edges[3]=1;
        edges[4]=3;
        edges[5]=2;

        //生成拓扑图根据结点数 ，index和edges设置为不可重构生成的拓扑图存储到comm1内
        MPI_Graph_create( MPI_COMM_WORLD, size, index, edges, 0, &comm1 );

        //复制comm1到comm2
        MPI_Comm_dup( comm1, &comm2 );

        //对comm2内的拓扑图进行判断将其拓扑类型保存在topo_type内
        MPI_Topo_test( comm2, &topo_type );

        cout<< "拓扑类型是" <<topo_type<<endl;

        if (topo_type != MPI_GRAPH) { //如果拓扑类型不是MPI_GRAPH就报错
            errs++;
            cout<<"该拓扑类型不是MPI图拓扑" <<endl;
        }
        else {
            int nnodes, nedges;
            //通过comm2获取相关拓扑图的信息结点数和边数
            MPI_Graphdims_get( comm2, &nnodes, &nedges );

       //指针使用前要分配内存空间
        outindex = new int(size);
        outedges = new int(size * 2);

        MPI_Graph_get ( comm2, size, 2*size, outindex, outedges);

        cout<<"-------------------------------------"<<endl;
        for( k=0;k<size;k++){
            MPI_Graph_neighbors_count(comm2,k,&neighbourNumber);
            cout<<"我是"<<k<<"结点 ，我有 "<<neighbourNumber<<" 个邻结点"<<endl;
            MPI_Graph_neighbors(comm2,k,neighbourNumber,neighbours);
            cout<<"我的邻结点是: ";
            for(i=0;i<neighbourNumber;i++){
                cout<<neighbours[i]<<"|";
            }
            cout<<endl;
        }
        //释放内存空间
        delete outindex;
        delete outedges;
        delete index;
        delete edges;
        MPI_Comm_free(&comm2);
        MPI_Comm_free(&comm1);
        }

        MPI_Finalize();
        return 0;
    }
}
