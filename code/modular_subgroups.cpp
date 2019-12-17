#include "globalstate.h"
#include "code.h"
#include "timing.h"
#include "eval.h"
#include <map>
#include "ex_cont.h"


//TODO: get rid of these defines and get rid of the bad code

#define LAB_CT 2
#define u32 uint32_t
#define i32 int32_t
#define u64 uint64_t
#define i64 int64_t


u32 GCD(u32 a, u32 b) {
   for(;;){ if(a==0){return b;} b%=a; if(b==0){return a;} a%=b;}
}
u64 GCD(u64 a, u64 b) {
   for(;;){ if(a==0){return b;} b%=a; if(b==0){return a;} a%=b;}
}
u32 LCM(u32 a, u32 b) {
   u32 t=GCD(a,b); return t ? ((a/t)*b) : 0;
}
u64 LCM(u64 a, u64 b) {
   u64 t=GCD(a,b); return t ? ((a/t)*b) : 0;
}
inline i64 Sign(i64 x) {
   return (x != 0) || (x >> 63);
}
inline i64 FlipSign(i64 x, i64 y) {
   return (x ^ (y >> 63)) - (y >> 63);
}
inline i64 Abs(i64 x) {
   return FlipSign(x,x);
}



bool pmIdQ(u32 n, u32 *p);
bool pmSameQ(u32 n, u32 *p, u32 *q);
bool pmValidQ(u32 n, u32*p);
void pmId(u32 n, u32*p);
void pmMov(u32 n, u32*p, u32*q);
void pmPow (u32 n,u32*x, u32*y, i32 p, i32*b, u32*t);
void pmInv(u32 n, u32*p, u32*q);
void pmMulInv(u32 n, u32*p, u32*q, u32*r);
bool pmMulInvIdQ(u32 n, u32*p, u32*q, u32*r);
void pmMul(u32 n, u32*p, u32*q, u32*r);
void pmMul(u32 n, u32*p, u32*q, u32*r, u32*s);
void pmMul(u32 n, u32*p, u32*q, u32*r, u32*s, u32*t);
void pmMul(u32 n, u32*p, u32*q, u32*r, u32*s, u32*t, u32*u);
void pmPrintCycles(u32 n, u32*p);
u32 pmFixedCount(u32 n, u32*p);
u32 pmCycleCount(u32 n, u32*p, u32*vis);
u32 pmOrder(u32 n, u32*p, u32*vis);



void pmId(u32 n, u32*p) {
   u32 i=0; do {p[i]=i;} while (++i<n);
}

void pmMov(u32 n, u32*p, u32*q) {
   u32 i=0; do {p[i]=q[i];} while (++i<n);
}

void pmInv(u32 n, u32*p, u32*q) {
   u32 i=0; do {p[q[i]]=i;} while (++i<n);
}

void pmMulInv(u32 n, u32*p, u32*q, u32*r) {
   u32 i=0; do {p[r[i]]=q[i];} while (++i<n);
}
bool pmMulInvIdQ(u32 n, u32*p, u32*q, u32*r) {
   bool x = true;
   u32 a, b;
   u32 i=0; do {a=r[i]; b=q[i]; p[a]=b; x&=(a==b);} while (++i<n);
   return x;
}


void pmMul(u32 n, u32*p, u32*q, u32*r) {
   u32 i=0; do {p[i]=q[r[i]];} while (++i<n);
}
void pmMul(u32 n, u32*p, u32*q, u32*r, u32*s) {
   u32 i=0; do {p[i]=q[r[s[i]]];} while (++i<n);
}
void pmMul(u32 n, u32*p, u32*q, u32*r, u32*s, u32*t) {
   u32 i=0; do {p[i]=q[r[s[t[i]]]];} while (++i<n);
}
void pmMul(u32 n, u32*p, u32*q, u32*r, u32*s, u32*t, u32*u) {
   u32 i=0; do {p[i]=q[r[s[t[u[i]]]]];} while (++i<n);
}

void pmPow (u32 n, u32*x, u32*y, i32 p, u32*b, u32*t) {
   if (p<0) {p=-p;pmInv(n,b,y);}
   else {pmMov(n,b,y);}
   pmId(n,x);
   while (p>0) {
      if (p&1) {pmMul(n,t,x,b);pmMov(n,x,t);}
      p=p/2;
      pmMul(n,t,b,b);pmMov(n,b,t);
   }
}

bool pmIdQ(u32 n, u32 *p){
   u32 i=0; do {if (p[i]!=i) return false;} while (++i<n);
   return true;
}

bool pmSameQ(u32 n, u32 *p, u32 *q){
   u32 i=0; do {if (p[i]!=q[i]) return false;} while (++i<n);
   return true;
}

u32 pmFixedCount(u32 n, u32*p) {
   u32 ct=0;
   u32 i=0; do {ct+=(p[i]==i);} while (++i<n);
   return ct;
}

u32 pmCycleCount(u32 n, u32*p, u32*vis) {
   u32 i, j, k, ct = 0;
   for (i=0;i<n;i++) {vis[i]=0;};
   for (k=0;k<n;k++) {
      if (vis[k]==0) {
         vis[k]=1;
         for(j=p[k]; j!=k; j=p[j]) {
            assert(vis[j]==0);
            vis[j]=1;
         }
         ct++;
      }
   }
   return ct;
}

u32 pmOrder(u32 n, u32*p, u32*vis) {
   u32 i, j, k, ct = 0, lcm=1;
   for(i=0;i<n;i++){vis[i]=0;};
   for(k=0;k<n;k++) {
      if (vis[k]==0) {
         vis[k]=1;
         ct=1;
         for(j=p[k]; j!=k; j=p[j]) {
            assert(vis[j]==0);
            vis[j]=1;
            ct++;
         }
         lcm = LCM(lcm,ct);
      }
   }
   return lcm;
}

bool pmValidQ(u32 n, u32*p) {
   u32 i, j, k;
   bool * vis = new bool[n];
   for (i=0;i<n;i++){vis[i]=false;};
   for (i=0;i<n;i++){
      if ((p[i]>=n)||vis[p[i]]){ delete[] vis; return false;}
      vis[p[i]]=true;
   };
   delete[] vis;
   return true;
}

void pmPrintCycles(u32 n, u32*p) {
   u32 i, j, k, ct=0;
   bool * visited = new bool[n];
   assert(pmValidQ(n,p));
   //printf("{");
   for(i=0;i<n;i++){visited[i]=false;};
   for(k=0;k<n;k++) {
      if (!visited[k]) {
         printf("(%d",k+1); ct++; visited[k]=true;
         for(j=p[k]; j!=k; j=p[j]) {
            printf(",%d",j+1); ct++; visited[j]=true;
         }
         printf(")");
         //if (ct!=n) {printf(", ");}
      }
   }
   //printf("}");
   delete[] visited;
   return;
}



struct DisjointSetNode {
   u32 parent;
   u32 rank;
};

class DisjointSet {
   u32 size;
   DisjointSetNode* nodes;
   public:
   DisjointSet() {
      size=0; nodes=NULL;
   }
   DisjointSet(u32 n) {
      size=n;
      nodes = new DisjointSetNode[n];
      for (u32 i=0; i<n; i++) {
         nodes[i].rank=0; nodes[i].parent=i;
      }
   };
   void Set(u32 n) {
      if (nodes) {delete [] nodes;}
      size=n;
      nodes = new DisjointSetNode[n];
      for (u32 i=0; i<n; i++) {
         nodes[i].rank=0; nodes[i].parent=i;
      }
   }
   ~DisjointSet() {
      delete [] nodes;
      size=0;
   };
   void Print() {
      printf("{");
      for(u32 i=0;i<size;i++){
         printf("%d",nodes[i].parent);
         if(i+1<size) {printf(", ");}
      }
      printf("}");
   }
   u32 Find(u32 n) {
      if (nodes[n].parent==n) {return n;}
      else {return nodes[n].parent=Find(nodes[n].parent);}
   };
   u32 QFind(u32 n) {  // does not modify the forest
      if (nodes[n].parent==n) {return n;}
      else {return Find(nodes[n].parent);}
   };
   bool RootQ(u32 n) {return nodes[n].parent==n;};
   u32 Merge(u32 X, u32 Y) {
      u32 x=Find(X), y=Find(Y);
      if (x==y) {return -1;}
      if (nodes[x].rank>nodes[y].rank) {return nodes[y].parent=x;}
      else if (nodes[y].rank>nodes[x].rank) {return nodes[x].parent=y;}
      else {nodes[x].rank=nodes[x].rank+1; return nodes[y].parent=x;}
   }
};


struct PrevNext {
   u32 prev;
   u32 next;   
};

struct HeadTail {
   u32 head;
   u32 tail;
};

struct GraphEdge {
   u32 ini;
   u32 ter;
   u32 lab;  //lab=-1 means no label, otherwise lab should be <LAB_CT
   u32 extra;
   GraphEdge() {;};
   GraphEdge(u32 ini1, u32 lab1, u32 ter1) {ini=ini1;lab=lab1;ter=ter1;};
};

void PrintNodes(HeadTail &ht, PrevNext*np);
bool NodeElemQ(u32 c, HeadTail &ht, PrevNext*np);
bool NodeFreeQ(u32 c, HeadTail &ht, PrevNext*np);
void AppendNode(u32 c, HeadTail &ht, PrevNext*np);
void DeleteNode(u32 c, HeadTail &ht, PrevNext*np);
void JoinNodes(u32 l1, u32 l2, HeadTail*ht, PrevNext*np);

