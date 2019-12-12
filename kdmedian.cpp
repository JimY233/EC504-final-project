/***************************************************
KD-tree
***********************************************/
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <time.h>
#include <sstream>
#include <vector>
#include <chrono>

using namespace std;

#define PI 3.1415926

double distance(double point1[], double point2[]) 
{ 
    double d2 = 0;
    double x = (point1[1]-point2[1])*cos((point1[0]+point2[0])*PI/360);
    double y = (point1[0]-point2[0]); 
    d2 = x*x+y*y;
    return sqrt(d2); 
} 
/*

double* transform(double latitude, double longitude)
{
    double *point = new double [3];
    double rad = 6371000;
    double x,y,z;
    double lon = longitude*PI/180;
    double lat = latitude*PI/180;
    point[0] = rad * cos(lon) * sin(lat);
    point[1] = rad * sin(lon) * sin(lat);
    point[2] = rad * cos(lat);
    return point;
}
*/

const int k = 2; //dimension
  
struct Data
{
    string state;
    string county;
    double point[k];
};

void swap(struct Data* data, int i, int j)  
{  
    struct Data temp;
    temp = data[i];
    data[i] = data[j];
    data[j] = temp;
}  

int  partition(struct Data* data, int split, int left, int right)  
{  
    double piv = data[right].point[split];
    int i = left - 1;  
    for (int j = left;j<right;j++)  
    {    
        if (data[j].point[split] < piv)  
        {  
            i++;  
            swap(data,i,j);  
        }  
    }  
    swap(data,i+1,right);  
    return i+1;  
}  

struct Data* quicksort(struct Data* data, int split, int left, int right)
{
    if (left < right)  
    { 
        int pi = partition(data, split, left, right);  
        data = quicksort(data, split,left, pi-1);  
        data = quicksort(data, split, pi+1, right);  
    }  
    return data;
}

struct Data* Sort(struct Data* data, int split, int n)
{
    quicksort(data,split,0,n-1);
    return data;
}

struct Data* sortdata(struct Data* data, double point[],int K)
{
    double a, b;
    struct Data temp;
    for(int i=0;i<K;i++)
    {
        for(int j=0;j<K-i-1;j++) //attention not n-i
        { 
            a = distance(data[j].point,point);
            b = distance(data[j+1].point,point);
            if(a>b)
            {
                temp = data[j];
                data[j] = data[j+1];
                data[j+1] = temp;
            }
        }
    }
    return data;
}

struct Data* adddata(struct Data* data, struct Data possible,double point[], int K)
{
    for(int i=0;i<K;i++)
    {
        if(distance(point,possible.point)<distance(point,data[i].point))
        {
            data[i].state = possible.state;
            data[i].county = possible.county;
            data[i].point[0] = possible.point[0];
            data[i].point[1] = possible.point[1];
            return data;
        }
    }
    sortdata(data,point,K);
    return data;
}

/*
struct Rect
{
    double *min;
    double *max;
};
*/
struct Node
{ 
    bool isLeaf,isLeft,isRight;
    //bool visited;
    int split; // split dim
    double pivot;
    //struct Data *data;
    string state;
    string county;
    double lat,lon;
    //struct Rect *rect;
    struct Node *parent, *left, *right; 
}; 

struct Node* build(struct Node *root,struct Data* data,unsigned depth,int n) 
{
    unsigned split = depth % k; //split dim

    if(n==0) 
    {
        root = NULL;
        return root;
    }

    //root->visited = false;

    if(n==1)  
    {
        root->split = split;
        root->pivot = data->point[split];
        root->isLeaf = true;
        root->left = NULL;
        root->right = NULL;
        root->state = data->state;
        root->county = data->county;
        root->lat = data->point[0];
        root->lon = data->point[1];
        return root;
    }

    root->isLeaf = false;

    data = Sort(data,split,n);
    int mid = (n+1)/2;
    struct Data* Leftdata = new Data [mid];
    struct Data* Rightdata = new Data [n-mid];
    struct Data midData;
    midData = data[mid];

    for(int i=0;i<mid;i++)
    {
        Leftdata[i] = data[i];
    }
    for(int i=0;i<n-mid-1;i++)
    {
        Rightdata[i] = data[mid+i+1]; 
    }