void PrintNodes(HeadTail &ht, PrevNext*np) {
   printf("{");
   for (u32 h=ht.head; h!=-1; h=np[h].next) {
      printf("%d",h);
      if (np[h].next!=-1) {printf(", ");}
   }
   printf("}%d",ht.tail);
}

bool NodeElemQ(u32 c, HeadTail &ht, PrevNext*np) {
   return np[c].next!=-1 || ht.tail==c;
}
bool NodeFreeQ(u32 c, HeadTail &ht, PrevNext*np) {
   return np[c].next==-1 && ht.tail!=c;
}
void AppendNode(u32 c, HeadTail &ht, PrevNext*np) {
   u32 h=ht.head, t=ht.tail;
   if (h==-1) {ht.head=c; ht.tail=c; np[c].next=-1; np[c].prev=-1;}
   else {ht.tail=c; np[t].next=c; np[c].prev=t; np[c].next=-1;}
}
void DeleteNode(u32 c, HeadTail &ht, PrevNext*np) {
   u32 h=ht.head, t=ht.tail;
   u32 p=np[c].prev, n=np[c].next;
   if      (h==c&&c==t) {ht.head=-1;   ht.tail=-1;}
	else if (h==c&&c!=t) {ht.head=n;    np[n].prev=-1;}
	else if (h!=c&&c==t) {ht.tail=p;    np[p].next=-1;}
	else                 {np[p].next=n; np[n].prev=p;}
   np[c].next=-1; np[c].prev=-1;
}
void JoinNodes(u32 l1, u32 l2, HeadTail*ht, PrevNext*np) {
   u32 h1=ht[l1].head, t1=ht[l1].tail;
   u32 h2=ht[l2].head, t2=ht[l2].tail;
   if      (h2==-1) {return;}
   else if (h1==-1) {ht[l1].head=h2; ht[l1].tail=t2; ht[l2].head=-1;  ht[l2].tail=-1;}
   else  {np[t1].next=h2; np[h2].prev=t1; ht[l1].tail=t2; ht[l2].head=-1; ht[l2].tail=-1;}
}



class Graph {
   u32 ep; //edge count
   u32 nu; //vertex count
   DisjointSet ds;
   GraphEdge*edges;
   PrevNext*edgeIniPN; // edgeIniPN[e].next = next edge that comes from initial vertex of e
   PrevNext*edgeTerPN; // dido for terminals
   HeadTail*vertIniHT; // vertexIniHT[v].head = head of the list of edges that come from vertex v
   HeadTail*vertTerHT; // dido for terminals
   PrevNext*checkPN;  // checkPN[v].next = next vertex after v in the list of vertices that need to be checked
   HeadTail checkHT;  // checkHT.head = head of list of list of vertices that need to be checked
   public:
   friend bool NodeElemQ(u32 c, HeadTail &ht, PrevNext*np);
   friend bool NodeFreeQ(u32 c, HeadTail &ht, PrevNext*np);
   friend void AppendNode(u32 c, HeadTail &ht, PrevNext*np);
   friend void DeleteNode(u32 c, HeadTail &ht, PrevNext*np);
   friend void JoinNodes(u32 l1, u32 l2, HeadTail*ht, PrevNext*np);
   void Initialize(u32 Rep, u32 Rnu);
   void Print();
   void AddEdge(u32 k, u32 ini, u32 lab, u32 ter);
   void MergeVertices(u32 v1, u32 v2);
   bool FoldedQ();
   void Fold();
   void ToGroup(std::vector<u32> &S, std::vector<u32> &O);
   Graph(u32 ep_, u32 nu_);
   Graph(u32 mu, u32*S, u32*O);
   ~Graph();
};

Graph::~Graph() {
   delete [] edges;
   delete [] edgeIniPN;
   delete [] edgeTerPN;
   delete [] vertIniHT;
   delete [] vertTerHT;
   delete [] checkPN;
}


void Graph::Initialize(u32 Rep, u32 Rnu) {
   ep=Rep; //edge count
   nu=Rnu; //vertex count
   ds.Set(nu);
   edges = new GraphEdge [ep];
   edgeIniPN = new PrevNext [ep];
   edgeTerPN = new PrevNext [ep];
   vertIniHT = new HeadTail [nu];
   vertTerHT = new HeadTail [nu];
   checkPN = new PrevNext [nu];
   for (u32 k=0; k<ep; k++) {
      edgeIniPN[k].prev=-1; edgeIniPN[k].next=-1;
      edgeTerPN[k].prev=-1; edgeTerPN[k].next=-1;
      edges[k].ini=-1; edges[k].lab=-1; edges[k].ter=-1;
   }
   checkHT.head=-1; checkHT.tail=-1;
   for (u32 k=0; k<nu; k++) {
      checkPN[k].next=-1;   checkPN[k].prev=-1;
      vertIniHT[k].head=-1; vertIniHT[k].tail=-1;
      vertTerHT[k].head=-1; vertTerHT[k].tail=-1;
   }
}

Graph::Graph(u32 ep_, u32 nu_) {
   this->Initialize(ep_,nu_);
}

Graph::Graph(u32 mu, u32*S, u32*O) {
   this->Initialize(mu+mu,mu);
   u32 k=0;
   for(u32 n=0; n<mu; n++) {
      this->AddEdge(k,n,0,S[n]); k++;
      this->AddEdge(k,n,1,O[n]); k++;
   }
   assert(this->FoldedQ());
}


void Graph::ToGroup(std::vector<u32> &S, std::vector<u32> &O)
{
    u32 k, j, h, t, f;
    std::vector<u32> map, roots;
    u32* P;

    map.resize(nu + 1);
    for (j = 0; j <= nu; j++)
    {
        map[j] = 0;
    }

    k = 0; // pseudo index of resulting group
    for (j = 0; j < nu; j++)
    {
        f = ds.Find(j);
        if (map[f] == 0)
        {
            map[f] = ++k;
        }
        map[j] = map[f];

        if (j == f)
        {
            roots.push_back(j);
        }
    }

    assert(k > 0);

    S.resize(k + 1);
    O.resize(k + 1);
    for (j = 0; j <= k; j++)
    {
        S[j] = O[j] = 0;
    }

    for (j = 0; j < roots.size(); j++)
    {
        h = vertIniHT[roots[j]].head;
        t = vertIniHT[roots[j]].tail;
        if (h != t)
        {
            assert(edgeIniPN[h].next == t);
            assert(edgeIniPN[t].prev == h);
        }

        if (h != -1)
        {
            P = (edges[h].lab == 0) ? S.data() : O.data();
            P[map[edges[h].ini]] = map[edges[h].ter];
        }

        if (t != -1)
        {
            P = (edges[t].lab == 0) ? S.data() : O.data();
            P[map[edges[t].ini]] = map[edges[t].ter];
        }
    }

    return;
}


void Graph::Print() {
   printf("to check: ");
   PrintNodes(checkHT,checkPN);
   printf("\n");      
   printf("edges: ");
   for (u32 e=0; e<ep; e++) {
      if (edges[e].lab<LAB_CT) {
         assert(edges[e].ini!=-1); assert(edges[e].ter!=-1);
         printf("%d > %d > %d, ",edges[e].ini,/*'a'+*/edges[e].lab,edges[e].ter);
      }
   }
   printf("\n");
   for (u32 v=0; v<nu; v++) {
      if (ds.RootQ(v)) {
         printf("vert %d  ini: ",v);
         PrintNodes(vertIniHT[v],edgeIniPN);
         printf("  ter: ");
         PrintNodes(vertTerHT[v],edgeTerPN);
         printf("\n");
      }
   }
}

// Adds the edge k. Runs in O(LAB_CT) time.
void Graph::AddEdge(u32 k, u32 ini, u32 lab, u32 ter) {
   assert(edges[k].ini==-1 && edges[k].lab==-1 && edges[k].ter==-1);
   assert(ini<nu); assert(lab<LAB_CT); assert(ter<nu);
   edges[k].ini=ini; edges[k].lab=lab; edges[k].ter=ter;
   if (NodeFreeQ(ini,checkHT,checkPN)) {
      for (u32 e=vertIniHT[ini].head; e!=-1; e=edgeIniPN[e].next) {
         if (lab==edges[e].lab) {AppendNode(ini,checkHT,checkPN);}
      }
   }
   if (NodeFreeQ(ter,checkHT,checkPN)) {
      for (u32 e=vertTerHT[ter].head; e!=-1; e=edgeTerPN[e].next) {
         if (lab==edges[e].lab) {AppendNode(ter,checkHT,checkPN);}
      }
   }
   AppendNode(k,vertIniHT[ini],edgeIniPN);
   AppendNode(k,vertTerHT[ter],edgeTerPN);
}

// merge the two vertices v1 and v2. Runs in O(1) time.
// v1 and v2 don't have to be roots
// v2's edges are merged into v1's edges
void Graph::MergeVertices(u32 v1, u32 v2) {
   assert(v1!=-1); assert(v2!=-1);
   v1=ds.Find(v1); v2=ds.Find(v2);
   u32 v = ds.Merge(v1,v2);
   if (v!=v1) {
      JoinNodes(v,v1,vertIniHT,edgeIniPN);
      JoinNodes(v,v1,vertTerHT,edgeTerPN);
      if (NodeFreeQ(v,checkHT,checkPN)) {AppendNode(v,checkHT,checkPN);}
   }
   if (v!=v2) {
      JoinNodes(v,v2,vertIniHT,edgeIniPN);
      JoinNodes(v,v2,vertTerHT,edgeTerPN);
      if (NodeFreeQ(v,checkHT,checkPN)) {AppendNode(v,checkHT,checkPN);}
   }

}

// Check if the graph is folded.
// This could be replaced with a more robust check.
bool Graph::FoldedQ() {return checkHT.tail==-1;}

// Folds the graph. Runs in O(LAB_CT*ep*LogStar[ep]) time.
void Graph::Fold() {
   u32 e1, e2, e, v, u, w, l, i;
   u32 tab[LAB_CT];
   while ((v=checkHT.tail)!=-1) {
      //printf("loop start\n");
      //this->Print();getchar();
      DeleteNode(v,checkHT,checkPN);
      if (!ds.RootQ(v)) {continue;}
      for (i=0; i<LAB_CT; i++) {tab[i]=-1;}
      for (e=vertIniHT[v].head; e!=-1; e=edgeIniPN[e].next) {
         edges[e].ini=ds.Find(edges[e].ini); edges[e].ter=ds.Find(edges[e].ter);
         l=edges[e].lab; assert(l!=-1);
         if (tab[l]==-1) {tab[l]=e;}
         else {e1=tab[l]; e2=e; goto FoundIni;}
      }
      for (i=0; i<LAB_CT; i++) {tab[i]=-1;}
      for (e=vertTerHT[v].head; e!=-1; e=edgeTerPN[e].next) {
         edges[e].ini=ds.Find(edges[e].ini); edges[e].ter=ds.Find(edges[e].ter);
         l=edges[e].lab; assert(l!=-1);
         if (tab[l]==-1) {tab[l]=e;}
         else {e1=tab[l]; e2=e; goto FoundTer;}
      }
FoundNone:
      continue;
FoundIni:
      assert(v==edges[e1].ini); u=edges[e1].ter; w=edges[e2].ter;
      goto RemoveEdge;
FoundTer:
      assert(v==edges[e1].ter); u=edges[e1].ini; w=edges[e2].ini;
RemoveEdge:
      //printf("e1: %d  e2: %d\n",e1,e2);
      assert(ds.RootQ(v)); assert(ds.RootQ(u)); assert(ds.RootQ(w));
      DeleteNode(e2,vertIniHT[edges[e2].ini],edgeIniPN); edges[e2].ini=-1; 
      DeleteNode(e2,vertTerHT[edges[e2].ter],edgeTerPN); edges[e2].ter=-1; edges[e2].lab=-1;
      if (u!=w) {
         if (w==ds.Merge(u,w)) {std::swap(u,w);} //ensure u is the root
         JoinNodes(u,w,vertIniHT,edgeIniPN);
         JoinNodes(u,w,vertTerHT,edgeTerPN);
         if (NodeFreeQ(u,checkHT,checkPN)) {AppendNode(u,checkHT,checkPN);}
      }
      v=ds.Find(v);
      if (NodeFreeQ(v,checkHT,checkPN)) {AppendNode(v,checkHT,checkPN);}
   }
}


class Matrix {
public:
    xfmpz _11, _12, _21, _22;

    Matrix() : _11((ulong)(1)), _22((ulong)(1)) {};

    Matrix(slong a11, slong a12, slong a21, slong a22) : _11(a11), _12(a12), _21(a21), _22(a22) {};

    Matrix(Matrix&& other) noexcept : _11(other._11), _12(other._12), _21(other._21), _22(other._22) {};

    Matrix& operator=(Matrix&& other) noexcept
    {
        if (this != &other)
        {
            fmpz_swap(_11.data, other._11.data);
            fmpz_swap(_12.data, other._12.data);
            fmpz_swap(_21.data, other._21.data);
            fmpz_swap(_22.data, other._22.data);
            _fmpz_demote(other._11.data);
            _fmpz_demote(other._12.data);
            _fmpz_demote(other._21.data);
            _fmpz_demote(other._22.data);
        }  
        return *this;  
    }

    Matrix(const Matrix& other) : _11(other._11), _12(other._12), _21(other._21), _22(other._22) {};

    Matrix& operator=(const Matrix& other)
    {
        fmpz_set(_11.data, other._11.data);
        fmpz_set(_12.data, other._12.data);
        fmpz_set(_21.data, other._21.data);
        fmpz_set(_22.data, other._22.data);
        return *this;  
    }

    void zero()
    {
        fmpz_zero(_11.data);
        fmpz_zero(_12.data);
        fmpz_zero(_21.data);
        fmpz_zero(_22.data);
    };

    bool is_zero()
    {
        return fmpz_is_zero(_11.data)
            && fmpz_is_zero(_12.data)
            && fmpz_is_zero(_21.data)
            && fmpz_is_zero(_22.data);
    }

    void set(const Matrix &other)
    {
        fmpz_set(_11.data, other._11.data);
        fmpz_set(_12.data, other._12.data);
        fmpz_set(_21.data, other._21.data);
        fmpz_set(_22.data, other._22.data);
    };

    void set(xfmpz &a11, xfmpz &a12, xfmpz &a21, xfmpz &a22)
    {
        fmpz_set(_11.data, a11.data);
        fmpz_set(_12.data, a12.data);
        fmpz_set(_21.data, a21.data);
        fmpz_set(_22.data, a22.data);
    };

    void set(slong a11, slong a12, slong a21, slong a22)
    {
        fmpz_set_si(_11.data, a11);
        fmpz_set_si(_12.data, a12);
        fmpz_set_si(_21.data, a21);
        fmpz_set_si(_22.data, a22);
    };

    bool set_ex(er e);
    ex get_ex() const;
    ex get_ex_neg() const;

    void invert()
    {
        fmpz_swap(_11.data, _22.data);
        fmpz_neg(_12.data, _12.data);
        fmpz_neg(_21.data, _21.data);
    }

    // this = -S.this
    void lmul_mS()
    {
        fmpz_neg(_11.data, _11.data);
        fmpz_neg(_12.data, _12.data);
        fmpz_swap(_11.data, _21.data);
        fmpz_swap(_12.data, _22.data);
    };

    // this = -OO.this
    void lmul_mOO()
    {
        fmpz_sub(_11.data, _21.data, _11.data);
        fmpz_sub(_12.data, _22.data, _12.data);
        fmpz_swap(_11.data, _21.data);
        fmpz_swap(_12.data, _22.data);
    };

    // this = O.this
    void lmul_O()
    {
        fmpz_sub(_21.data, _11.data, _21.data);
        fmpz_sub(_22.data, _12.data, _22.data);
        fmpz_swap(_11.data, _21.data);
        fmpz_swap(_12.data, _22.data);
    };

    // this = this.m
    void RMulBy(Matrix m)
    {
        xfmpz a, b, c, d;
        fmpz_mul(a.data, _11.data, m._11.data); fmpz_addmul(a.data, _12.data, m._21.data);
        fmpz_mul(b.data, _11.data, m._12.data); fmpz_addmul(b.data, _12.data, m._22.data);
        fmpz_mul(c.data, _21.data, m._11.data); fmpz_addmul(c.data, _22.data, m._21.data);
        fmpz_mul(d.data, _21.data, m._12.data); fmpz_addmul(d.data, _22.data, m._22.data);
        fmpz_swap(_11.data, a.data);
        fmpz_swap(_12.data, b.data);
        fmpz_swap(_21.data, c.data);
        fmpz_swap(_22.data, d.data);
    };

    // this = inverse(m).this
    void LMulByInverse(Matrix m)
    {
        xfmpz a, b, c, d;
        fmpz_mul(a.data, _11.data, m._22.data); fmpz_submul(a.data, _21.data, m._12.data);
        fmpz_mul(b.data, _12.data, m._22.data); fmpz_submul(b.data, _22.data, m._12.data);
        fmpz_mul(c.data, _21.data, m._11.data); fmpz_submul(c.data, _11.data, m._21.data);
        fmpz_mul(d.data, _22.data, m._11.data); fmpz_submul(d.data, _12.data, m._21.data);
        fmpz_swap(_11.data, a.data);
        fmpz_swap(_12.data, b.data);
        fmpz_swap(_21.data, c.data);
        fmpz_swap(_22.data, d.data);
    };

    bool det_is_one()
    {
        xfmpz s, t;
        fmpz_mul(t.data, _11.data, _22.data);
        fmpz_mul(s.data, _12.data, _21.data);
        fmpz_add_ui(s.data, s.data, 1);
        return fmpz_equal(t.data, s.data);
    }

    void print() const;
};