    root->left = new Node;
    root->left->parent = root;
    root->left->isLeft = true;
    root->left->isRight = false;

    root->right = new Node;
    root->right->parent = root;
    root->right->isLeft = false;
    root->right->isRight = true;

    root->split = split;
    root->pivot = midData.point[split];
    root->state = midData.state;
    root->county = midData.county;
    root->lat = midData.point[0];
    root->lon = midData.point[1];

    root->left = build(root->left,Leftdata,depth+1,mid);
    root->right = build(root->right,Rightdata,depth+1,n-mid);

    return root;
}

struct Node* searchdown(double point[], struct Node* current)
{
    while(!current->isLeaf)
    {
        unsigned dim = current->split;
        if(point[dim]<current->pivot)
        {
            if(current->left!=NULL)
              current = current->left;
            else 
              return current;            
        }
        else
        {
            if(current->right!=NULL)
              current = current->right;
            else 
              return current; 
        }
    }
    //struct Data* nearest = current->data;
    //double mindist = distance(point,nearest->point);
    return current;
}

struct Data searchNN(double point[], struct Node* root)
{
    struct Node *current;
    current = root; 
    current = searchdown(point,current);
    struct Data nearestdata;
    nearestdata.state = current->state;
    nearestdata.county = current->county;
    nearestdata.point[0] = current->lat;
    nearestdata.point[1] = current->lon;
    double mindist = distance(point,nearestdata.point);
    //current->visited = true;

    while(current->parent!=NULL)
    {
        current = current->parent;
        struct Data possible;
        unsigned dim = current->split;
        double rectdist = fabs(current->pivot - point[dim]);
        if(rectdist < mindist)
        {
            possible.state = current->state;
            possible.county = current->county;
            possible.point[0] = current->lat;
            possible.point[1] = current->lon;
            if(distance(point,possible.point)<distance(point,nearestdata.point))
            {
                nearestdata.state = possible.state;
                nearestdata.county = possible.county;
                nearestdata.point[0] = possible.point[0];
                nearestdata.point[1] = possible.point[1];
            }

            possible.state = current->left->state;
            possible.county = current->left->county;
            possible.point[0] = current->left->lat;
            possible.point[1] = current->left->lon;
            if(distance(point,possible.point)<distance(point,nearestdata.point))
            {
                nearestdata.state = possible.state;
                nearestdata.county = possible.county;
                nearestdata.point[0] = possible.point[0];
                nearestdata.point[1] = possible.point[1];
            }

            possible.state = current->right->state;
            possible.county = current->right->county;
            possible.point[0] = current->right->lat;
            possible.point[1] = current->right->lon;
            if(distance(point,possible.point)<distance(point,nearestdata.point))
            {
                nearestdata.state = possible.state;
                nearestdata.county = possible.county;
                nearestdata.point[0] = possible.point[0];
                nearestdata.point[1] = possible.point[1];
            }
        }
    } 
    return nearestdata;
}


struct Data* search(double point[], struct Node* root, int K)
{
    struct Data *knearestdata = new Data[K];
    struct Node *current;
    current = root;
    current = searchdown(point,current);
    knearestdata[0].state = current->state;
    knearestdata[0].county = current->county;
    knearestdata[0].point[0] = current->lat;
    knearestdata[0].point[1] = current->lon;
    //double mindist = distance(point,nearestdata.point);
    for(int i=1;i<K;i++)
    {
        current = current->parent;
        knearestdata[i].state = current->state;
        knearestdata[i].county = current->county;
        knearestdata[i].point[0] = current->lat;
        knearestdata[i].point[1] = current->lon;
    }
    sortdata(knearestdata,point,K);

    while(current->parent!=NULL)
    {
        current = current->parent;
        struct Data possible;
        unsigned dim = current->split;
        possible.state = current->state;
        possible.county = current->county;
        possible.point[0] = current->lat;
        possible.point[1] = current->lon;
        adddata(knearestdata,possible,point,K);
    } 
    return knearestdata;
}