int cmp_norm(const Matrix & a, const Matrix & b)
{
    xfmpz t, asum, bsum, amax, bmax;
    int cmp;

//std::cout << "cmping: "; a.print(); std::cout << std::endl;
//std::cout << "   and: "; b.print(); std::cout << std::endl;

    fmpz_abs(t.data, a._11.data);
    fmpz_add(asum.data, asum.data, t.data);
    fmpz_abs(t.data, a._12.data);
    fmpz_add(asum.data, asum.data, t.data);
    fmpz_abs(t.data, a._21.data);
    fmpz_add(asum.data, asum.data, t.data);
    fmpz_abs(t.data, a._22.data);
    fmpz_add(asum.data, asum.data, t.data);

    fmpz_abs(t.data, b._11.data);
    fmpz_add(bsum.data, bsum.data, t.data);
    fmpz_abs(t.data, b._12.data);
    fmpz_add(bsum.data, bsum.data, t.data);
    fmpz_abs(t.data, b._21.data);
    fmpz_add(bsum.data, bsum.data, t.data);
    fmpz_abs(t.data, b._22.data);
    fmpz_add(bsum.data, bsum.data, t.data);

    cmp = fmpz_cmpabs(asum.data, bsum.data);
    if (cmp != 0)
{
//std::cout << "0reting: " << cmp << std::endl;
        return cmp;
}

    if (fmpz_cmpabs(amax.data, a._11.data) < 0)
        fmpz_abs(amax.data, a._11.data);
    if (fmpz_cmpabs(amax.data, a._12.data) < 0)
        fmpz_abs(amax.data, a._12.data);
    if (fmpz_cmpabs(amax.data, a._21.data) < 0)
        fmpz_abs(amax.data, a._21.data);
    if (fmpz_cmpabs(amax.data, a._22.data) < 0)
        fmpz_abs(amax.data, a._22.data);

    if (fmpz_cmpabs(bmax.data, b._11.data) < 0)
        fmpz_abs(bmax.data, b._11.data);
    if (fmpz_cmpabs(bmax.data, b._12.data) < 0)
        fmpz_abs(bmax.data, b._12.data);
    if (fmpz_cmpabs(bmax.data, b._21.data) < 0)
        fmpz_abs(bmax.data, b._21.data);
    if (fmpz_cmpabs(bmax.data, b._22.data) < 0)
        fmpz_abs(bmax.data, b._22.data);

    cmp = fmpz_cmpabs(amax.data, bmax.data);
    if (cmp != 0)
{
//std::cout << "1reting: " << cmp << std::endl;
        return cmp;
}

    cmp = fmpz_cmpabs(a._11.data, b._11.data);
    if (cmp != 0)
{
//std::cout << "2reting: " << cmp << std::endl;
        return cmp;
}

    cmp = fmpz_cmpabs(a._12.data, b._12.data);
    if (cmp != 0)
{
//std::cout << "3reting: " << cmp << std::endl;
        return cmp;
}

    cmp = fmpz_cmpabs(a._21.data, b._21.data);
    if (cmp != 0)
{
//std::cout << "4reting: " << cmp << std::endl;
        return cmp;
}

    cmp = fmpz_cmpabs(a._22.data, b._22.data);
//std::cout << "5reting: " << cmp << std::endl;
    return cmp;
}


void Matrix::print() const
{
    printf("{{");
    fmpz_print(_11.data);
    printf(", ");
    fmpz_print(_12.data);
    printf("}, {");
    fmpz_print(_21.data);
    printf(", ");
    fmpz_print(_22.data);
    printf("}}");
}

ex Matrix::get_ex() const
{
    uex m11(emake_int_copy(_11));
    uex m12(emake_int_copy(_12));
    uex m21(emake_int_copy(_21));
    uex m22(emake_int_copy(_22));

    uex m1(emake_node(gs.sym_sList.copy(), m11.release(), m12.release()));
    ex m2 = emake_node(gs.sym_sList.copy(), m21.release(), m22.release());
    return emake_node(gs.sym_sList.copy(), m1.release(), m2);
}

ex Matrix::get_ex_neg() const
{
    xfmpz t11(_11), t12(_12), t21(_21), t22(_22);
    fmpz_neg(t11.data, t11.data);
    fmpz_neg(t12.data, t12.data);
    fmpz_neg(t21.data, t21.data);
    fmpz_neg(t22.data, t22.data);

    uex m11(emake_int_move(t11));
    uex m12(emake_int_move(t12));
    uex m21(emake_int_move(t21));
    uex m22(emake_int_move(t22));

    uex m1(emake_node(gs.sym_sList.copy(), m11.release(), m12.release()));
    ex m2 = emake_node(gs.sym_sList.copy(), m21.release(), m22.release());
    return emake_node(gs.sym_sList.copy(), m1.release(), m2);
}

bool Matrix::set_ex(er e)
{
    if (!ehas_head_sym_length(e, gs.sym_sList.get(), 2))
    {
        return false;
    }

    er e1 = echild(e,1);
    er e2 = echild(e,2);

    if (   !ehas_head_sym_length(e1, gs.sym_sList.get(), 2)
        || !ehas_head_sym_length(e2, gs.sym_sList.get(), 2))
    {
        return false;
    }

    if (   !eis_int(echild(e1,1))
        || !eis_int(echild(e1,2))
        || !eis_int(echild(e2,1))
        || !eis_int(echild(e2,2)))
    {
        return false;
    }

    set(eint_number(echild(e1,1)), eint_number(echild(e1,2)),
        eint_number(echild(e2,1)), eint_number(echild(e2,2)));

    return det_is_one();
}

class MatrixMemberFunction {
    er f;
public:

    MatrixMemberFunction(er f_) : f(f_) {};

    virtual bool operator() (const Matrix & m) const
    {
        ex mm = m.get_ex();
        ex e = emake_node(ecopy(f), mm);
        ex rr = eval(e);
        bool r = eis_sym(rr, gs.sym_sTrue.get());
        eclear(rr);
        if (r)
            return r;

        mm = m.get_ex_neg();
        e = emake_node(ecopy(f), mm);
        rr = eval(e);
        r = eis_sym(rr, gs.sym_sTrue.get());
        eclear(rr);
        return r;
    }
};


class Group
{
public:
	std::vector<u32> S, O;

    ex get_ex() const;
    ex get_index_ex() const;
    bool set_ex(er e);
    void from_memberq(const MatrixMemberFunction & f);
    bool from_generators_ex(er l);
    void get_cosets_generators(std::vector<Matrix> &c, std::vector<Matrix> &v);
    bool is_valid() const;
    bool standardize();
    void print();
    void printbasic();
    bool meet(const Group & a, const Group & b);
    bool join(const Group & a, const Group & b);
    u32 matrix_coset(const Matrix & mm);
    bool set_GammaN(mp_limb_t N);
    bool is_core() const;
    bool morphism_exists(const Group & h);
};


static bool ex_get_permutation(std::vector<u32> &p, er e)
{
    if (eis_parray(e))
    {
std::cout << "setting permutation from parray" << std::endl;

std::cout << "type: " << eparray_type(e) << std::endl;
std::cout << "rank: " << eparray_rank(e) << std::endl;

        if (eparray_type(e) != 0)
            return false;

        if (eparray_rank(e) != 1)
            return false;

        slong n = eparray_dimension(e, 0);
        slong * data = eparray_int_data(e);

std::cout << "dim0: " << n << std::endl;


        p.resize(n + 1);
        p[0] = 0;
        for (slong i = 0; i < n; i++)
        {
            p[i + 1] = data[i];
            if (data[i] < 0 || data[i] > n)
                return false;
        }

std::cout << "setting permutation success" << std::endl;

        return true;
    }

    if (!ehas_head_sym(e, gs.sym_sList.get()))
    {
        return false;
    }

    p.resize(1);
    p[0] = 0;

    for (size_t i = 1; i <= elength(e); i++)
    {
        er ei = echild(e,i);
        if (!eis_machine_int(ei))
        {
            return false;
        }

        slong j = eget_machine_int(ei);
        if (j < 0 || j > elength(e))
        {
            return false;
        }

        p.push_back(j);
    }

    return true;
}

bool Group::set_ex(er e)
{
    return ehas_head_sym_length(e, gs.sym_msGroup.get(), 2) 
        && ex_get_permutation(S, echild(e,1))
        && ex_get_permutation(O, echild(e,2))
        && is_valid();
}


ex Group::get_ex() const
{
/*
    uex s(gs.sym_sList.get(), S.size());
    for (size_t i = 1; i < S.size(); i++)
    {
        s.push_back(emake_int_ui(S[i]));
    }

    uex o(gs.sym_sList.get(), O.size());
    for (size_t i = 1; i < O.size(); i++)
    {
        o.push_back(emake_int_ui(O[i]));
    }
*/
    assert(S.size() > 1);
    assert(O.size() > 1);
    assert(S.size() == O.size());

    slong * data;

    uex s(emake_parray1(0, S.size() - 1));
    data = eparray_int_data(s.get());
    for (size_t i = 1; i < S.size(); i++)
    {
        data[i - 1] = S[i];
    }

    uex o(emake_parray1(0, O.size() - 1));
    data = eparray_int_data(o.get());
    for (size_t i = 1; i < O.size(); i++)
    {
        data[i - 1] = O[i];
    }

    ex r = emake_node(gs.sym_msGroup.copy(), s.release(), o.release());
    enode_statusflags(r) |= ESTATUSFLAG_VALIDQ;
    return r;
}

ex Group::get_index_ex() const
{
    for (size_t i = S.size(); i > 1; i--)
    {
        if (S[i - 1] == 0)
        {
            return gs.sym_sInfinity.copy();
        }
    }

    for (size_t i = O.size(); i > 1; i--)
    {
        if (O[i - 1] == 0)
        {
            return gs.sym_sInfinity.copy();
        }
    }

    return emake_int_ui(S.size() - 1);
}