int main(int argc, char *argv[])
{ 
    chrono::time_point<chrono::steady_clock> start, stop; 
    chrono::duration<double> difference_in_time;
    double building;
    double querytest,querytime;

    Data *inputdata = new Data[2300000];
    //struct Data *inputdata = (struct Data *)malloc(sizeof(struct Data)*20000);
    ifstream fin;
	fin.open("NationalFile_StateProvinceDecimalLatLong.txt");
    string str;
    getline(fin,str);
    int N = 0;
    vector<string> res;
    //for(int i=0;i<2000;i++)
    while (!fin.eof())
    {
        getline(fin,str);
        if(str.empty())
          break;
        stringstream input(str);
        string result;
        while(input>>result) {res.push_back(result);}
        int len = res.size();
        inputdata[N].state = res[0];
        inputdata[N].county = res[1];
        if(len>4)
        {
            for(int j=2;j<len-2;j++)
            {
                inputdata[N].county += " "+res[j];
            }
        }
        inputdata[N].point[0] = stod(res[len-2]);
        inputdata[N].point[1] = stod(res[len-1]);
        N = N+1;
        res.clear();
    }
    fin.close();

    start = chrono::steady_clock::now();
    struct Node *tree = new Node; 
    tree->split = 0;
    tree->pivot = 0;
    tree->parent = NULL;
    tree = build(tree,inputdata,0,N);
    stop = chrono::steady_clock::now();

    difference_in_time = stop - start;
    building = double(difference_in_time.count());
    cout<<"build time is "<<building<<endl;
    cout<<"points number"<<N<<endl;
    
    //test data
    double point[2];
    point[0] = 33.24;
    point[1] = -112.75;
    int K = 1;
    cout<<"test start"<<endl; 

    start = chrono::steady_clock::now();
    if(K==1)
    {
      struct Data ans;
      ans = searchNN(point,tree);
      stop = chrono::steady_clock::now();
      cout<<ans.state<<" "<<ans.county<<" "<<endl;
      cout<<ans.point[0]<<" "<<ans.point[1]<<endl;
    }
    else
    {
      struct Data* answer;
      answer = search(point,tree,K);
      stop = chrono::steady_clock::now();
      for(int j=0;j<K;j++)
      {
        cout<<answer[j].state<<" "<<answer[j].county<<" "<<endl;
        cout<<answer[j].point[0]<<" "<<answer[j].point[1]<<endl;
      }
    }
    difference_in_time = stop - start;
    querytest = double(difference_in_time.count());
    cout<<"search time is "<<querytest<<endl;
    cout<<"test finished"<<endl;
    cout<<""<<endl;

    string YN;
    while(1)
    {
        cout<<"input K(from 1 to 10)"<<endl;
        cin>>K;
        cout<<"input latitude"<<endl;
        cin>>point[0];
        cout<<"input longitude"<<endl;
        cin>>point[1];
        start = chrono::steady_clock::now();
        if(K==1)
        {
            struct Data ans;
            ans = searchNN(point,tree);
            stop = chrono::steady_clock::now();
            cout<<ans.state<<" "<<ans.county<<" "<<endl;
            cout<<ans.point[0]<<" "<<ans.point[1]<<endl;
        }
        else
        {
            struct Data* answer;
            answer = search(point,tree,K);
            stop = chrono::steady_clock::now();
            for(int j=0;j<K;j++)
            {
              cout<<answer[j].state<<" "<<answer[j].county<<" "<<endl;
              cout<<answer[j].point[0]<<" "<<answer[j].point[1]<<endl;
            }
        }
        difference_in_time = stop - start;
        querytime = double(difference_in_time.count());
        cout<<"search time is "<<querytime<<endl;
        cout<<"voting by 5 nearest points"<<endl;
        K = 5;
        struct Data* answers;
        answers = search(point,tree,K);
        if(answers[0].county==answers[1].county)
          cout<<answers[0].state<<" "<<answers[0].county<<" "<<endl;
        else
        {
            int vote0,vote1;
            for(int v=2;v<5;v++)
            {
                if(answers[v].county==answers[0].county)
                  vote0 = vote0+1;
                if(answers[v].county==answers[1].county)
                  vote1 = vote1+1;
            }
            if(vote0>vote1)
              cout<<answers[0].state<<" "<<answers[0].county<<" "<<endl;
            else
              cout<<answers[1].state<<" "<<answers[1].county<<" "<<endl;
        }
        cout<<"continue: Y or N"<<endl;
        cin>>YN;
        if(YN == "N")
          break;
        else
          continue;
    }

    return 0;
}