void Group::print()
{
    u32 mu = S.size();
   if (mu==0) {printf("$BadGroup"); return;}
   u32*t = new u32[mu];
   u32*T = new u32[mu];
   pmMul(mu,T,O.data(),S.data());
   printf("Group[mu = %d, e2 = %d, e3 = %d, ei = %d,\n   S = ",
            mu, 
            pmFixedCount(mu,S.data()),
            pmFixedCount(mu,O.data()),
            pmCycleCount(mu,T,t));
   pmPrintCycles(mu,S.data());
   printf(",\n   O = ");
   pmPrintCycles(mu,O.data());
   printf(",\n   T = ");
   pmPrintCycles(mu,T);
   printf("]");
   delete [] t;
   delete [] T;
}

void print_vecu32(std::vector<u32>&v)
{
    std::cout << "{";
    for (size_t i = 0; i < v.size(); i++)
    {
        std::cout << v[i];
        if (i + 1 < v.size())
            std::cout << ", ";
    }
    std::cout << "}";
}

void Group::printbasic()
{
    std::cout << "Group[";
    print_vecu32(S);
    std::cout << ", ";
    print_vecu32(O);
    std::cout << "}]";
}

bool Group::is_valid() const
{
    u32 mup1 = S.size();

    if (mup1 < 2 || mup1 != O.size()
        || S[0] != 0 || O[0] != 0)
    {
        return false;
    }

    for (u32 i = 1; i < mup1; i++)
    {
        if (S[i] != 0 && S[S[i]] != i)
        {
            return false;
        }

        if (O[i] != 0 && O[O[O[i]]] != i)
        {
            return false;
        }
    }

    return true;
}


bool Group::is_core() const
{
    // check no free limbs

    assert(is_valid());

    for (u32 i = 1; i < S.size(); i++)
    {
        if (O[i] == 0)
        {
            if (i != 1)
            {
                return false;
            }
        }

        if (S[i] == 0)
        {
            u32 j = O[i];
            if (j == 0 || j == i)
            {
                if (i != 1)
                {
                    return false;
                }
                continue;
            }

            u32 k = O[j];
            assert(k != 0);

            if (S[k] == 0 && S[j] == 0)
            {
                return false;
            }
            else if (  (S[k] == 0 && S[j] != 0)
                    || (S[k] != 0 && S[j] == 0))
            {
                if (i != 1 && j != 1 && k != 1)
                {
                    return false;
                }
            }
        }
    }

    // check one orbit

    DisjointSet ds(S.size());

    for (u32 i = 1; i < S.size(); i++)
    {
        if (S[i] != 0)
        {
            ds.Merge(i, S[i]);
        }
        if (O[i] != 0)
        {
            ds.Merge(i, O[i]);
        }
    }

    for (u32 i = 1; i < S.size(); i++)
    {
        if (ds.Find(i) != ds.Find(1))
        {
            return false;
        }
    }

    return true;
}



bool Group::standardize()
{
    u32 i, j, k;
    std::vector<u32> map, Sn, On, queue;
    bool changed;

    assert(is_valid());

//std::cout << "before standardize" << std::endl;
//printbasic();
//std::cout << std::endl;


    u32 mu = S.size() - 1;
    assert(mu > 0);

    map.resize(mu + 1);

    for (i = 0; i <= mu; i++)
    {
        map[i] = 0;
        if (S[i] == 0 || O[i] == 0)
        {
            queue.push_back(i);
        }
    }

    while (!queue.empty())
    {
        i = queue.back(); queue.pop_back();
        if (O[i] == 0)
        {
            j = S[i];
            if (j != 0 && j != i)
            {
                if (i != 1 || O[j] == 0)
                {
                    S[i] = S[j] = 0;
                    queue.push_back(j);
                }
            }
        }
        if (S[i] == 0)
        {
            j = O[i];
            k = O[j];
            if (j != 0 && j != i)
            {
                assert(k != 0);
                assert(k != i);
                assert(k != j);

                if (S[j] == 0)
                {
                    if ((i != 1 && j != 1) || S[k] == 0)
                    {
                        O[i] = O[j] = O[k] = 0;
                        queue.push_back(k);
                    }
                }
                else if (S[k] == 0)
                {
                    if ((i != 1 && k != 1) || S[j] == 0)
                    {
                        O[i] = O[j] = O[k] = 0;
                        queue.push_back(j);
                    }
                }
            }
        }
    }

    queue.resize(1);
    queue[0] = 1;
    u32 new_mu = 0;
    
    while (!queue.empty())
    {
        i = queue.back(); queue.pop_back();
        assert(i != 0);
        if (map[i] == 0)
        {
            map[i] = ++new_mu;
            if (S[i] != 0)
                queue.push_back(S[i]);
            if (O[i] != 0)
                queue.push_back(O[i]);
        }
    }

    Sn.resize(new_mu + 1);
    On.resize(new_mu + 1);
    for (i = 0; i <= new_mu; i++)
    {
        Sn[i] = 0;
        On[i] = 0;
    }

    changed = (mu != new_mu);
    for (i = 0; i <= mu; i++)
    {
        Sn[map[i]] = map[S[i]];
        On[map[i]] = map[O[i]];
        changed = changed || Sn[map[i]] != S[map[i]];
        changed = changed || On[map[i]] != O[map[i]];
    }

    std::swap(S, Sn);
    std::swap(O, On);

//std::cout << "after standardize" << std::endl;
//printbasic();
//std::cout << std::endl;

    assert(is_core());

    return changed;
}


bool Group::morphism_exists(const Group & h)
{
    assert(is_core());
    assert(h.is_core());

    std::vector<u32> map(S.size(), 0);
    std::vector<u32> queue(1, 1);
    map[1] = 1;

    while (!queue.empty())
    {
        u32 i = queue.back(); queue.pop_back();
        assert(map[i] != 0);

        u32 j = S[i];
        if (j != 0)
        {
            if (map[j] == 0)
            {
                if (S[map[i]] == 0)
                {
                    return false;
                }
                else
                {
                    map[j] = S[map[i]];
                    queue.push_back(j);
                }
            }
            else
            {
                if (map[j] != S[map[i]])
                {
                    return false;
                }
            }
        }

        j = O[i];
        if (j != 0)
        {
            if (map[j] == 0)
            {
                if (O[map[i]] == 0)
                {
                    return false;
                }
                else
                {
                    map[j] = O[map[i]];
                    queue.push_back(j);
                }
            }
            else
            {
                if (map[j] != O[map[i]])
                {
                    return false;
                }
            }
        }
    }

    return true;
}

bool Group::join(const Group & a, const Group & b)
{
    assert(a.is_valid());
    assert(b.is_valid());

    size_t ep = 0;

    for (size_t i = 1; i < a.S.size(); i++)
        ep += (a.S[i] != 0) + (a.O[i] != 0);

    for (size_t i = 1; i < b.S.size(); i++)
        ep += (b.S[i] != 0) + (b.O[i] != 0);

    Graph g(ep, a.S.size() + b.S.size() - 3);

    size_t e = 0;

    for (size_t i = 1; i < a.S.size(); i++)
    {
#define MAP(j) ((j) == 1 ? 0 : (j) - 1)
        if (a.S[i] != 0)
            g.AddEdge(e++, MAP(i), 0, MAP(a.S[i]));
        if (a.O[i] != 0)
            g.AddEdge(e++, MAP(i), 1, MAP(a.O[i]));
#undef MAP
    }
    for (size_t i = 1; i < b.S.size(); i++)
    {
#define MAP(j) ((j) == 1 ? 0 : a.S.size() + (j) - 3)
        if (b.S[i] != 0)
            g.AddEdge(e++, MAP(i), 0, MAP(b.S[i]));
        if (b.O[i] != 0)
            g.AddEdge(e++, MAP(i), 1, MAP(b.O[i]));
#undef MAP
    }
    assert(e == ep);

    g.Fold();
    g.ToGroup(S, O);
    standardize();
    return true;
}


static uint64_t pack(uint32_t first, uint32_t second)
{
    return (uint64_t(first) << 32) + uint64_t(second);
}

static uint32_t unpack_first(uint64_t p)
{
    return (uint32_t)(p >> 32);
}

static uint32_t unpack_second(uint64_t p)
{
    return (uint32_t)(p);
}

bool Group::meet(const Group & a, const Group & b)
{
    std::map<uint64_t, u32> m;
    std::vector<uint64_t> queue;

    queue.resize(1);
    queue[0] = pack(1, 1);
    u32 new_mu = 0;

    while (!queue.empty())
    {
        uint64_t i = queue.back(); queue.pop_back();
        uint32_t i_first = unpack_first(i);
        uint32_t i_second = unpack_second(i);
        
        assert(i_first != 0);
        assert(i_second != 0);
        auto r = m.insert(std::pair<uint64_t, u32>(i, new_mu + 1));
        if (r.second)
        {
            new_mu++;
            if (a.S[i_first] != 0 && b.S[i_second] != 0)
                queue.push_back(pack(a.S[i_first], b.S[i_second]));
            if (a.O[i_first] != 0 && b.O[i_second] != 0)
                queue.push_back(pack(a.O[i_first], b.O[i_second]));
        }
    }

    S.resize(new_mu + 1);
    O.resize(new_mu + 1);
    for (uint32_t i = 0; i <= new_mu; i++)
    {
        S[i] = 0;
        O[i] = 0;
    }

    for (auto it = m.begin(); it != m.end(); it++)
    {
        S[it->second] = m[pack(a.S[unpack_first(it->first)], b.S[unpack_second(it->first)])];
        O[it->second] = m[pack(a.O[unpack_first(it->first)], b.O[unpack_second(it->first)])];
    }

    return true;
}


u32 apply_perm_pow(std::vector<std::vector<u32>> & p, const xfmpz & e, u32 r)
{
    mp_bitcnt_t bits = fmpz_bits(e.data);

    u32 n = p.back().size();

    while (p.size() < bits)
    {
        u32 * l = p.back().data();
        p.push_back(std::vector<u32>(n, 0));
        pmMul(n, p.back().data(), l, l);
    }

    for (mp_bitcnt_t i = 0; i < bits; i++)
    {
        if (fmpz_tstbit(e.data, i))
        {
            r = p[i][r];
        }
    }

    return r;
}

u32 Group::matrix_coset(const Matrix & mm)
{
    size_t mup1 = S.size();
    std::vector<std::vector<u32>> SOO_pow;
    std::vector<std::vector<u32>> SO_pow;
    int s11, s12, s21, s22;
    Matrix m(mm);
    m.invert();
    xfmpz n;
    u32 r = 1;
//    std::string w;

    SOO_pow.push_back(std::vector<u32>(mup1, 0));
    SO_pow.push_back(std::vector<u32>(mup1, 0));

    pmMul(mup1, SO_pow[0].data(), S.data(), O.data());
    pmMul(mup1, SOO_pow[0].data(), SO_pow[0].data(), O.data());

looper:

//printf("looper M: "); m.print(); std::cout << std::endl;

    s11 = fmpz_sgn(m._11.data);
    s12 = fmpz_sgn(m._12.data);
    s21 = fmpz_sgn(m._21.data);
    s22 = fmpz_sgn(m._22.data);

    if (r == 0)
    {
        return r;
    }

    if (s11 == 0)
    {
        fmpz_abs(m._22.data, m._22.data);
        if (s22 == 0)
        {
//            w = "S" + w;
//            std::cout << w << std::endl;
            return S[r];
        }
        if (s22 == s12)
        {
            fmpz_sub_ui(m._22.data, m._22.data, 1);
//            w = "O(SO)^" + m._22.tostring() + w;
//            std::cout << w << std::endl;
            return O[apply_perm_pow(SO_pow, m._22, r)];
        }
        else
        {
//            w = "(SOO)^" + m._22.tostring() + "S" + w;
//            std::cout << w << std::endl;
            return apply_perm_pow(SOO_pow, m._22, S[r]);
        }
    }
    else if (s12 == 0)
    {
        fmpz_abs(m._21.data, m._21.data);
        if (s21 == 0)
        {
//            std::cout << w << std::endl;
            return r;
        }
        if (s21 == s11)
        {
//            w = "(SO)^" + m._21.tostring() + w;
//            std::cout << w << std::endl;
            return apply_perm_pow(SO_pow, m._21, r);
        }
        else
        {
            fmpz_sub_ui(m._21.data, m._21.data, 1);
//            w = "OO(SOO)^" + m._21.tostring() + "S" + w;
//            std::cout << w << std::endl;
            return O[O[apply_perm_pow(SOO_pow, m._21, S[r])]];
        }
    }
    else if (s21 == 0)
    {
        fmpz_abs(m._12.data, m._12.data);
        if (s12 == 0)
        {
//            std::cout << w << std::endl;
            return r;
        }
        if (s12 == s11)
        {
//            w = "(SOO)^" + m._12.tostring() + w;
//            std::cout << w << std::endl;
            return apply_perm_pow(SOO_pow, m._12, r);
        }
        else
        {
            fmpz_sub_ui(m._12.data, m._12.data, 1);
//            w = "O(SO)^" + m._12.tostring() + "S" + w;
//            std::cout << w << std::endl;
            return O[apply_perm_pow(SO_pow, m._12, S[r])];
        }
    }
    else if (s22 == 0)
    {
        fmpz_abs(m._11.data, m._11.data);
        if (s11 == 0)
        {
//            w = "S" + w;
//            std::cout << w << std::endl;
            return r;
        }
        if (s11 == s12)
        {
//            w = "(SO)^" + m._11.tostring() + "S" + w;
//            std::cout << w << std::endl;
            return apply_perm_pow(SO_pow, m._11, S[r]);
        }
        else
        {
            fmpz_sub_ui(m._11.data, m._11.data, 1);
//            w = "OO(SOO)^" + m._11.tostring() + w;
//            std::cout << w << std::endl;
            return O[O[apply_perm_pow(SOO_pow, m._11, r)]];
        }
    }

    // no entry of m is zero now

    if (s11 != s21)
    {
        assert(s12 != s22);

        fmpz_swap(m._11.data, m._21.data);
        fmpz_swap(m._12.data, m._22.data);
        std::swap(s11, s21);
        std::swap(s12, s22);
//        w = "S" + w;
        r = S[r];
        if (s21 > 0)
        {
            fmpz_neg(m._11.data, m._11.data);
            fmpz_neg(m._12.data, m._12.data);
            s11 = -s11;
            s12 = -s12;
        }
        else
        {
            fmpz_neg(m._21.data, m._21.data);
            fmpz_neg(m._22.data, m._22.data);
            s21 = -s21;
            s22 = -s22;
        }
    }
    else if (s21 < 0)
    {
        fmpz_neg(m._11.data, m._11.data);
        fmpz_neg(m._12.data, m._12.data);
        s11 = -s11;
        s12 = -s12;
        fmpz_neg(m._21.data, m._21.data);
        fmpz_neg(m._22.data, m._22.data);
        s21 = -s21;
        s22 = -s22;
    }

    assert(s11 == fmpz_sgn(m._11.data));
    assert(s12 == fmpz_sgn(m._12.data));
    assert(s21 == fmpz_sgn(m._21.data));
    assert(s22 == fmpz_sgn(m._22.data));
    assert(s11 > 0);
    assert(s21 > 0);
    assert(s12 == s22);

    if (s12 > 0)
    {
        if (fmpz_cmp(m._12.data, m._22.data) >= 0)
        {
            fmpz_fdiv_q(n.data, m._12.data, m._22.data);
            assert(fmpz_sgn(n.data) > 0);
            goto use_soo_power;
        }
        else
        {
            fmpz_fdiv_q(n.data, m._21.data, m._11.data);
            assert(fmpz_sgn(n.data) > 0);
            goto use_so_power;
        }
    }
    else
    {
        if (fmpz_cmp(m._11.data, m._21.data) >= 0)
        {
            fmpz_fdiv_q(n.data, m._11.data, m._21.data);
            assert(fmpz_sgn(n.data) > 0);
            goto use_soo_power;
        }
        else
        {
            fmpz_fdiv_q(n.data, m._22.data, m._12.data);
            assert(fmpz_sgn(n.data) > 0);
            goto use_so_power;
        }
    }

use_so_power:
    fmpz_submul(m._21.data, n.data, m._11.data);
    fmpz_submul(m._22.data, n.data, m._12.data);
    fmpz_neg(m._22.data, m._22.data);
    fmpz_neg(m._21.data, m._21.data);
    fmpz_swap(m._12.data, m._22.data);
    fmpz_swap(m._11.data, m._21.data);
    fmpz_sub_ui(n.data, n.data, 1);
//    w = "O(SO)^" + n.tostring() + w;
    r = O[apply_perm_pow(SO_pow, n, r)];
    goto looper;

use_soo_power:
    fmpz_submul(m._12.data, n.data, m._22.data);
    fmpz_submul(m._11.data, n.data, m._21.data);
    fmpz_neg(m._12.data, m._12.data);
    fmpz_neg(m._11.data, m._11.data);
    fmpz_swap(m._12.data, m._22.data);
    fmpz_swap(m._11.data, m._21.data);
    fmpz_sub_ui(n.data, n.data, 1);
//    w = "OO(SOO)^" + n.tostring() + w;
    r = O[O[apply_perm_pow(SOO_pow, n, r)]];
    goto looper;

    assert(false);
    return -1;
}



void Group::get_cosets_generators(std::vector<Matrix> &coset, std::vector<Matrix> &gen)
{
    assert(is_valid());

    size_t mu = S.size() - 1;

    std::set<uint64_t> L;
    std::vector<uint32_t> Q;
    std::vector<uint32_t> wordlen;
    std::vector<bool> inL;

    coset.resize(mu + 1);
    inL.resize(mu + 1);
    wordlen.resize(mu + 1);
    for (size_t i = 0; i <= mu; i++)
    {
        coset[i].zero();
        wordlen[i] = -1;
        inL[i] = false;
    }

#define Linsert(i) assert(wordlen[i] != -1); L.insert(pack(wordlen[i], i)); inL[i] = true

    if (O[1] == 1 || O[1] == 0)
    {
        coset[1].set(1,0,0,1); wordlen[1] = 0;

        if (O[1] == 1)
        {
            gen.push_back(Matrix(1,-1,1,0));
        }

        if (S[1] == 1 || S[1] == 0)
        {
            if (S[1] == 1)
            {
                gen.push_back(Matrix(0,-1,1,0));
            }
            return;
        }

        if (O[S[1]] == 0)
        {
            return;
        }

        if (O[S[1]] == S[1])
        {
            assert(mu == 2);
            coset[2].set(0,-1,1,0); wordlen[2] = 1;
            gen.push_back(Matrix(0,-1,1,1));
            return;
        }

        assert(mu >= 4);

        coset[S[1]].set(0,-1,1,0);      wordlen[S[1]] = 1;
        coset[O[S[1]]].set(1,1,0,1);    wordlen[O[S[1]]] = 2;
        coset[O[O[S[1]]]].set(1,0,1,1); wordlen[O[O[S[1]]]] = 2;
        Linsert(O[S[1]]);
        Linsert(O[O[S[1]]]);
        Q.push_back(O[O[S[1]]]);
        Q.push_back(O[S[1]]);
    }
    else
    {
        coset[1].set(1,0,0,1);          wordlen[1] = 0;
        coset[O[1]].set(1,-1,1,0);      wordlen[O[1]] = 1;
        coset[O[O[1]]].set(0,-1,1,-1);  wordlen[O[O[1]]] = 1;
        Linsert(1);
        Linsert(O[1]);
        Linsert(O[O[1]]);
        Q.push_back(O[O[1]]);
        Q.push_back(O[1]);
        Q.push_back(1);
    }

looper:

    while (!Q.empty())
    {
        u32 i = Q.back(); Q.pop_back();
        u32 j = S[i];
        assert(O[i] != i && O[O[i]] != O[i]);

        if (!inL[i])
            continue;

        if (j == 0)
        {
            inL[i] = false;
        }
        else if (i == j)
        {
            gen.push_back(Matrix(0,-1,1,0));
            gen.back().RMulBy(coset[i]);
            gen.back().LMulByInverse(coset[i]);
            inL[i] = false;
        }
        else if (O[j] == 0)
        {
            inL[i] = false;
        }
        else if (O[j] == j)
        {
            coset[j].set(0,-1,1,0); wordlen[j] = wordlen[i] + 1;
            coset[j].RMulBy(coset[i]);
            gen.push_back(Matrix(1,-1,1,0));
            gen.back().RMulBy(coset[j]);
            gen.back().LMulByInverse(coset[j]);
            inL[i] = false;
        }
        else if (inL[j])
        {
            gen.push_back(Matrix(0,-1,1,0));
            gen.back().RMulBy(coset[i]);
            gen.back().LMulByInverse(coset[j]);
            inL[j] = 0;
            inL[i] = 0;
        }
    }

    while (!L.empty())
    {
        u32 i = unpack_second(*L.begin()); L.erase(L.begin());
        u32 j = S[i];
        if (!inL[i])
        {
            continue;
        }

        assert(i != 0);
        assert(j != 0);
        assert(O[j] != 0);
        assert(O[O[j]] != 0);
        assert(j != i);
        assert(O[j] != i);
        assert(O[O[j]] != i);
        assert(O[j] != j);
        assert(O[O[j]] != j);
        assert(O[O[j]] != O[j]);
        assert(!coset[i].is_zero());
        assert(coset[j].is_zero());
        assert(coset[O[j]].is_zero());
        assert(coset[O[O[j]]].is_zero());

        coset[j].set(coset[i]);             wordlen[j] = wordlen[i] + 1;
        coset[j].lmul_mS();
        coset[O[j]].set(coset[j]);          wordlen[O[j]] = wordlen[j] + 1;
        coset[O[j]].lmul_O();
        coset[O[O[j]]].set(coset[O[j]]);    wordlen[O[O[j]]] = wordlen[j] + 1;
        coset[O[O[j]]].lmul_O();

        Linsert(O[j]);
        Linsert(O[O[j]]);
        Q.push_back(O[O[j]]);
        Q.push_back(O[j]);

        goto looper;
    }

    for (size_t i = 1; i <= mu; i++)
    {
        assert(!coset[i].is_zero());
    }

    return;

#undef PACK
}

bool Group::from_generators_ex(er l)
{
    size_t nu = 1;
    std::vector<GraphEdge> sm;
    Matrix m;
    xfmpz a, b, a1, a2, b1, b2;

    for (size_t i = 1; i <= elength(l); i++)
    {
        if (!m.set_ex(echild(l,i)))
        {
            return false;
        }

        u32 toAp=0, sta=-1, end=0, AP_NONE=0, AP_S=1, AP_O=2, AP_OO=3, AP_DONE=4; 
        while (toAp!=AP_DONE)
        {
            if (fmpz_is_zero(m._21.data))
            {
                int sgn = fmpz_sgn(m._12.data);
                if (sgn == 0)
                {
                    toAp = AP_DONE;
                }
                else if ((sgn ^ fmpz_sgn(m._22.data)) >= 0)
                {
                    m.lmul_mOO();
                    toAp = AP_O;
                }
                else
                {
                    m.lmul_mS();
                    toAp = AP_S;
                }
            }
            else if (fmpz_is_zero(m._22.data))
            {
                int sgn = fmpz_sgn(m._11.data);
                if (sgn == 0)
                {
                    m.lmul_mS();
                    toAp = AP_S;
                }
                else if ((sgn ^ fmpz_sgn(m._21.data)) >= 0)
                {
                    m.lmul_mOO();
                    toAp = AP_O;
                }
                else
                {
                    m.lmul_mS();
                    toAp = AP_S;
                }
            }
            else
            {
                fmpz_set(a1.data, m._11.data);
                fmpz_set(a2.data, m._21.data);
                if (fmpz_sgn(m._21.data) <= 0)
                {
                    fmpz_neg(a1.data, a1.data);
                    fmpz_neg(a2.data, a2.data);
                }

                fmpz_set(b1.data, m._12.data);
                fmpz_set(b2.data, m._22.data);
                if (fmpz_sgn(m._22.data) <= 0)
                {
                    fmpz_neg(b1.data, b1.data);
                    fmpz_neg(b2.data, b2.data);
                }

                fmpz_add(a.data, a1.data, b1.data);
                fmpz_add(b.data, a2.data, b2.data);

                if (fmpz_sgn(a.data) < 0)
                {
                    m.lmul_mS();
                    toAp = AP_S;
                }
                else if (fmpz_cmp(a.data, b.data) < 0)
                {
                    m.lmul_O();
                    toAp = AP_OO;
                }
                else
                {
                    m.lmul_mOO();
                    toAp = AP_O;
                }
            }

            sta = end;
            if (fmpz_is_zero(m._12.data) && fmpz_is_zero(m._21.data))
            {
                end = 0;
            }
            else
            {
                end = nu++;
            }
            if (toAp == AP_S)
            {
                sm.push_back(GraphEdge(sta, 0, end));
                sm.push_back(GraphEdge(end, 0, sta));
            }
            else if (toAp == AP_OO)
            {
                sm.push_back(GraphEdge(sta, 1, end));
                if (sta != end)
                {
                    sm.push_back(GraphEdge(end, 1, nu));
                    sm.push_back(GraphEdge(nu, 1, sta));
                    nu++;
                }
            }
            else if (toAp == AP_O)
            {
                sm.push_back(GraphEdge(end, 1, sta));
                if (sta != end)
                {
                    sm.push_back(GraphEdge(nu, 1, end));
                    sm.push_back(GraphEdge(sta, 1, nu));
                    nu++;
                }
            }
        }
    }

    size_t ep = sm.size();
    if (ep == 0)
    {
        S.resize(2);
        S[0] = S[1] = 0;
        O.resize(2);
        O[0] = O[1] = 0;
        return true;
    }

    Graph graph(ep, nu);
    for (size_t i = 0; i < ep; i++)
    {
        graph.AddEdge(i, sm[i].ini, sm[i].lab, sm[i].ter);
    }

    graph.Fold();
    graph.ToGroup(S, O);
    standardize();
    return true;
}


// given a function f for testing if a given matrix is in the group,
//  construct the representation of the group
// If the index gets too large or the function f is bad, the function fails
void Group::from_memberq(const MatrixMemberFunction & f)
{
    Matrix m, mi;
    std::vector <u32> li;
    std::vector <Matrix> lm;
    size_t si, ei, i, j, p, q, k;

    S.clear();
    O.clear();
    S.push_back(0);
    O.push_back(0);
    lm.push_back(Matrix(0,0,0,0)); li.push_back(0);

    if (f(Matrix(1,-1,1,0)))
    {
        if (f(Matrix(0,-1,1,0)))
        {
            S.push_back(1);
            O.push_back(1);
            return;
        }
        if (f(Matrix(0,-1,1,1)))
        {
            S.push_back(2); S.push_back(1);
            O.push_back(1); O.push_back(2);
            return;
        }

        S.push_back(2); S.push_back(1); S.push_back(0); S.push_back(0);
        O.push_back(1); O.push_back(3); O.push_back(4); O.push_back(2);
        lm.push_back(Matrix(1,1,0,1)); li.push_back(3);
        lm.push_back(Matrix(1,0,1,1)); li.push_back(4);
        si = 1; ei = 2; k = 4;
    }
    else
    {
        S.push_back(0); S.push_back(0); S.push_back(0);
        O.push_back(2); O.push_back(3); O.push_back(1);
        lm.push_back(Matrix(1,0,0,1)); li.push_back(1);
        lm.push_back(Matrix(1,-1,1,0)); li.push_back(2);
        lm.push_back(Matrix(0,-1,1,-1)); li.push_back(3);
        si = 1; ei = 3; k = 3;
    }

Loop:
/*
printf("\nlooop**********\n");
std::cout << "s: "; print_vecu32(S); std::cout<<std::endl;
std::cout << "o: "; print_vecu32(O); std::cout<<std::endl;

SleepMS(1000);

    printf("list:   si: %d  ei: %d \n", si, ei);
    for (i = 1; i <= ei; i++)
    {
        printf("  %d ",li[i]);
        lm[i].print();
        printf("\n");
    }
*/

    for (i = si; i <= ei; i++)
    {
        p = li[i];

        m.set(0,-1,1,0); m.RMulBy(lm[i]); m.LMulByInverse(lm[i]);
        if (f(m))
        {
            assert(S[p] == 0); S[p] = p;
            lm[i] = lm[ei]; li[i] = li[ei];
            ei--; i--; continue;
        }

        m.set(0,-1,1,1); m.RMulBy(lm[i]); m.LMulByInverse(lm[i]);
        if (f(m))
        {
            k++;
            assert(S[p] == 0); S[p] = k; S.push_back(p); O.push_back(k);
            lm[i] = lm[ei]; li[i] = li[ei];
            ei--; i--; continue;
        }

        for (j = 1; j < i; j++)
        {
            q = li[j];
            m.set(0,-1,1,0); m.RMulBy(lm[j]); m.LMulByInverse(lm[i]);
            if (f(m))
            {
                assert(S[p] == 0); assert(S[q] == 0); S[p] = q; S[q] = p;
                lm[j] =lm[i-1]; lm[i-1] = lm[ei]; lm[i] = lm[ei-1];
                li[j] =li[i-1]; li[i-1] = li[ei]; li[i] = li[ei-1];
                i -= 2; ei -= 2; break;
            }
        }
    }

//std::cout << "k: " << k << std::endl;

    if (ei == 0) {goto Done;}
    if (k > 1000) {printf("done from k > 1000\n");goto Failed;}

    // find index i of simplist matrix

	i = 1;
    for (j=1; j<=ei; j++)
    {

//        int cmp = cmp_norm(lm[i], lm[j]);

//std::cout << "cmp(" << i <<", " << j << ") = " << cmp << std::endl;


        if (cmp_norm(lm[i], lm[j]) > 0)
        {
            i = j;
        }
    }

	mi = lm[i];
    p = li[i];

    O.push_back(k + 2); O.push_back(k + 3); O.push_back(k + 1);
    assert(S[p] == 0);
    S[p] = k + 1;
    S.push_back(p); S.push_back(0); S.push_back(0);
	k = k + 3;

	si = ei;
	lm[i] = lm[ei]; li[i] = li[ei];
    m.set(1,1,0,1); m.RMulBy(mi);
    li[ei] = O[S[p]]; lm[ei] = m;
	ei++;
    m.set(1,0,1,1); m.RMulBy(mi);

    if (ei >= lm.size())
    {
        assert(ei == lm.size());
        assert(ei == li.size());
        li.push_back(O[O[S[p]]]);
        lm.push_back(m);
    }
    else
    {
        li[ei] = O[O[S[p]]];
        lm[ei] = m;
    }
    goto Loop;
   
Done:
    assert(k == S.size()-1);
    assert(k == O.size()-1);
    return;

Failed:
    assert(false);
    S.clear();
    O.clear();
    return;
}




class nmodMatrix {
    mp_limb_t _11, _12, _21, _22;

    nmodMatrix(mp_limb_t a11, mp_limb_t a12, mp_limb_t a21, mp_limb_t a22) : _11(a11), _12(a12), _21(a21), _22(a22) {};

};



bool Group::set_GammaN(mp_limb_t N)
{
    return false;
/*
    u32 i, j, k;
    std::vector<u32> map, Sn, On, queue;
    bool changed;
    nmod_t mod;
    nmod_init(mod, N);

    std::vector<std::pair<nmodMatrix, u32>> map;

    for (mp_limb_t b = 0; b < N; b++)
    {
    for (mp_limb_t c = 0; b < N; c++)
    {
    for (mp_limb_t d = 0; b < N; d++)
    {
        for (mp_limb_t a = 0; a < N; a++)
        {
            if (1 == nmod_sub(nmod_mul(a, d, mod), nmod_mul(b, c, mod), mod))
            {
                map.push_back(std::pair<nmodMatrix, u32>(nmodMatrix(a,b,c,d), 0));
            }
        }
    }}}

std::cout << "map.size: " << map.size() << std::endl;

    u32 new_mu = 0;
    
    while (!queue.empty())
    {
        i = queue.back(); queue.pop_back();
        assert(i != 0);
        if (map[i] == 0)
        {
            map[i] = ++new_mu;
            queue.push_back(S[i]);
            queue.push_back(O[i]);
        }
    }


    S.resize(mu + 1);
    O.resize(mu + 1);
    for (i = 0; i <= mu; i++)
    {
        Sn[i] = 0;
        On[i] = 0;
    }




    changed = (mu != new_mu);
    for (i = 0; i <= mu; i++)
    {
        Sn[map[i]] = map[S[i]];
        On[map[i]] = map[O[i]];
        changed = changed || Sn[map[i]] != S[map[i]];
        changed = changed || On[map[i]] != O[map[i]];
    }

    std::swap(S, Sn);
    std::swap(O, On);
    return changed;
*/
}



/******************************************************************************/
/*
g = ModularSubgroups`Group[{1,2,3},{3,1,2}]
{Developer`PackedArrayQ[g[[1]]], Developer`PackedArrayQ[g]}

*/
ex dcode_msGroup(er e)
{
//std::cout << "dcode_msGroup: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_msGroup.get()));

    if (elength(e) != 2)
    {
//std::cout << "Group bad args" << std::endl;
        return ecopy(e);
    }

    if (espec_statusflags(e) & ESTATUSFLAG_VALIDQ)
    {
//std::cout << "Group valid" << std::endl;
        return ecopy(e);
    }

    Group g;

    if (!g.set_ex(e))
    {
//std::cout << "Group failed" << std::endl;
        return gs.sym_s$Failed.copy();
    }

    if (g.standardize())
    {
        ex r = g.get_ex();
//std::cout << "Group returning: " << ex_tostring_full(etor(r)) << std::endl;
        return r;
    }
    else
    {
//std::cout << "Group unchanged" << std::endl;
        return ecopy(e);
    }
}


ex dcode_msIndex(er e)
{
//std::cout << "dcode_msIndex: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_msIndex.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    Group g;

    if (!g.set_ex(echild(e,1)))
    {
        return ecopy(e);
    }

    g.standardize();

    return g.get_index_ex();
}


ex dcode_msValidGroupQ(er e)
{
//std::cout << "dcode_msValidGroupQ: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_msValidGroupQ.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    Group g;
    return emake_boole(g.set_ex(echild(e,1)));
}

ex dcode_msGenerators(er e)
{
    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    Group g;

    if (!g.set_ex(echild(e,1)))
    {
        return ecopy(e);
    }

    g.standardize();

    std::vector<Matrix> c, v;
    g.get_cosets_generators(c, v);

    uex r(gs.sym_sList.get(), v.size());
    for (size_t i = 0; i < v.size(); i++)
    {
        r.push_back(v[i].get_ex());
    }

    return r.release();
}

ex dcode_msLessEqual(er e)
{
    if (elength(e) != 2)
    {
        return _handle_message_argx1(e);
    }

    Group g, h;

    if (   !g.set_ex(echild(e,1))
        || !h.set_ex(echild(e,2)))
    {
        return ecopy(e);
    }

    return emake_boole(g.morphism_exists(h));
}

ex dcode_msJoin(er e)
{
    if (elength(e) != 2)
    {
        return _handle_message_argx2(e);
    }

    Group g, a, b;

    if (!a.set_ex(echild(e,1)) || !b.set_ex(echild(e,2)))
    {
        return ecopy(e);
    }

    if (g.join(a, b))
    {
        return g.get_ex();
    }
    else
    {
        return ecopy(e);
    }
}

ex dcode_msMeet(er e)
{
    if (elength(e) != 2)
    {
        return _handle_message_argx2(e);
    }

    Group g, a, b;

    if (!a.set_ex(echild(e,1)) || !b.set_ex(echild(e,2)))
    {
        return ecopy(e);
    }

    if (g.meet(a, b))
    {
        return g.get_ex();
    }
    else
    {
        return ecopy(e);
    }
}


ex dcode_msFromMemberQ(er e)
{
    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    Group g;
    MatrixMemberFunction f(echild(e,1));

    g.from_memberq(f);

    return g.get_ex();
}


ex dcode_msMemberQ(er e)
{
//std::cout << "dcode_msMemberQ: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_msMemberQ.get()));

    if (elength(e) != 2)
    {
        return _handle_message_argx2(e);
    }

    Group g;
    Matrix m;

    if (!g.set_ex(echild(e,1)))
    {
//std::cout << "get g failed" << std::endl;
        return ecopy(e);
    }
    
    if (!m.set_ex(echild(e,2)))
    {
//std::cout << "get m failed" << std::endl;
        return ecopy(e);
    }

//std::cout << "returning coset " << g.matrix_coset(m) << std::endl;

    return emake_boole(1 == g.matrix_coset(m));
}


ex dcode_msFromGenerators(er e)
{
//std::cout << "dcode_msFromGenerators: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_msFromGenerators.get()));

    if (elength(e) != 1)
    {
        return _handle_message_argx1(e);
    }

    Group g;

    if (!g.from_generators_ex(echild(e,1)))
    {
        return ecopy(e);
    }

    return g.get_ex();
}
