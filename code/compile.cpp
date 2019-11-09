#include<vector>
#include<list>
#include<map>
#include<set>
#include<cfloat>
#include<iomanip>
#include<sstream>

#include "ex.h"
#include "uex.h"
#include "code.h"
#include "digits.h"
#include "eval.h"
#include "ex_print.h"


/* bad legacy api */

typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef double f64;

i64 expr_getn(ex e) {return elength(e) + 1;}
bool is_atom(ex e) {return !eis_node(e);}
u64 sym_getc(ex e) {return (u64)(e);}

i64 expr_getn(er e) {return elength(e) + 1;}
bool is_atom(er e) {return !eis_node(e);}
u64 sym_getc(er e) {return (u64)(e);}


/*
char* int_print(char* s, u64 x) {
  u64 r = x%10;
  u64 q = x/10;
  if (q!=0) {s = int_print(s, q);}
  *s++ = r + '0';
  return s; 
}

char* int_print(char* s, i64 x) {
 if (x<0) {*s++='-';x=-x;}
 return int_print(s,(u64)x);
}
*/
char* print_sym(char*s, er sym) {
  const char* p = estr_string(eto_sym(sym)->name).c_str();
  while ((*s++ = *p++)) {}
  return --s;
}

typedef struct {
  u64 junk;
  i64 morejunk;
} SymTab;

SymTab gst;

const char * symtab_getstring(SymTab*tab, u64 code) {
    return estr_string(eto_sym((ex)(code))->name).c_str();
}

/* compiler */

#define REGT_VOID 0
#define REGT_BOL 1
#define REGT_INT 2
#define REGT_FLT 3
#define REGT_CPX 4

struct Register {
  u64 type;
  u32 num, var;
  Register() {};
  Register(u64 _type, u32 _num, u32 _var) : type(_type), num(_num), var(_var) {};
  bool operator<( const Register & other ) const {
    return this->type != other.type ? this->type < other.type :
           this->num != other.num   ? this->num  < other.num  :
                                      this->var  < other.var  ;
  }
};

bool operator !=(const Register& a, const Register& b) {
  return a.type != b.type || a.num != b.num || a.var != b.var;
}
bool operator ==(const Register& a, const Register& b) {
  return a.type == b.type && a.num == b.num && a.var == b.var;
}


u32 rtype_to_bsize(u64 type)
{
    u64 hi = type>>32;
    u64 lo = type&0x0FFFFFFFFULL;
    if (hi==REGT_INT) {return 8/*INT_LIMBS+8*lo*/;}
    if (hi==REGT_FLT) {return 8/*FLT_LIMBS+8*lo*/;}
    if (hi==REGT_BOL) {return 8;}
    assert(false);
    return 0;
}

void copy_regdata(unsigned char* b, u64 type, er e)
{
    u64 hi = type>>32;
    u64 lo = type&0x0FFFFFFFFULL;
    if (hi == REGT_BOL && eis_sym(e, gs.sym_sTrue.get()))
    {
        *(i64*)(b) = -1;
    }
    else if (hi == REGT_BOL && eis_sym(e, gs.sym_sFalse.get()))
    {
        *(i64*)(b) = 0;
    }
    else if (hi == REGT_INT && eis_int(e) && fmpz_fits_si(eint_data(e)))
    {
        *(i64*)(b) = fmpz_get_si(eint_data(e));
    }
    else if (hi == REGT_FLT && eis_double(e))
    {
        *(f64*)(b) = edouble_number(e);
    }
    else
    {
        assert(false);
    }
}


// mp integer
// mp flt
u64 expr_to_rtype(er e)
{
    if (eis_int(e))
    {
        return /*abs(int_getn(e))*/0 + (u64(REGT_INT)<<32);
    }
    else if (eis_double(e))
    {
        return /*abs(flt_getn(e))*/0 + (u64(REGT_FLT)<<32);
    }
    else
    {
        return 0;
    }
}

u64 exprtype_to_rtype(er e)
{
    if (eis_sym(e))
    {
        if (e == gs.sym_sInteger.get()) {return (u64(REGT_INT)<<32);}
        if (e == gs.sym_sReal.get()) {return (u64(REGT_FLT)<<32);}
        return 0;
    }
/*
    no multiprecision in VM for now
    
    else if (has_head_sym(e,CODE_Integer))
    {
        if (expr_getn(e)==2 && is_int(part(e,1)) && int_is_machine(part(e,1)))
        {
            i64 n = int_get_machine(part(e,1));
            if (n<0) {return 0;}
            if (n>99999) {return 0;}
            return n+(u64(REGT_INT)<<32);
        }
        return 0;
    }
    else if (has_head_sym(e,CODE_Real))
    {
        if (expr_getn(e)==2 && is_int(part(e,1)) && int_is_machine(part(e,1)))
        {
            i64 n = int_get_machine(part(e,1));
            if (n<0) {return 0;}
            if (n>99999) {return 0;}
            return n+(u64(REGT_FLT)<<32);
        }
        return 0;
    }
*/
    return 0;
}


char* print_rtype(char*s, u64 type)
{
    u64 hi = type>>32;
    u64 lo = type&0x0FFFFFFFFULL;
    if (hi==REGT_INT)
    {
        s=int_print(s,lo); *s++='i';
    }
    else if (hi==REGT_FLT)
    {
        s=int_print(s,lo); *s++='f';
    }
    else if (hi==REGT_BOL)
    {
        *s++='b';
    }
    else
    {
        *s++='?';
    }
    return s;
}

void printout_rtype(u64 type) {
  char buffer[32], *s;
  s = print_rtype(buffer,type);
  *s='\0';
  printf(buffer);
}


struct KData {
  std::vector<Register> kregl;
  ex kexprl;
};

u32 compile_get_kreg(KData&kdata, er e) {
  u64 n=kdata.kregl.size();
  assert(expr_getn(kdata.kexprl)==1+n);
  for (u64 i=0; i<n; i++) {
    if( ex_same(echild(kdata.kexprl,1+i),e)) {
      return i;
    }
  }
  kdata.kregl.push_back(Register(expr_to_rtype(e),n,-1));
  kdata.kexprl = eappend(kdata.kexprl, ecopy(e));
  return n;
}

#define PNODET_NOP 0
#define PNODET_BEG 1
#define PNODET_RET 2
#define PNODET_MOV 3
#define PNODET_JMP 4
#define PNODET_JT  5
#define PNODET_JF  6
#define PNODET_FXN 7
#define PNODET_PHI 8

struct PNode {
  std::vector<Register> def;
  std::vector<Register> use;
  std::vector<u32> succ;
  std::vector<u32> pred;
  std::vector<u32> df;
  std::vector<u32> phi;
  er fxn;
  u32 type;
  PNode() {};
  PNode(u32 _type) : type(_type), fxn(0) {};
  PNode(u32 _type, er _fxn) : type(_type), fxn(_fxn) {};
};

struct PhiNode {
  Register def;
  std::vector<Register> use;
  u32 loc;
  PhiNode() {};
  PhiNode(u32 _loc) : loc(_loc) {};
};



struct CProgram {
  std::vector<PNode> node;
  std::vector<PhiNode> phinode;
  std::vector<u32> rpo;
  std::vector<u32> rpoi;
  std::vector<u32> idom;
  std::vector<std::vector<u32> > idomc;
  std::map<Register,u32> rmap;
  std::vector<Register> rmapi;
  std::vector<std::vector<u32> > rdef;
  std::vector<std::vector<u32> > ruse;
  KData kdata;
};



char* vPrint(char*s, const std::vector<u32> &v) {
  *s++='{';
  if (0==v.size()) {s++;}
  for (u64 i=0; i<v.size(); i++) {
    s = int_print(s,u64(v.at(i)));  
    *s++=',';
  }
  *(s-1)='}';
  return s;
}
void vPrintOut(const std::vector<u32> &v) {
  char buffer[512], *os;
  os = vPrint(buffer,v); *os='\0';
  printf(buffer);
}



char* rPrint(char*s, const Register &reg) {

  if (reg.var==-1) { *s++='K';}
  s = print_rtype(s, reg.type);
  if (reg.num==-1) {*s++='?';return s;}
  s = int_print(s,u64(reg.num));
  if (reg.var!=0&&reg.var!=-1) {
    *s++='.';
    s = int_print(s,u64(reg.var));
  };
  return s;
}

void rPrintOut(const Register &reg) {
  char buffer[32], *os;
  os = rPrint(buffer,reg); *os='\0';
  printf(buffer);
}

char* vPrint(char*s, std::vector<Register> &v) {
  *s++='{';
  if (0==v.size()) {s++;}
  for (u64 i=0; i<v.size(); i++) {
    s = rPrint(s,v.at(i));  
    *s++=',';
  }
  *(s-1)='}';
  return s;
}


char* pnPrint(char*s, PNode &node) {
  
  char*si=s;

  do {*s++=' ';} while (s-si<1);
  switch (node.type) {
    case PNODET_NOP: strcpy(s,"NOP"); s+=3; break;
    case PNODET_BEG: strcpy(s,"BEG"); s+=3; break;
    case PNODET_RET: strcpy(s,"RET"); s+=3; break;
    case PNODET_MOV: strcpy(s,"MOV"); s+=3; break;
    case PNODET_JMP: strcpy(s,"JMP"); s+=3; break;
    case PNODET_JT : strcpy(s,"JT "); s+=3; break;
    case PNODET_JF : strcpy(s,"JF "); s+=3; break;
    case PNODET_FXN: strcpy(s,"FXN"); s+=3; break;
    case PNODET_PHI: strcpy(s,"PHI"); s+=3; break;
    default: assert(false);
  }
  *s++=' ';
  if (node.type==PNODET_FXN) {
    s = print_sym(s,node.fxn);
  } else {
    //s = int_print(s,node.fxn);
  }

  do {*s++=' ';} while (s-si<13);
  s = vPrint(s, node.def);

  do {*s++=' ';} while (s-si<25);
  s = vPrint(s, node.use);

  do {*s++=' ';} while (s-si<38);
  s = vPrint(s, node.succ);

  do {*s++=' ';} while (s-si<46);
  s = vPrint(s, node.pred);

  do {*s++=' ';} while (s-si<56);
  s = vPrint(s, node.df);

  do {*s++=' ';} while (s-si<66);
  s = vPrint(s, node.phi);
    return s;
}

void pnPrintOut(PNode &node) {
  char buffer[512], *os;
  os = pnPrint(buffer,node); *os='\0';
  puts(buffer);
}


char* pnPrint(char*s, PhiNode &node) {
  
  char*si=s;

  do {*s++=' ';} while (s-si<1);
  strcpy(s,"PHI"); s+=3; 
  *s++=' ';
  if (node.loc==-1) {
    strcpy(s,"NONE"); s+=4;
  } else {
    s = int_print(s,(u64)(node.loc));
  }
  
  do {*s++=' ';} while (s-si<13);
  s = rPrint(s, node.def);

  do {*s++=' ';} while (s-si<25);
  s = vPrint(s, node.use);
  return s;
}

void pnPrintOut(PhiNode &node) {
  char buffer[512], *os;
  os = pnPrint(buffer,node); *os='\0';
  puts(buffer);
}


void cpPrintOut(CProgram &prog) {
  char buffer[512], *os;

  puts("constants:");
  for (u64 i=0; i<prog.kdata.kregl.size(); i++) {
    printf("  ");
    rPrintOut(prog.kdata.kregl.at(i));
    printf(" = ");
    std::cout << ex_tostring_full(echild(prog.kdata.kexprl,1+i));
  }
  printf("\n");

  puts("nodes:");
  for (u64 i=0; i<prog.node.size(); i++) {
    os=int_print(buffer,i);
    while (os-buffer<4) {*os++=' ';} *os='\0';
    printf("%s",buffer);
    pnPrintOut(prog.node.at(i));
  }
  
  puts("phi nodes:");
  for (u64 i=0; i<prog.phinode.size(); i++) {
    os=int_print(buffer,i);
    while (os-buffer<4) {*os++=' ';} *os='\0';
    printf("%s",buffer);
    pnPrintOut(prog.phinode.at(i));
  }
  
  printf("rpo: ");
  os=vPrint(buffer,prog.rpo); *os='\0';
  puts(buffer);
  
  printf("rpoi: ");
  os=vPrint(buffer,prog.rpoi); *os='\0';
  puts(buffer);

  printf("idom and idomc: {");
  for (u64 i=0; i<prog.idom.size(); i++){
    if (i!=0) {printf(", ");}
    printf("%d",prog.idom.at(i));
    vPrintOut(prog.idomc.at(i)); *os='\0';
  }
  puts("}");
  
  printf("rmap: {");
  for (std::map<Register,u32>::iterator it=prog.rmap.begin(); it!=prog.rmap.end(); ++it){
    if (it!=prog.rmap.begin()) {printf(", ");}
    rPrintOut(it->first);
    printf(" -> %d",it->second);
  }
  puts("}");
  
  printf("rmapi and rdef, ruse: {");
  for (u64 i=0; i<prog.rmap.size(); i++){
    if (i!=0) {printf(", ");}
    rPrintOut(prog.rmapi.at(i));
    vPrintOut(prog.rdef.at(i));vPrintOut(prog.ruse.at(i));
  }
  puts("}");

}








void compile_findvar(Register&x, u64 var, std::vector<u64>&vl) {
//  printf("compile_findvar vl.size: %d \n", vl.size());
  for (u64 i=0; i<vl.size(); i+=3) {
    if (var==vl.at(i+0)) {
      x.type=vl.at(i+1);
      x.num =vl.at(i+2);
      x.var =0;
      return;
    }
  }
  assert(false);

}




u64 expr_compilelist_Plus_helper(std::list<PNode>&prog, std::vector<u64>&RETL,i64 spi, i64 ia, i64 ib) {
//printf("plus helper  ia=%d  ib=%d \n",ia,ib);
  assert(ib>ia);
  assert(ia>=0);
  if (ib-ia==1) {return RETL.at(ia);}
  i64 mid=(ia+ib)/2;
  u64 tA=expr_compilelist_Plus_helper(prog,RETL,spi,ia,mid);
  u64 tB=expr_compilelist_Plus_helper(prog,RETL,spi,mid,ib);
  u64 tC = std::max(tA,tB);
  PNode n(PNODET_FXN, gs.sym_sPlus.get());
  n.use.push_back(Register(tA,spi+ia,0));
  n.use.push_back(Register(tB,spi+mid,0));
  n.def.push_back(Register(tC,spi+ia,0));
  prog.push_back(n);
  return tC;
}

u64 expr_compilelist_Times_helper(std::list<PNode>&prog, std::vector<u64>&RETL,i64 spi, i64 ia, i64 ib) {
//printf("plus helper  ia=%d  ib=%d \n",ia,ib);
  assert(ib>ia);
  assert(ia>=0);
  if (ib-ia==1) {return RETL.at(ia);}
  i64 mid=(ia+ib)/2;
  u64 tA=expr_compilelist_Times_helper(prog,RETL,spi,ia,mid);
  u64 tB=expr_compilelist_Times_helper(prog,RETL,spi,mid,ib);
  u64 tC = std::max(tA,tB);
  PNode n(PNODET_FXN, gs.sym_sTimes.get());
  n.use.push_back(Register(tA,spi+ia,0));
  n.use.push_back(Register(tB,spi+mid,0));
  n.def.push_back(Register(tC,spi+ia,0));
  prog.push_back(n);
  return tC;
}



i64 expr_compilelist(std::list<PNode>&prog, std::vector<u64>&retl, std::vector<u64> &varl, KData&kdata, er e, i64 spi)
{
    i64 sp=spi;
    i64 retc;

    printf("  **** \n");
    printf("> expr_compilelist e:        "); std::cout << ex_tostring_full(e); printf("\n");
    printf("> expr_compilelist prog:     \n"); for (std::list<PNode>::iterator i = prog.begin(); i != prog.end(); i++) {pnPrintOut(*i);};
    printf("> expr_compilelist retl(%d): ",retl.size()); for (u64 i = 0; i < retl.size(); i+=1) {printf("("); printout_rtype(retl.at(i)); printf(") ");}; printf("\n");
    printf("> expr_compilelist varl(%d): ",varl.size()/3);
    for (u64 i = 0; i < varl.size(); i+=3)
    {
        printf("(");
        printf(symtab_getstring(&gst, varl.at(i)));
        printf(" ");
        printout_rtype(varl.at(i+1));
        printf(" %d) ",varl.at(i+2));
    }; printf("\n");
    printf("< expr_compilelist sp:       %d\n",sp);

    printf("> expr_compilelist spi:      %d\n",spi);


  //assert(prog.size()==0);

  
  if (is_atom(e)) {
    if (eis_sym(e)) {
      PNode n(PNODET_MOV);
      Register x;
      compile_findvar(x,sym_getc(e),varl);
      n.def.push_back(Register(x.type,spi,0));
      n.use.push_back(x);
      prog.push_back(n);
      retl.push_back(x.type);
      sp = spi+1;
      
    } else if (expr_to_rtype(e)!=0) {
      PNode n(PNODET_MOV);
      u32 i = compile_get_kreg(kdata,e);
      u64 type = kdata.kregl.at(i).type;
      n.def.push_back(Register(type,spi,0));
      n.use.push_back(kdata.kregl.at(i));
      prog.push_back(n);
      retl.push_back(type);
      sp = spi+1;

    } else {
    assert(false);retc=0;
    }

  } else {
    
    if (!eis_sym(echild(e,0))) {assert(false);}

    std::list<PNode> PROG;
    std::vector<u64> RETL;


    if (ehas_head_sym(e, gs.sym_sCompoundExpression.get()))
    {
      for (u64 i=1; i<expr_getn(e); i++) {
        RETL.clear();
        sp = expr_compilelist(PROG,RETL,varl,kdata,echild(e,i),spi);
        prog.splice(prog.end(),PROG);
        assert(RETL.size()==sp-spi);
      }
      for (u64 i=0; i<RETL.size(); i++) {
        retl.push_back(RETL.at(i));
      }



    } else if (ehas_head_sym(e, gs.sym_sSequence.get())) {
      sp = spi;
      for (u64 i=1; i<expr_getn(e); i++) {
        sp = expr_compilelist(PROG,RETL,varl,kdata,echild(e,i),sp);
        prog.splice(prog.end(),PROG);
        assert(RETL.size()==sp-spi);
      }
      for (u64 i=0; i<RETL.size(); i++) {
        retl.push_back(RETL.at(i));
      }
      
      

    } else if (ehas_head_sym_length(e, gs.sym_sSet.get(), 2)) {
      sp = expr_compilelist(PROG,RETL,varl,kdata,echild(e,2),spi);
      prog.splice(prog.end(),PROG);
      assert(RETL.size()==sp-spi);
      
      PNode n(PNODET_MOV);
      Register x;
      n.def.push_back(Register(0,0,0));
      n.use.push_back(Register(0,0,0));
          
      if (eis_sym(echild(e,1))) {
        assert(sp-spi==1);
        compile_findvar(x,sym_getc(echild(e,1)),varl);
        n.def.at(0)=x;
        n.use.at(0)=Register(RETL.at(0),spi,0);
        prog.push_back(n);
        retl.push_back(RETL.at(0));
        sp = spi+1;
      
      } else if (ehas_head_sym(echild(e,1), gs.sym_sSequence.get())) {  
        assert(sp-spi==expr_getn(echild(e,1))-1);
        for (u64 i=0; i<sp-spi; i++) {
          compile_findvar(x,sym_getc(echild(e,1,i+1)),varl);
          n.def.at(0)=x;
          n.use.at(0)=Register(RETL.at(i),spi+i,0);
          prog.push_back(n);
          retl.push_back(RETL.at(i));
        }
        
      } else {
        assert(false);
        return -1;
      }



    } else if (ehas_head_sym(e, gs.sym_sPlus.get())) {
      sp = spi;
      for (u64 i=1; i<expr_getn(e); i++) {
        sp = expr_compilelist(PROG,RETL,varl,kdata,echild(e,i),sp);
        prog.splice(prog.end(),PROG);
        assert(RETL.size()==sp-spi);
      }
      u64 typ=expr_compilelist_Plus_helper(prog,RETL,spi,0,sp-spi);
      retl.push_back(typ);
      sp = spi+1;
      


    } else if (ehas_head_sym(e, gs.sym_sMinus.get())) {
      sp = spi;
      for (u64 i=1; i<expr_getn(e); i++) {
        sp = expr_compilelist(PROG,RETL,varl,kdata,echild(e,i),sp);
        prog.splice(prog.end(),PROG);
        assert(RETL.size()==sp-spi);
      }
      if (sp-spi==1) {
        u64 typ = RETL.at(0);
        PNode n(PNODET_FXN, gs.sym_sMinus.get());
        n.use.push_back(Register(typ,spi,0));
        n.def.push_back(Register(typ,spi,0));
        prog.push_back(n);
        retl.push_back(typ);
        sp = spi+1;
      } else {
        u64 tA=RETL.at(0);
        u64 tB=expr_compilelist_Plus_helper(prog,RETL,spi,1,sp-spi);
        u64 tC = std::max(tA,tB);
        PNode n(PNODET_FXN, gs.sym_sMinus.get());
        n.use.push_back(Register(tA,spi+0,0));
        n.use.push_back(Register(tB,spi+1,0));
        n.def.push_back(Register(tC,spi+0,0));
        prog.push_back(n);
        retl.push_back(tC);
        sp = spi+1;
      }

    } else if (ehas_head_sym_length(e, gs.sym_sSin.get(), 1)) {
      sp = spi;
      for (u64 i=1; i<=1; i++) {
        sp = expr_compilelist(PROG,RETL,varl,kdata,echild(e,i),sp);
        prog.splice(prog.end(),PROG);
        assert(RETL.size()==sp-spi);
      }
        assert(sp-spi==1);

        u64 typ = RETL.at(0);
        PNode n(PNODET_FXN, gs.sym_sSin.get());
        n.use.push_back(Register(typ,spi,0));
        u64 tR = std::max(typ, u64(REGT_FLT) << 32);
        n.def.push_back(Register(tR,spi,0));
        prog.push_back(n);
        retl.push_back(tR);
        sp = spi+1;

    } else if (ehas_head_sym_length(e, gs.sym_sCos.get(), 1)) {
      sp = spi;
      for (u64 i=1; i<=1; i++) {
        sp = expr_compilelist(PROG,RETL,varl,kdata,echild(e,i),sp);
        prog.splice(prog.end(),PROG);
        assert(RETL.size()==sp-spi);
      }
        assert(sp-spi==1);

        u64 typ = RETL.at(0);
        PNode n(PNODET_FXN, gs.sym_sCos.get());
        n.use.push_back(Register(typ,spi,0));
        u64 tR = std::max(typ, u64(REGT_FLT) << 32);
        n.def.push_back(Register(tR,spi,0));
        prog.push_back(n);
        retl.push_back(tR);
        sp = spi+1;

    } else if (ehas_head_sym(e, gs.sym_sTimes.get())) {
      sp = spi;
      for (u64 i=1; i<expr_getn(e); i++) {
        sp = expr_compilelist(PROG,RETL,varl,kdata,echild(e,i),sp);
        prog.splice(prog.end(),PROG);
        assert(RETL.size()==sp-spi);
      }
      u64 typ=expr_compilelist_Times_helper(prog,RETL,spi,0,sp-spi);
      retl.push_back(typ);
      sp = spi+1;



    } else if (ehas_head_sym(e, gs.sym_sDivide.get())) {
      sp = spi;
      for (u64 i=1; i<expr_getn(e); i++) {
        sp = expr_compilelist(PROG,RETL,varl,kdata,echild(e,i),sp);
        prog.splice(prog.end(),PROG);
        assert(RETL.size()==sp-spi);
      }
      if (sp-spi==1) {
        u64 typ = RETL.at(0);
        PNode n(PNODET_FXN, gs.sym_sDivide.get());
        n.use.push_back(Register(typ,spi,0));
        n.def.push_back(Register(typ,spi,0));
        prog.push_back(n);
        retl.push_back(typ);
        sp = spi+1;
      } else {
        u64 tA=RETL.at(0);
        u64 tB=expr_compilelist_Times_helper(prog,RETL,spi,1,sp-spi);
        u64 tC = std::max(tA,tB);
        PNode n(PNODET_FXN, gs.sym_sDivide.get());
        n.use.push_back(Register(tA,spi+0,0));
        n.use.push_back(Register(tB,spi+1,0));
        n.def.push_back(Register(tC,spi+0,0));
        prog.push_back(n);
        retl.push_back(tC);
        sp = spi+1;
      }



    } else if (ehas_head_sym_length(e, gs.sym_sPower.get(), 2)) {
      i64 n;
      if (eis_int(echild(e,2)) && fmpz_fits_si(eint_data(echild(e,2)))) {n = fmpz_get_si(eint_data(echild(e,2)));}
      else if (ehas_head_sym_length(echild(e,2),gs.sym_sMinus.get(), 1) && eis_int(echild(e,2,1)) && fmpz_fits_si(eint_data(echild(e,2,1)))) {n = -fmpz_get_si(eint_data(echild(e,2,1)));}
      else {assert(false); return -1;}

      sp = spi;
      sp = expr_compilelist(PROG,RETL,varl,kdata,echild(e,1),sp);
      prog.splice(prog.end(),PROG);
      assert(RETL.size()==sp-spi);
      assert(sp-spi==1);
      
      u64 type=0;
      PNode mov_n(PNODET_MOV);
      mov_n.def.push_back(Register(0,0,0));
      mov_n.use.push_back(Register(0,0,0));
      
      PNode mul_n(PNODET_FXN, gs.sym_sTimes.get());
      mul_n.def.push_back(Register(0,0,0));
      mul_n.use.push_back(Register(0,0,0));
      mul_n.use.push_back(Register(0,0,0));

      bool rcp=false;
      if (n<0) {n=-n; rcp=true;}
      if (n==0) {
        u32 i = compile_get_kreg(kdata, eget_cint(1));
        type = kdata.kregl.at(i).type;
        mov_n.def.at(0)=Register(type,spi,0);
        mov_n.use.at(0)=kdata.kregl.at(i);
        prog.push_back(mov_n);
      } else {
        type=RETL.at(0);
        bool empty=true;
        while (n>=1) {
          if (!(n==1&&empty)){
            if ((n&1)==1) {
              if (empty) {
                mov_n.use.at(0)=Register(type,spi,0);
                mov_n.def.at(0)=Register(type,spi+1,0);
                prog.push_back(mov_n);
              } else {
                mul_n.use.at(0)=Register(type,spi+1,0);
                mul_n.use.at(1)=Register(type,spi,0);
                mul_n.def.at(0)=Register(type,spi+1,0);
                prog.push_back(mul_n);    
              }
              empty=false;
            }
            if (n==1) {
              mov_n.use.at(0)=Register(type,spi+1,0);
              mov_n.def.at(0)=Register(type,spi,0);
              prog.push_back(mov_n);              
            }
          }
          n=n/2;
          if (n>=1) {
            mul_n.use.at(0)=Register(type,spi,0);
            mul_n.use.at(1)=Register(type,spi,0);
            mul_n.def.at(0)=Register(type,spi,0);
            prog.push_back(mul_n);
          }
        }
      }
      if (rcp) {
        PNode rcp_n(PNODET_FXN, gs.sym_sDivide.get());
        rcp_n.def.push_back(Register(type,spi,0));
        rcp_n.use.push_back(Register(type,spi,0));        
        prog.push_back(rcp_n);
      }
      retl.push_back(type);
      sp = spi+1;



    } else if (ehas_head_sym(e, gs.sym_sLess.get())) {
      sp = spi;
      for (u64 i=1; i<expr_getn(e); i++) {
        sp = expr_compilelist(PROG,RETL,varl,kdata,echild(e,i),sp);
        prog.splice(prog.end(),PROG);
        assert(RETL.size()==sp-spi);
      }
      assert(sp-spi==2);
      
      PNode n(PNODET_FXN, gs.sym_sLess.get());
      u64 rtype= u64(REGT_BOL)<<32;
      n.def.push_back(Register(rtype,spi,0));
      n.use.push_back(Register(RETL.at(0),spi,0));
      n.use.push_back(Register(RETL.at(1),spi+1,0));
      prog.push_back(n);
      retl.push_back(rtype);
      sp = spi+1;
      
      

    } else if (ehas_head_sym_length(e, gs.sym_sWhile.get(), 2)) {
      
      std::list<PNode> PROG2;

      sp = expr_compilelist(PROG,RETL,varl,kdata,echild(e,1),spi);
      assert(RETL.size()==sp-spi);
      assert(sp-spi==1);
      u64 condtype = RETL.at(0);
      RETL.clear();
            
      sp = expr_compilelist(PROG2,RETL,varl,kdata,echild(e,2),spi);
      assert(RETL.size()==sp-spi);
      RETL.clear();
      
      PNode n(PNODET_JMP);
      n.succ.push_back(1+PROG2.size());
      prog.push_back(n);
      n.type = PNODET_JT;
      n.use.push_back(Register(condtype,spi,0));
      n.succ.at(0) = (-PROG2.size()-PROG.size());
      prog.splice(prog.end(),PROG2);
      prog.splice(prog.end(),PROG);
      prog.push_back(n);

      sp = spi;      
      
      

    } else {
      assert(false);
        return -1;
    }
    
    
  }

  printf("< expr_compilelist e:        "); std::cout << ex_tostring_full(e); printf("\n");
  printf("< expr_compilelist prog:     \n"); for (std::list<PNode>::iterator i = prog.begin(); i != prog.end(); i++) {pnPrintOut(*i);};

  printf("> expr_compilelist retl(%d): ",retl.size()); for (u64 i = 0; i < retl.size(); i+=1) {printf("("); printout_rtype(retl.at(i)); printf(") ");}; printf("\n");

//  printf("< expr_compilelist retl(%d): ",retl.size()); for (u64 i = 0; i < retl.size(); i+=1) {printf("(%d) ",retl.at(i));}; printf("\n");
  printf("< expr_compilelist varl(%d): ",varl.size()/3);
  for (u64 i = 0; i < varl.size(); i+=3) {
    printf("(");
    printf(symtab_getstring(&gst, varl.at(i)));
    printf(" ");
    printout_rtype(varl.at(i+1));
    printf(" %d) ",varl.at(i+2));
  }; printf("\n");
  printf("< expr_compilelist sp:       %d\n",sp);

  return sp;
}






//*****************************************
void compute_pred_succ(CProgram&prog) {
  for (u64 i=0; i<prog.node.size(); i++) {
    for (u64 j=0; j<prog.node.at(i).succ.size(); j++) {
      prog.node.at(i).succ.at(j)+=i;
    }
    if (prog.node.at(i).type!=PNODET_RET && prog.node.at(i).type!=PNODET_JMP) {
      prog.node.at(i).succ.push_back(i+1);
    }
    for (u64 j=0; j<prog.node.at(i).succ.size(); j++) {
      u32 k = prog.node.at(i).succ.at(j);
      prog.node.at(k).pred.push_back(i);
    }
  }
  return;
}


//**************************************************8
void _rpo_helper(CProgram&prog, u32 x, u64&k) {
  //printf("helper at x=%d\n",x);
  if (prog.rpo.at(x)==-2) {prog.rpo.at(x)=-1;}
  for (u64 i=0; i<prog.node.at(x).succ.size(); i++) {
    u32 y=prog.node.at(x).succ.at(i);
    //printf("helper at y=%d\n",y);
    if (prog.rpo.at(y)==-2) {
      _rpo_helper(prog, y, k);
    }
  }  
  k--;
  prog.rpo.at(x)=k;
  prog.rpoi.at(k)=x;
}

void compute_reverse_post_order(CProgram&prog) {
  u64 k=prog.node.size();
  prog.rpo.resize(k);
  prog.rpoi.resize(k);
  for (u64 i=0; i<k; i++) {
    prog.rpo.at(i)=-2;
  }
  _rpo_helper(prog,0,k);
  return;
}


//**************************************************8
void compute_idom(CProgram&prog) {
  u64 n = prog.node.size();
  prog.idom.resize(n);
  for (u64 i=0; i<n; i++) {
    prog.idom.at(i)=-1;
  }
  prog.idom.at(0) = 0;
//printf("here\n");
  bool changed=true;
  while (changed) {
    changed=false;
    for (u64 bi=1; bi<n; bi++) {
      u32 b=prog.rpoi.at(bi);
//printf("b=%d\n",b);getch();
      u32 new_idom=-1;        
      for (u64 ni=0; ni<prog.node.at(b).pred.size(); ni++) {
        new_idom = prog.node.at(b).pred.at(ni);
        if (prog.idom.at(new_idom) != -1) {break;}
      }
      assert(new_idom!=-1);
//printf("new_idom=%d\n",new_idom);getch();
      for (u64 pi=0; pi<prog.node.at(b).pred.size(); pi++) {
        u32 p = prog.node.at(b).pred.at(pi);
        if (p==new_idom) {continue;}
//printf("  pred=%d\n",p);getch();
        if (prog.idom.at(p) != -1) {
          u32 f1=p, f2=new_idom;
          while (f1!=f2) {
            while (prog.rpo.at(f1)>prog.rpo.at(f2)) {f1=prog.idom.at(f1);/*printf("(1)  f1: %d f2: %d\n",f1,f2);getch();*/}
            while (prog.rpo.at(f2)>prog.rpo.at(f1)) {f2=prog.idom.at(f2);/*printf("(2)  f1: %d f2: %d\n",f1,f2);getch();*/}
          }
          new_idom=f1;
        }
      }
      if (prog.idom.at(b)!=new_idom) {
        prog.idom.at(b) = new_idom;
//printf("  idom(%d)=%d\n",b,prog.idom.at(b));getch();
        changed = true;
      }
    }
  }
  
  
  prog.idomc.clear();
  prog.idomc.resize(n,std::vector<u32>());
  for (u64 k=0; k<n; k++) {
    prog.idomc.at(prog.idom.at(k)).push_back(k);
    //printf(" %d push back %d \n",prog.idom.at(k),k);
  }
  
  return;
}




//**************************************************

void vU32Add(std::vector<u32>&v, u32 a) {
  for (u64 j=0; j<v.size(); j++) {
    if (v.at(j)==a) {return;}
  }
  v.push_back(a);
}



bool vU32MemberQ(std::vector<u32>&v, u32 a) {
  for (u64 j=0; j<v.size(); j++) {
    if (v.at(j)==a) {return true;}
  }
  return false;
}

u64 vU32Position(std::vector<u32>&v, u32 a) {
  for (u64 j=0; j<v.size(); j++) {
    if (v.at(j)==a) {return j;}
  }
  assert(false);
  return v.size();
}

void vU32RemoveMul(std::vector<u32>&v, u32 a) {
  for (u64 j=0; j<v.size(); j++) {
    if (v.at(j)==a) {
      v.at(j)=v.back();
      v.pop_back();
      return;
    }
  }
  assert(false);
}

void vU32InsertMul(std::vector<u32>&v, u32 a) {
  //for (u64 j=0; j<v.size(); j++) {
  //  if (v.at(j)==a) {return;}
  //}
  v.push_back(a);
}


// replace x by y
void vU32Replace(std::vector<u32>&v, u32 x, u32 y) {
  for (u64 j=0; j<v.size(); j++) {
    if (v.at(j)==x) {
      v.at(j)=y;
      return;
    }
  }
  assert(false);
}




void compute_df(CProgram&prog) {
  u64 n = prog.node.size();
  for (u64 i=0; i<n; i++) {
    prog.node.at(i).df.resize(0);
  }
  for (u64 b=0; b<n; b++) {
    if (prog.node.at(b).pred.size()<1) {continue;}
    for (u64 pi=0; pi<prog.node.at(b).pred.size(); pi++) {
      u32 run = prog.node.at(b).pred.at(pi);
      while (run != prog.idom.at(b)) {
  
        vU32Add(prog.node.at(run).df,b);
/*      
        for (u64 j=0; j<prog.node.at(run).df.size(); j++) {
          if (prog.node.at(run).df.at(j)==b) {goto already_in;}
        }
        prog.node.at(run).df.push_back(b);
already_in:
*/        
        
        run = prog.idom.at(run);
        
      }
    }
  }
}


//**************************************************

void GenName(std::vector<std::vector<u32> >&stacks, u32 vi) {
  //printf("pushing %d \n",vi);
  u32 i = stacks.at(vi).at(0);
  i++;
  stacks.at(vi).at(0)=i;
  stacks.at(vi).push_back(i);
}

u32 TopStack(std::vector<std::vector<u32> >&stacks, u32 vi) {
  assert(stacks.at(vi).size()>0);
  if (stacks.at(vi).size()==1) {printf("stack empty\n"); return 0;} // triggered for phi functions with one use not defined
  return stacks.at(vi).back(); 
}

void PopStack(std::vector<std::vector<u32> >&stacks, u32 vi) {
  //printf("popping %d \n",vi);
  assert(stacks.at(vi).size()>1);
  stacks.at(vi).pop_back();
  return;
}


void _rename_helper(CProgram&prog, std::vector<std::vector<u32> >&stacks, std::vector<u32>&visited, u32 b) {
  
  //printf("_rename_helper b=%d \n",b);
  if (visited.at(b)!=0) {return;}
  visited.at(b)=-1;
 
  // rename definitions of phi functions in b
  for (std::vector<u32>::iterator pi = prog.node.at(b).phi.begin(); pi != prog.node.at(b).phi.end(); pi++) {
    Register v = prog.phinode.at(*pi).def;
    u32 vi = prog.rmap.at(v);
    GenName(stacks,vi);
    v.var=TopStack(stacks,vi);
    prog.phinode.at(*pi).def = v;    //printf("phi def renamed: "); rPrintOut(v); printf("\n");
  }
  // rename uses in b
  for (std::vector<Register>::iterator ri = prog.node.at(b).use.begin(); ri != prog.node.at(b).use.end(); ri++) {
    Register v = *ri;
    if (v.var==-1) {continue;}//skip constants
    u32 vi = prog.rmap.at(v);
    v.var=TopStack(stacks,vi);
    *ri = v;    //printf("use renamed: "); rPrintOut(v); printf("\n");
  }
  // rename definitions in b
  for (std::vector<Register>::iterator ri = prog.node.at(b).def.begin(); ri != prog.node.at(b).def.end(); ri++) {
    Register v = *ri;
    u32 vi = prog.rmap.at(v);
    GenName(stacks,vi);
    v.var=TopStack(stacks,vi);
    *ri = v;    //printf("def renamed: "); rPrintOut(v); printf("\n");
  }
  // rename uses in phi functions in successors to b
  for (std::vector<u32>::iterator si = prog.node.at(b).succ.begin(); si != prog.node.at(b).succ.end(); si++) {
    u64 j = vU32Position(prog.node.at(*si).pred,b);
    for (std::vector<u32>::iterator pi = prog.node.at(*si).phi.begin(); pi != prog.node.at(*si).phi.end(); pi++) {
      Register v = prog.phinode.at(*pi).use.at(j);
      if (v.var==-1) {continue;}//skip constants
      u32 vi = prog.rmap.at(v);
      v.var=TopStack(stacks,vi);
      prog.phinode.at(*pi).use.at(j) = v;      //printf("phi use renamed:"); rPrintOut(v); printf("\n");
    }
  }  
  // rename children in dominator tree
  for (std::vector<u32>::iterator ci = prog.idomc.at(b).begin(); ci != prog.idomc.at(b).end(); ci++) {
    _rename_helper(prog,stacks,visited,*ci);
  }
  // pop renames of definitions in b
  for (std::vector<Register>::iterator ri = prog.node.at(b).def.begin(); ri != prog.node.at(b).def.end(); ri++) {
    Register v = *ri;
    v.var=0;
    u32 vi = prog.rmap.at(v);
    PopStack(stacks,vi);
  }
  // pop renames of definitions of phi functions in b
  for (std::vector<u32>::iterator pi = prog.node.at(b).phi.begin(); pi != prog.node.at(b).phi.end(); pi++) {
    Register v = prog.phinode.at(*pi).def;
    v.var=0;
    u32 vi = prog.rmap.at(v);
    PopStack(stacks,vi);
  }
  return;  
}






void compute_ssa(CProgram&prog) {
  // fill in register map and each register's set of definitions
  assert(0==prog.rmap.size());
  assert(0==prog.rmapi.size());
  assert(0==prog.rdef.size());
  assert(0==prog.ruse.size());
  u32 k=0;
  u64 ni=0;
  for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++ni, ++nit) {
    for (std::vector<Register>::iterator rit = nit->def.begin(); rit != nit->def.end(); ++rit) {
      std::pair<std::map<Register,u32>::iterator,bool> ret;
      ret = prog.rmap.insert ( std::pair<Register,u32>(*rit,k) );
      if (ret.second==true) {
        k++;
        prog.rmapi.push_back(*rit);
        prog.rdef.push_back(std::vector<u32>(1,ni));
      } else {
        vU32Add(prog.rdef.at(ret.first->second),ni);
      }
    }
  }
  assert(k==prog.rmap.size());
  assert(k==prog.rmapi.size());
  assert(k==prog.rdef.size());
  prog.ruse.resize(k,std::vector<u32>());  // we don't use this at this point, so just set them all empty;
  
  // compute phi functions
  u32 phik=0;
  prog.phinode.clear();
  for (u64 vi=0;  vi<prog.rmapi.size(); vi++) {
    Register v=prog.rmapi.at(vi);
//printf(" working on variable "); rPrintOut(v); printf("\n");
    std::vector<u32> WorkList;
    std::vector<u32> EverOnWorkList;
    std::vector<u32> AlreadyHasPhiFunc;
    WorkList = prog.rdef.at(vi);
    EverOnWorkList = WorkList;
    AlreadyHasPhiFunc.clear();
    while (WorkList.size()>0) {
      u32 n=WorkList.back();
      WorkList.pop_back();
//printf("  at node n=%d \n", n);
      for (std::vector<u32>::iterator d = prog.node.at(n).df.begin(); d != prog.node.at(n).df.end(); ++d) {
//        printf("    at df d=%d \n",*d);      
        if (vU32MemberQ(AlreadyHasPhiFunc,*d)) {continue;}
        AlreadyHasPhiFunc.push_back(*d);
        prog.node.at(*d).phi.push_back(phik);
        PhiNode nn;
        nn.loc=*d;
        nn.def=v;
        nn.use.clear(); nn.use.resize(prog.node.at(*d).pred.size(),v);
        prog.phinode.push_back(nn);
        phik++;
        if (!vU32MemberQ(EverOnWorkList,*d)) {
          vU32Add(WorkList,*d);
          EverOnWorkList.push_back(*d);
        }
      }
    }
  }
  
  printf(" after computing phi functions \n");
  cpPrintOut(prog);
  
  // rename variables
  std::vector<std::vector<u32> > stacks;
  stacks.resize(prog.rmapi.size(),std::vector<u32>(1,0));
  std::vector<u32> visited(prog.node.size(),0);
  _rename_helper(prog,stacks,visited,0);
  for (std::vector<std::vector<u32> >::iterator i = stacks.begin(); i!= stacks.end(); i++) {
    assert(i->size()==1); 
  }
  
  // fill in register data
  prog.rmap.clear();
  prog.rmapi.clear();
  prog.rdef.clear();
  prog.ruse.clear();

  return;

}



bool domgreater(std::vector<u32>&tree, u32 a, u32 b) {
  if (a==b) {return false;}
  while (tree.at(b)!=b) {
    b=tree.at(b);
    if (a==b) {return true;}
  }
  return false;
}

bool domgreaterequal(std::vector<u32>&tree, u32 a, u32 b) {
  if (a==b) {return true;}
  while (tree.at(b)!=b) {
    b=tree.at(b);
    if (a==b) {return true;}
  }
  return false;
}



void optimize_ssa(CProgram&prog) {
  assert(0==prog.rmap.size());
  assert(0==prog.rmapi.size());
  assert(0==prog.rdef.size());
  assert(0==prog.ruse.size());
  u32 k=0;
  u64 ni=0;
  for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++ni, ++nit) {
    for (std::vector<Register>::iterator rit = nit->def.begin(); rit != nit->def.end(); ++rit) {
      std::pair<std::map<Register,u32>::iterator,bool> ret;
      ret = prog.rmap.insert ( std::pair<Register,u32>(*rit,k) );
      assert(ret.second); // only supposed to be one def
      prog.rmapi.push_back(*rit);
      prog.rdef.push_back(std::vector<u32>(1,ni));
      k++;
    }
  }
  for (std::vector<PhiNode>::iterator nit = prog.phinode.begin(); nit != prog.phinode.end(); ++ni, ++nit) {
    Register rit = nit->def;
    std::pair<std::map<Register,u32>::iterator,bool> ret;
    ret = prog.rmap.insert ( std::pair<Register,u32>(rit,k) );
    assert(ret.second==true); // only supposed to be one def
    prog.rmapi.push_back(rit);
    prog.rdef.push_back(std::vector<u32>(1,ni));
    k++;
  }
  assert(k==prog.rmap.size());
  assert(k==prog.rmapi.size());
  assert(k==prog.rdef.size());
  prog.ruse.resize(k,std::vector<u32>());  // set them all empty;

  ni=0;
  for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++ni, ++nit) {
    for (std::vector<Register>::iterator rit = nit->use.begin(); rit != nit->use.end(); ++rit) {
      if (rit->var==-1) {continue;}
      std::pair<std::map<Register,u32>::iterator,bool> ret;
      ret = prog.rmap.insert ( std::pair<Register,u32>(*rit,-1) );
      if (!ret.second) {
        vU32InsertMul(prog.ruse.at(ret.first->second),ni);
      } else {
        prog.rmap.erase(*rit);
      }
    }
  }
  for (std::vector<PhiNode>::iterator nit = prog.phinode.begin(); nit != prog.phinode.end(); ++ni, ++nit) {
    for (std::vector<Register>::iterator rit = nit->use.begin(); rit != nit->use.end(); ++rit) {
      if (rit->var==-1) {continue;}
      std::pair<std::map<Register,u32>::iterator,bool> ret;
      ret = prog.rmap.insert ( std::pair<Register,u32>(*rit,-1) );
      if (!ret.second) {
        vU32InsertMul(prog.ruse.at(ret.first->second),ni);
      } else {
        prog.rmap.erase(*rit);
      }
    }
  }

  
  printf(" inside optimize_ssa \n");
  cpPrintOut(prog);

  // replace mov operands
  u64 vn = prog.rmapi.size();
  std::vector<u32> replace;
  replace.clear();
  replace.reserve(vn);
  for (u32 i=0; i<vn; i++) {
    replace.push_back(i);
  }

//  u64 kvn = prog.kdata.kregl.size();
  std::vector<u32> kreplace;
  kreplace.clear();
  kreplace.resize(vn,-1);


  for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++nit) {
    if (nit->type==PNODET_MOV) {
      assert(nit->use.size()==1 && nit->use.size()==1);
      Register r=nit->use.at(0);
      assert(r.var!=0);
      if (r.var==-1) {
        kreplace.at(prog.rmap.at(nit->def.at(0))) = r.num;
      } else {
        replace.at(prog.rmap.at(nit->def.at(0)))=prog.rmap.at(nit->use.at(0));
      }
    }
  }
  for (std::vector<PhiNode>::iterator pit = prog.phinode.begin(); pit != prog.phinode.end(); ++pit) {
    u32 count=0, regi=-1, isk=0;
    for (std::vector<Register>::iterator rit = pit->use.begin(); rit != pit->use.end(); ++rit) {
      if (rit->var!=0) {
        count++;
        if (rit->var==-1) {
          regi=rit->num;
          isk=-1;
        } else {
          regi=prog.rmap.at(*rit);
          isk=0;
        }
      }
    }
    if (count==1) {
      if (isk==0) {
        replace.at(prog.rmap.at(pit->def))=regi;
      } else {
        kreplace.at(prog.rmap.at(pit->def))=regi;
      }
    }
  }


  // **** loop ****

  bool changed = true;
  while (changed) {
    changed = false;

    ni=0;
    for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++ni, ++nit) {
      for (std::vector<Register>::iterator rit = nit->use.begin(); rit != nit->use.end(); ++rit) {
        if (rit->var==0) {continue;}
        if (rit->var==-1) {continue;}
        u32 ai = prog.rmap.at(*rit);
        u32 bi = replace.at(ai);
        if (ai!=bi) {
          vU32RemoveMul(prog.ruse.at(ai),ni);
          vU32InsertMul(prog.ruse.at(bi),ni);
          *rit = prog.rmapi.at(bi);
        }
        u32 ki = kreplace.at(ai);
        if (ki!=-1) {
          vU32RemoveMul(prog.ruse.at(ai),ni);
          *rit = prog.kdata.kregl.at(ki);
        }
      }
    }
    for (std::vector<PhiNode>::iterator nit = prog.phinode.begin(); nit != prog.phinode.end(); ++ni, ++nit) {
      for (std::vector<Register>::iterator rit = nit->use.begin(); rit != nit->use.end(); ++rit) {
        if (rit->var==0) {continue;}
        if (rit->var==-1) {continue;}
        u32 ai = prog.rmap.at(*rit);
        u32 bi = replace.at(ai);
        if (ai!=bi) {
          vU32RemoveMul(prog.ruse.at(ai),ni);
          vU32InsertMul(prog.ruse.at(bi),ni);
          *rit = prog.rmapi.at(bi);
        }
        u32 ki = kreplace.at(ai);
        if (ki!=-1) {
          vU32RemoveMul(prog.ruse.at(ai),ni);
          *rit = prog.kdata.kregl.at(ki);
        }
      }
    }
    printf(" inside optimize_ssa after replace mov\n");
    cpPrintOut(prog);

    // dead code ellimination
    ni=0;
    for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++ni, ++nit) {
//      printf("%d\n",ni); pnPrintOut(*nit);
      if (nit->type==PNODET_NOP) {continue;}
      if (nit->type!=PNODET_FXN && nit->type!=PNODET_MOV) {continue;}
      bool used = false;
      for (std::vector<Register>::iterator rit = nit->def.begin(); rit != nit->def.end(); ++rit) {
        used |= prog.ruse.at(prog.rmap.at(*rit)).size()>0;
      }
      if (used) {continue;}
      nit->type=PNODET_NOP;
      nit->fxn=0;
      for (std::vector<Register>::iterator rit = nit->def.begin(); rit != nit->def.end(); ++rit) {
        vU32RemoveMul(prog.rdef.at(prog.rmap.at(*rit)),ni);
      }
      nit->def.clear();
      for (std::vector<Register>::iterator rit = nit->use.begin(); rit != nit->use.end(); ++rit) {
        assert(rit->var!=0);
        if (rit->var==-1) {continue;}
        vU32RemoveMul(prog.ruse.at(prog.rmap.at(*rit)),ni);
      }
      nit->use.clear();
      changed = true;
    }
    for (std::vector<PhiNode>::iterator nit = prog.phinode.begin(); nit != prog.phinode.end(); ++ni, ++nit) {
//      printf("%d : ",ni); pnPrintOut(*nit);
      if (nit->loc==-1) {continue;}
      bool used = prog.ruse.at(prog.rmap.at(nit->def)).size()>0;
      if (used) {continue;}
      vU32RemoveMul(prog.node.at(nit->loc).phi,ni-prog.node.size());
      nit->loc=-1;
      vU32RemoveMul(prog.rdef.at(prog.rmap.at(nit->def)),ni);
      nit->def=Register(0,0,0);
      for (std::vector<Register>::iterator rit = nit->use.begin(); rit != nit->use.end(); ++rit) {
        if (rit->var==0) {continue;}
        if (rit->var==-1) {continue;}
        vU32RemoveMul(prog.ruse.at(prog.rmap.at(*rit)),ni);
      }
      nit->use.clear();
      changed = true;
    }
    printf(" inside optimize_ssa after dead code ellimination \n");
    cpPrintOut(prog);

    // common subexpression ellimination

    ni=0;
    for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++ni, ++nit) {
      if (nit->type!=PNODET_FXN) {continue;}
      u32 mi=0;
      for (std::vector<PNode>::iterator mit = prog.node.begin(); mit != prog.node.end(); ++mi, ++mit) {
        if (mit->type!=PNODET_FXN) {continue;}
        if (mit->fxn!=nit->fxn) {continue;}
        if (mit->use.size()!=nit->use.size()) {continue;}
        bool same = true;
        for (u64 kk=0; kk<nit->use.size(); kk++) {
          if (nit->use.at(kk)!=mit->use.at(kk)) {
            same =false;
          }
        }
        if (!same) {continue;}
        if (!domgreater(prog.idom,ni,mi)) {continue;}
        for (u64 kk=0; kk<nit->def.size(); kk++) {
          replace.at(prog.rmap.at(mit->def.at(kk)))=prog.rmap.at(nit->def.at(kk));
          rPrintOut(mit->def.at(kk));printf(" => ");
          rPrintOut(nit->def.at(kk));printf("\n");
          changed = true;
        }
      }
    }
    
    printf(" inside optimize_ssa after CSE \n");
    cpPrintOut(prog);

  }

  prog.rmap.clear();
  prog.rmapi.clear();
  prog.rdef.clear();
  prog.ruse.clear();


}






struct RegAllocData {
  std::vector<u32> undo;
  std::vector<u32> r2c; // register to color
  std::vector<u32> c2r; // color to register
  u64 type;
};

void _reg_alloc_helper(CProgram&prog,RegAllocData&data,u32 b) {
  
  u64 sp = data.undo.size();
  
  
  // color definitions of phi functions in b
  for (std::vector<u32>::iterator pi = prog.node.at(b).phi.begin(); pi != prog.node.at(b).phi.end(); pi++) {
    Register r = prog.phinode.at(*pi).def;
    if (r.type!=data.type) {continue;}
    if (r.var==0) {r.num=-1; prog.phinode.at(*pi).def=r; continue;}
    u32 ri = prog.rmap.at(r);
    u32 newc=0; while (data.c2r.at(newc)!=-1) { newc++;}
    assert(data.c2r.at(newc)==-1);
    data.r2c.at(ri)=newc;
    data.c2r.at(newc)=ri; data.undo.push_back(newc); data.undo.push_back(-1);
    r.num=newc;
    r.var=0;    
    prog.phinode.at(*pi).def=r;    
  }

  
  // color uses in b
  std::vector<u32> colors_in_used;
  for (std::vector<Register>::iterator rit = prog.node.at(b).use.begin(); rit != prog.node.at(b).use.end(); ++rit) {
    if (rit->type!=data.type) {continue;}
    if (rit->var==-1) {continue;}
    if (rit->var==0) {rit->num=-1;continue;}
    u32 ri = prog.rmap.at(*rit);
    u32 newc =data.r2c.at(ri); 
    assert(newc!=-1);
    assert(data.c2r.at(newc)==ri); // variable is live
    rit->num=newc;
    rit->var=0;
    vU32Add(colors_in_used,newc);
    assert(vU32MemberQ(prog.ruse.at(ri),b));
    vU32RemoveMul(prog.ruse.at(ri),b);
    // if variable is not used down the tree, remove it from live list
    bool dead=true;
    for ( std::vector<u32>::iterator ni=prog.ruse.at(ri).begin(); ni!=prog.ruse.at(ri).end(); ++ni) {
      if (*ni<prog.node.size()) { // if use is an ordinary node
        if (domgreaterequal(prog.idom,b,*ni)) {dead=false;}
      } else { // if use is a phi node
        u32 pi = *ni-prog.node.size();
        u32 loc = prog.phinode.at(pi).loc;
        for (u64 j=0; j< prog.phinode.at(pi).use.size(); j++) {
          if (prog.rmap.at(prog.phinode.at(pi).use.at(j))!=ri) {continue;}
          if (domgreaterequal(prog.idom,b,prog.node.at(loc).pred.at(j))) {dead=false;}
        }
      }
    }
    if (dead) {
      data.c2r.at(newc)=-1; data.undo.push_back(newc); data.undo.push_back(ri);
    }
  }


  // color definitions in b
  for (std::vector<Register>::iterator rit = prog.node.at(b).def.begin(); rit != prog.node.at(b).def.end(); ++rit) {
    Register r = *rit;
    if (r.type!=data.type) {continue;}
    if (r.var==0) {r.num=-1; *rit=r; continue;}
    u32 ri = prog.rmap.at(r);
    u32 newc=0; while (data.c2r.at(newc)!=-1 || vU32MemberQ(colors_in_used,newc)) { newc++;}
    assert(data.c2r.at(newc)==-1);
    data.r2c.at(ri)=newc;
    data.c2r.at(newc)=ri; data.undo.push_back(newc); data.undo.push_back(-1);
    r.num=newc;
    r.var=0;
    *rit=r;
  }


  // rename uses in phi functions in successors to b
  for (std::vector<u32>::iterator si = prog.node.at(b).succ.begin(); si != prog.node.at(b).succ.end(); si++) {
    u64 j = vU32Position(prog.node.at(*si).pred,b);
    for (std::vector<u32>::iterator pi = prog.node.at(*si).phi.begin(); pi != prog.node.at(*si).phi.end(); pi++) {
      Register r = prog.phinode.at(*pi).use.at(j);
      if (r.type!=data.type) {continue;}
      if (r.var==0) {r.num=-1; prog.phinode.at(*pi).use.at(j)=r; continue;}
      if (r.var==-1) {continue;}
      u32 ri = prog.rmap.at(r);
      u32 newc =data.r2c.at(ri); 
      assert(newc!=-1);
      assert(data.c2r.at(newc)==ri); // variable is live
      r.num=newc;
      r.var=0;
      assert(vU32MemberQ(prog.ruse.at(ri),*pi+prog.node.size()));
      vU32RemoveMul(prog.ruse.at(ri),*pi+prog.node.size());
      bool dead=true;
      for ( std::vector<u32>::iterator ni=prog.ruse.at(ri).begin(); ni!=prog.ruse.at(ri).end(); ++ni) {
        if (*ni<prog.node.size()) { // if use is an ordinary node
          if (domgreaterequal(prog.idom,b,*ni)) {dead=false;}
        } else { // if use is a phi node
          u32 ppii = *ni-prog.node.size();
          u32 loc = prog.phinode.at(ppii).loc;
          for (u64 jj=0; jj< prog.phinode.at(ppii).use.size(); jj++) {
            if (prog.rmap.at(prog.phinode.at(ppii).use.at(jj))!=ri) {continue;}
            if (domgreaterequal(prog.idom,b,prog.node.at(loc).pred.at(jj))) {dead=false;}
          }
        }
      }
      if (dead) {
        data.c2r.at(newc)=-1; data.undo.push_back(newc); data.undo.push_back(ri);
      }
      prog.phinode.at(*pi).use.at(j) = r;
    }
  }

  // color children down the tree
  for (std::vector<u32>::iterator cit = prog.idomc.at(b).begin(); cit != prog.idomc.at(b).end(); ++cit) {
    if (*cit==b) {continue;}
    _reg_alloc_helper(prog,data,*cit);
  }
  
  // undo colorings
  while(data.undo.size()>sp) {
    u32 ri = data.undo.back(); data.undo.pop_back();
    u32 co = data.undo.back(); data.undo.pop_back();
    data.c2r.at(co)=ri;
  }
  assert(data.undo.size()==sp);


  return;
}



void optimize_regalloc(CProgram&prog) {
  assert(0==prog.rmap.size());
  assert(0==prog.rmapi.size());
  assert(0==prog.rdef.size());
  assert(0==prog.ruse.size());
  
  std::set<u64> typeset; // set of register types in prog
  
  u32 k=0;
  u64 ni=0;
  for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++ni, ++nit) {
    for (std::vector<Register>::iterator rit = nit->def.begin(); rit != nit->def.end(); ++rit) {
      std::pair<std::map<Register,u32>::iterator,bool> ret;
      ret = prog.rmap.insert ( std::pair<Register,u32>(*rit,k) );
      assert(ret.second); // only supposed to be one def
      prog.rmapi.push_back(*rit);
      prog.rdef.push_back(std::vector<u32>(1,ni));
      k++;
      typeset.insert(rit->type);
    }
  }
  for (std::vector<PhiNode>::iterator nit = prog.phinode.begin(); nit != prog.phinode.end(); ++ni, ++nit) {
    if (nit->loc==-1) {continue;}
    Register rit = nit->def;
    std::pair<std::map<Register,u32>::iterator,bool> ret;
    ret = prog.rmap.insert ( std::pair<Register,u32>(rit,k) );
    assert(ret.second==true); // only supposed to be one def
    prog.rmapi.push_back(rit);
    prog.rdef.push_back(std::vector<u32>(1,ni));
    k++;
  }
  assert(k==prog.rmap.size());
  assert(k==prog.rmapi.size());
  assert(k==prog.rdef.size());
  prog.ruse.resize(k,std::vector<u32>());  // set them all empty;

  ni=0;
  for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++ni, ++nit) {
    for (std::vector<Register>::iterator rit = nit->use.begin(); rit != nit->use.end(); ++rit) {
      std::pair<std::map<Register,u32>::iterator,bool> ret;
      ret = prog.rmap.insert ( std::pair<Register,u32>(*rit,-1) );
      if (!ret.second) {
        vU32InsertMul(prog.ruse.at(ret.first->second),ni);
      } else {
        prog.rmap.erase(*rit);
      }
    }
  }
  for (std::vector<PhiNode>::iterator nit = prog.phinode.begin(); nit != prog.phinode.end(); ++ni, ++nit) {
    for (std::vector<Register>::iterator rit = nit->use.begin(); rit != nit->use.end(); ++rit) {
      std::pair<std::map<Register,u32>::iterator,bool> ret;
      ret = prog.rmap.insert ( std::pair<Register,u32>(*rit,-1) );
      if (!ret.second) {
        vU32InsertMul(prog.ruse.at(ret.first->second),ni);
      } else {
        prog.rmap.erase(*rit);
      }
    }
  }

  
  printf(" inside optimize_regalloc \n");
  cpPrintOut(prog);
  
  RegAllocData data;
  data.r2c.clear();
  data.r2c.resize(prog.rmapi.size(),-1);
  
  
  for (std::set<u64>::iterator type=typeset.begin(); type!=typeset.end(); ++type) {
    printf("in typeset: %d \n",*type);
    data.type=*type;
    data.c2r.clear();
    data.c2r.resize(prog.rmapi.size(),-1);
    data.undo.clear();
    _reg_alloc_helper(prog,data,0);
  }

  for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++ni, ++nit) {
    nit->df.clear();
  }
  prog.rmap.clear();
  prog.rmapi.clear();
  prog.rdef.clear();
  prog.ruse.clear();
}



#define MNE_BEG 1
#define MNE_RET 2

#define MNE_ADD_fff 4
#define MNE_ADD_ffi (MNE_ADD_fff+1)
#define MNE_ADD_fif (MNE_ADD_fff+2)
#define MNE_ADD_iii (MNE_ADD_fff+3)

#define MNE_MUL_fff 8
#define MNE_MUL_ffi (MNE_MUL_fff+1)
#define MNE_MUL_fif (MNE_MUL_fff+2)
#define MNE_MUL_iii (MNE_MUL_fff+3)

#define MNE_SUB_fff 12
#define MNE_SUB_ffi (MNE_SUB_fff+1)
#define MNE_SUB_fif (MNE_SUB_fff+2)
#define MNE_SUB_iii (MNE_SUB_fff+3)

#define MNE_DIV_fff 16
#define MNE_DIV_ffi (MNE_DIV_fff+1)
#define MNE_DIV_fif (MNE_DIV_fff+2)
#define MNE_DIV_fii (MNE_DIV_fff+3)

#define MNE_SIN_ff 20
#define MNE_SIN_fi 21

#define MNE_COS_ff 22
#define MNE_COS_fi 23


u32 rt_fFiI(Register &r) {
  u64 hi = r.type>>32;
  u64 lo = r.type&0x0FFFFFFFFULL;

//printf("hi: %d  lo: %d  return: ",hi,lo);

  if (hi==REGT_FLT && lo==0)  {printf("0\n");return 0;}
  if (hi==REGT_FLT && lo!=0)  {printf("1\n");return 1;}
  if (hi==REGT_INT && lo==0)  {printf("2\n");return 2;}
  if (hi==REGT_INT && lo!=0)  {printf("3\n");return 3;}
  assert(false);
    return -1;
}

u32 rt_fF(Register r) {
  u64 hi = r.type>>32;
  u64 lo = r.type&0x0FFFFFFFFULL;
  if (hi==REGT_FLT && lo==0)  {return 0;}
  if (hi==REGT_FLT && lo!=0)  {return 1;}
  assert(false);
    return -1;
}


u32 rt_plus_minus_times(Register&a, Register&b, Register&c)
{
    u64 ahi = a.type>>32;
    u64 bhi = b.type>>32;
    u64 chi = c.type>>32;

std::cout << "ahi: " << ahi << "  bhi: " << bhi << "  chi: " << chi << std::endl;

    if (ahi == REGT_FLT && bhi == REGT_FLT && chi == REGT_FLT)
        return MNE_ADD_fff - MNE_ADD_fff;
    if (ahi == REGT_FLT && bhi == REGT_FLT && chi == REGT_INT)
        return MNE_ADD_ffi - MNE_ADD_fff;
    if (ahi == REGT_FLT && bhi == REGT_INT && chi == REGT_FLT)
        return MNE_ADD_fif - MNE_ADD_fff;
    if (ahi == REGT_INT && bhi == REGT_INT && chi == REGT_INT)
        return MNE_ADD_iii - MNE_ADD_fff;

    assert(false);
    return -1;
}


u32 rt_divide(Register&a, Register&b, Register&c)
{
    u64 ahi = a.type>>32;
    u64 bhi = b.type>>32;
    u64 chi = c.type>>32;

std::cout << "ahi: " << ahi << "  bhi: " << bhi << "  chi: " << chi << std::endl;

    if (ahi == REGT_FLT && bhi == REGT_FLT && chi == REGT_FLT)
        return MNE_DIV_fff - MNE_DIV_fff;
    if (ahi == REGT_FLT && bhi == REGT_FLT && chi == REGT_INT)
        return MNE_DIV_ffi - MNE_DIV_fff;
    if (ahi == REGT_FLT && bhi == REGT_INT && chi == REGT_FLT)
        return MNE_DIV_fif - MNE_DIV_fff;
    if (ahi == REGT_FLT && bhi == REGT_INT && chi == REGT_INT)
        return MNE_DIV_fii - MNE_DIV_fff;

    assert(false);
    return -1;
}

u32 rt_sin_cos(Register&a, Register&b)
{
    u64 ahi = a.type>>32;
    u64 bhi = b.type>>32;

std::cout << "ahi: " << ahi << "  bhi: " << bhi << std::endl;

    if (ahi == REGT_FLT && bhi == REGT_FLT)
        return MNE_SIN_ff - MNE_SIN_ff;
    if (ahi == REGT_FLT && bhi == REGT_INT)
        return MNE_SIN_fi - MNE_SIN_ff;

    assert(false);
    return -1;
}



ex optimize_makecode(CProgram&prog) {
  
  // add phi instructions
  u32 ni=0;
  for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++ni, ++nit) {
    u64 pind;
    for (u32 pi=0; pi<nit->pred.size(); pi++) {
      PNode node(PNODET_MOV);
      node.def.clear();
      node.use.clear();
      for (std::vector<u32>::iterator phit = nit->phi.begin(); phit != nit->phi.end(); ++phit) {
        if (prog.phinode.at(*phit).def == prog.phinode.at(*phit).use.at(pi)) {continue;}
        node.def.push_back(prog.phinode.at(*phit).def);
        node.use.push_back(prog.phinode.at(*phit).use.at(pi));      
      }
      if (node.def.size()==0) {continue;}
      u32 a = nit->pred.at(pi);
      u32 b = ni;
      u32 n = prog.node.size();
      node.pred.clear(); node.pred.push_back(a);
      node.succ.clear(); node.succ.push_back(b);
      vU32Replace(prog.node.at(a).succ,b,n);
      vU32Replace(prog.node.at(b).pred,a,n);
      prog.node.push_back(node);
    }
    nit->phi.clear();
  }
  prog.phinode.clear();

  // remove useless instructions
  ni=0;
  for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++ni, ++nit) {
    if (!(nit->type==PNODET_NOP || nit->type==PNODET_JMP)) {continue;}
    assert(nit->use.size()==0);
    assert(nit->def.size()==0);
    assert(nit->succ.size()==1);
    u32 s=nit->succ.at(0);
    vU32RemoveMul(prog.node.at(s).pred,ni);
    for (std::vector<u32>::iterator p = nit->pred.begin(); p != nit->pred.end(); ++p) {
      vU32InsertMul(prog.node.at(s).pred,*p);
      vU32Replace(prog.node.at(*p).succ,ni,s);
    }
    nit->pred.clear();
    nit->succ.clear();
  }

  prog.rmap.clear();
  for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++nit) {
    for (std::vector<Register>::iterator rit = nit->use.begin(); rit != nit->use.end(); ++rit) {
      if (rit->var==0) {continue;}
      assert(rit->var==-1);
      prog.rmap.insert ( std::pair<Register,u32>(*rit,0) );
    }
    for (std::vector<Register>::iterator rit = nit->def.begin(); rit != nit->def.end(); ++rit) {
      assert(rit->var==0);
      prog.rmap.insert ( std::pair<Register,u32>(*rit,0) );
    }
  }
  u32 k = prog.rmap.size();
  prog.rmapi.resize(k);
  prog.rdef.resize(k);
  prog.ruse.resize(k);


  // put offsets for constants in rmap
  u32 koffset=0;
  for (std::map<Register,u32>::iterator it=prog.rmap.begin(); it!=prog.rmap.end(); ++it){
    if (it->first.var==0) {continue;}
    it->second = koffset;
    koffset += rtype_to_bsize(it->first.type);
  }
  assert(koffset%8==0);

  // put offsets for registers in rmap
  u32 roffset=koffset;
  for (std::map<Register,u32>::iterator it=prog.rmap.begin(); it!=prog.rmap.end(); ++it){
    if (it->first.var==-1) {continue;}
    it->second = roffset;
    roffset += rtype_to_bsize(it->first.type);
  }
  
  // make code
  u32 offset=0;
  for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++nit) {

    nit->df.clear();
    if (nit->type==PNODET_NOP || nit->type==PNODET_JMP) {continue;}

    nit->phi.push_back(offset);

    if (nit->type==PNODET_FXN) {

      if (nit->fxn== gs.sym_sPlus.get()) {
        nit->df.push_back(MNE_ADD_fff + rt_plus_minus_times(nit->def.at(0), nit->use.at(0), nit->use.at(1)));
        nit->df.push_back(prog.rmap.at(nit->def.at(0)));
        nit->df.push_back(prog.rmap.at(nit->use.at(0)));
        nit->df.push_back(prog.rmap.at(nit->use.at(1)));
        offset+=5;

      } else if (nit->fxn==gs.sym_sTimes.get()) {
        nit->df.push_back(MNE_MUL_fff + rt_plus_minus_times(nit->def.at(0), nit->use.at(0), nit->use.at(1)));
        nit->df.push_back(prog.rmap.at(nit->def.at(0)));
        nit->df.push_back(prog.rmap.at(nit->use.at(0)));
        nit->df.push_back(prog.rmap.at(nit->use.at(1)));
        offset+=5;

      } else if (nit->fxn==gs.sym_sMinus.get()) {
        nit->df.push_back(MNE_SUB_fff + rt_plus_minus_times(nit->def.at(0), nit->use.at(0), nit->use.at(1)));
        nit->df.push_back(prog.rmap.at(nit->def.at(0)));
        nit->df.push_back(prog.rmap.at(nit->use.at(0)));
        nit->df.push_back(prog.rmap.at(nit->use.at(1)));
        offset+=5;

      } else if (nit->fxn==gs.sym_sDivide.get()) {
        nit->df.push_back(MNE_DIV_fff + rt_divide(nit->def.at(0), nit->use.at(0), nit->use.at(1)));
        nit->df.push_back(prog.rmap.at(nit->def.at(0)));
        nit->df.push_back(prog.rmap.at(nit->use.at(0)));
        nit->df.push_back(prog.rmap.at(nit->use.at(1)));
        offset+=5;

      } else if (nit->fxn==gs.sym_sSin.get()) {
        nit->df.push_back(MNE_SIN_ff + rt_sin_cos(nit->def.at(0), nit->use.at(0)));
        nit->df.push_back(prog.rmap.at(nit->def.at(0)));
        nit->df.push_back(prog.rmap.at(nit->use.at(0)));
        offset+=4;

      } else if (nit->fxn==gs.sym_sCos.get()) {
        nit->df.push_back(MNE_COS_ff + rt_sin_cos(nit->def.at(0), nit->use.at(0)));
        nit->df.push_back(prog.rmap.at(nit->def.at(0)));
        nit->df.push_back(prog.rmap.at(nit->use.at(0)));
        offset+=4;

      } else {
        printf("unhandled case in optimize_makecode\n"); 
        assert(false);
      }

    } else if (nit->type==PNODET_RET) {
      nit->df.push_back(MNE_RET);
      nit->df.push_back(nit->use.size());
      offset+=2;
      for (std::vector<Register>::iterator rit = nit->use.begin(); rit != nit->use.end(); ++rit) {
        u64 hi = (rit->type)>>32;
        u64 lo = (rit->type)&0x0FFFFFFFFULL;
        nit->df.push_back(lo);
        nit->df.push_back(hi);
        nit->df.push_back(prog.rmap.at(*rit));
        offset+=3;
      }

    } else if (nit->type==PNODET_BEG) {
      nit->df.push_back(MNE_BEG);
      nit->df.push_back(nit->def.size());
      offset+=2;
      for (std::vector<Register>::iterator rit = nit->def.begin(); rit != nit->def.end(); ++rit) {
        u64 hi = (rit->type)>>32;
        u64 lo = (rit->type)&0x0FFFFFFFFULL;
        nit->df.push_back(lo);
        nit->df.push_back(hi);
        nit->df.push_back(prog.rmap.at(*rit));
        offset+=3;
      }
      offset+=1;

    } else {
      assert(false);
    }

  }

  for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++nit) {
    for (std::vector<u32>::iterator sit = nit->succ.begin(); sit != nit->succ.end(); ++sit) {
      nit->df.push_back(prog.node.at(*sit).phi.at(0));
    }
  }

printf("making code_list\n");
    
//  expr code_list=(expr)e_alloc(1+(offset+1)/2);
//  set_type(code_list,NODET_DATA);
//  data_setn(code_list,(offset+1)/2);
    ex code_list = emake_bin(4*(offset + 1));
//  u32*p=(u32*)data_getl(code_list);
    u32 * p = (u32*)ebin_data(code_list);
  for (std::vector<PNode>::iterator nit = prog.node.begin(); nit != prog.node.end(); ++nit) {
    for (std::vector<u32>::iterator dfi = nit->df.begin(); dfi != nit->df.end(); ++dfi) {
      *p++=*dfi;
    }
  }

printf("making const_list  koffset=%d  roffset=%d\n",koffset,roffset);
//  expr const_list= (expr)e_alloc(1+roffset/8);
//  set_type(const_list,NODET_DATA);
//  data_setn(const_list,roffset/8);
    ex const_list = emake_bin(8*roffset);
//  char*sp=(char*)data_getl(const_list);
    unsigned char * sp = ebin_data(const_list);
  memset(sp,0,roffset);
  for (std::map<Register,u32>::iterator it=prog.rmap.begin(); it!=prog.rmap.end(); ++it){
    if (it->first.var==0) {continue;}
    copy_regdata(sp+it->second,it->first.type,echild(prog.kdata.kexprl,1+it->first.num));
  }

  
printf("returning code\n");
  return emake_node(ecopy(gs.sym_sCompiledFunction.get()),code_list,const_list);

}


static ex _apply_compiledfunction_error(ex E, size_t off)
{
    uex e(E);
    _gen_message(gs.sym_sCompiledFunction.get(), "cfn", "Numerical error encountered at offset `1`; proceeding with uncompiled evaluation.", emake_int_ui(off));
    er F = echild(E,0,3);
std::cout << "F: " << ex_tostring(F) << std::endl;
std::cout << "E: " << ex_tostring(e.get()) << std::endl;
    e.reset(_apply_function2(echild(F,2), echild(F,1), e.get()));
    return eval(e.release());
}

static ex _apply_compiledfunction_error_arg_int(ex E, size_t idx)
{
    uex e(E);
    _gen_message(gs.sym_sCompiledFunction.get(), "cfsa", "Argument `1` at position `2` should be a machine integer; proceeding with uncompiled evaluation.", ecopychild(e.get(),idx), emake_int_ui(idx));
    er F = echild(E,0,3);
std::cout << "F: " << ex_tostring(F) << std::endl;
    e.reset(_apply_function2(echild(F,2), echild(F,1), e.get()));
    return eval(e.release());
}

static ex _apply_compiledfunction_error_arg_flt(ex E, size_t idx)
{
    uex e(E);
    _gen_message(gs.sym_sCompiledFunction.get(), "cfsa", "Argument `1` at position `2` should be a machine real; proceeding with uncompiled evaluation.", ecopychild(e.get(),idx), emake_int_ui(idx));
    er F = echild(E,0,3);
    e.reset(_apply_function2(echild(F,2), echild(F,1), e.get()));
    return eval(e.release());
}


static void print_r(std::string &s, u32 off)
{
    s.append("r"); s.append(int_tostring(off));
}

class cfprinter
{
    std::vector<uex> finalans;
    std::vector<uex> curline;
    std::string s;
    char * stack;

public:
    cfprinter(char * _stack) : stack(_stack) {}
    void newline()
    {
        curline.push_back(emake_str_move(s));
        s.clear();
        while (curline.size() < 3)
            curline.push_back(emake_str(""));
        while (curline.size() > 3)
            curline.pop_back();
        finalans.push_back(uex(emake_node(ecopy(gs.sym_sList.get()), curline)));
        curline.clear();

std::cout << "newline finalans size: " << finalans.size() << std::endl;

    }
    void tab()
    {
        curline.push_back(emake_str_move(s));
        s.clear();

std::cout << "tab curline size: " << curline.size() << std::endl;

    }
    void rreg(u32 off)
    {
        s.push_back('r');
        s.append(int_tostring(off));
    }
    void ireg(u32 off)
    {
        s.push_back('i');
        s.append(int_tostring(off));
    }
    void gotol(u32 off)
    {
        s.append("Goto[");
        s.append(int_tostring(off));
        s.append("]");
    }
    void append(const char * str)
    {
        s.append(str);
    }
    void append(const std::string &str)
    {
        s.append(str);
    }
    void rdisp(u32 off)
    {
        rreg(off);
        s.push_back(':');
        s.push_back(' ');
        s.append(double_tostring((f64)(*(f64*)(stack+off))));
        s.push_back(' ');
        s.push_back(' ');        
    }
    void idisp(u32 off)
    {
        ireg(off);
        s.push_back(':');
        s.push_back(' ');
        s.append(int_tostring((i64)(*(i64*)(stack+off))));
        s.push_back(' ');
        s.push_back(' ');        
    }
    ex done()
    {
std::cout << "done finalans size: " << finalans.size() << std::endl;

        return emake_node(ecopy(gs.sym_sList.get()), finalans);
    }
};


ex dcode_sCompilePrint(er EE)
{
    uex e(ecopy(EE));
    e.reset(ecopychild(e.get(),1));
    er E = e.get();

std::cout << "E: " << ex_tostring_full(E) << std::endl;

    u32 * inst = (u32*)ebin_data(echild(E,1));
    u32 * end = (u32*)(ebin_data(echild(E,1)) + ebin_size(echild(E,1)));
    char * stack = (char*)ebin_data(echild(E,2));
    u32 * ip = inst;

    cfprinter s(stack);

read_inst:

std::cout << "ip: " << ip << "  end: " << end << std::endl;

    if (ip >= end)
    {
        return s.done();
    }

    s.append(int_tostring(ulong(ip - inst)));
    s.append(": ");
    s.tab();

std::cout << "off: " << ip - inst << std::endl;


    u32 mne = *ip++;

std::cout << "mne: " << mne << std::endl;

    switch (mne)
    {
        case MNE_SIN_ff: printf("MNE_SIN_ff\n");
            s.rreg(*(ip + 0));
            s.append(" = Sin[");
            s.rreg(*(ip + 1));
            s.append("]; ");
            s.gotol(*(ip + 2));
            s.append(";");
            s.tab();
            s.rdisp(*(ip + 1));
            s.newline();
            ip += 3;
            goto read_inst;

        case MNE_SIN_fi: //printf("MNE_SIN_fi\n");
//            *(f64*)(stack+*(ip+0)) = sin((f64)(*(i64*)(stack+*(ip+1))));
//            ip=inst+*(ip+2);
            s.newline();
            ip += 3;
            goto read_inst;

        case MNE_COS_ff: //printf("MNE_COS_ff\n");
//            *(f64*)(stack+*(ip+0)) = sin((f64)(*(f64*)(stack+*(ip+1))));
//            ip=inst+*(ip+2);
            s.rreg(*(ip + 0));
            s.append(" = Cos[");
            s.rreg(*(ip + 1));
            s.append("]; ");
            s.gotol(*(ip + 2));
            s.append(";");
            s.tab();
            s.rdisp(*(ip + 1));
            s.newline();
            ip += 3;
            goto read_inst;

        case MNE_COS_fi: //printf("MNE_COS_fi\n");
//            *(f64*)(stack+*(ip+0)) = sin((f64)(*(i64*)(stack+*(ip+1))));
//            ip=inst+*(ip+2);
            ip += 3;
            goto read_inst;


        case MNE_ADD_fff: //printf("MNE_ADD_fff\n");
//            *(f64*)(stack+*(ip+0)) = (f64)(*(f64*)(stack+*(ip+1))) + (f64)(*(f64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.rreg(*(ip + 0));
            s.append(" = Plus[");
            s.rreg(*(ip + 1));
            s.append(", ");
            s.rreg(*(ip + 2));
            s.append("]; ");
            s.gotol(*(ip + 3));
            s.append(";");
            s.tab();
            s.rdisp(*(ip + 1));
            s.rdisp(*(ip + 2));
            s.newline();
            ip += 4;
            goto read_inst;

        case MNE_ADD_fif: //printf("MNE_ADD_fif\n");
//            *(f64*)(stack+*(ip+0)) = (f64)(*(i64*)(stack+*(ip+1))) + (f64)(*(f64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.rreg(*(ip + 0));
            s.append(" = Plus[");
            s.ireg(*(ip + 1));
            s.append(", ");
            s.rreg(*(ip + 2));
            s.append("]; ");
            s.gotol(*(ip + 3));
            s.append(";");
            s.tab();
            s.idisp(*(ip + 1));
            s.rdisp(*(ip + 2));
            s.newline();
            ip += 4;
            goto read_inst;

        case MNE_ADD_ffi: //printf("MNE_ADD_fif\n");
//            *(f64*)(stack+*(ip+0)) = (f64)(*(f64*)(stack+*(ip+1))) + (f64)(*(i64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.newline();
            ip += 4;
            goto read_inst;

        case MNE_ADD_iii: //printf("MNE_ADD_iii\n");
//            *(i64*)(stack+*(ip+0)) = (i64)(*(i64*)(stack+*(ip+1))) + (i64)(*(i64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.newline();
            ip += 4;
            goto read_inst;



        case MNE_SUB_fff:
//            *(f64*)(stack+*(ip+0)) = (f64)(*(f64*)(stack+*(ip+1))) - (f64)(*(f64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.newline();
            ip += 4;
            goto read_inst;

        case MNE_SUB_fif:
//            *(f64*)(stack+*(ip+0)) = (f64)(*(i64*)(stack+*(ip+1))) - (f64)(*(f64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.newline();
            ip += 4;
            goto read_inst;

        case MNE_SUB_ffi:
//            *(f64*)(stack+*(ip+0)) = (f64)(*(f64*)(stack+*(ip+1))) - (f64)(*(i64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.newline();
            ip += 4;
            goto read_inst;

        case MNE_SUB_iii:
//            *(i64*)(stack+*(ip+0)) = (i64)(*(i64*)(stack+*(ip+1))) - (i64)(*(i64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.newline();
            ip += 4;
            goto read_inst;


        case MNE_MUL_fff: //printf("MNE_MULL_fff\n");
//            *(f64*)(stack+*(ip+0)) = fr = (f64)(*(f64*)(stack+*(ip+1))) * (f64)(*(f64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.rreg(*(ip + 0));
            s.append(" = Times[");
            s.rreg(*(ip + 1));
            s.append(", ");
            s.rreg(*(ip + 2));
            s.append("]; ");
            s.gotol(*(ip + 3));
            s.append(";");
            s.tab();
            s.rdisp(*(ip + 1));
            s.rdisp(*(ip + 2));
            s.newline();
            ip += 4;
            goto read_inst;

        case MNE_MUL_fif: //printf("MNE_MULL_fif\n");
//            *(f64*)(stack+*(ip+0)) = (f64)(*(i64*)(stack+*(ip+1))) * (f64)(*(f64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.rreg(*(ip + 0));
            s.append(" = Times[");
            s.ireg(*(ip + 1));
            s.append(", ");
            s.rreg(*(ip + 2));
            s.append("]; ");
            s.gotol(*(ip + 3));
            s.append(";");
            s.tab();
            s.idisp(*(ip + 1));
            s.rdisp(*(ip + 2));
            s.newline();
            ip += 4;
            goto read_inst;

        case MNE_MUL_ffi: //printf("MNE_MULL_ffi\n");
//            *(f64*)(stack+*(ip+0)) = (f64)(*(f64*)(stack+*(ip+1))) * (f64)(*(i64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.rreg(*(ip + 0));
            s.append(" = Times[");
            s.rreg(*(ip + 1));
            s.append(", ");
            s.ireg(*(ip + 2));
            s.append("]; ");
            s.gotol(*(ip + 3));
            s.append(";");
            s.tab();
            s.rdisp(*(ip + 1));
            s.idisp(*(ip + 2));
            s.newline();
            ip += 4;
            goto read_inst;

        case MNE_MUL_iii: //printf("MNE_MULL_iii\n");
//            *(i64*)(stack+*(ip+0)) = (i64)(*(i64*)(stack+*(ip+1))) * (i64)(*(i64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.newline();
            ip += 4;
            goto read_inst;



        case MNE_DIV_fff:
//            *(f64*)(stack+*(ip+0)) = (f64)(*(f64*)(stack+*(ip+1))) / (f64)(*(f64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.newline();
            ip += 4;
            goto read_inst;

        case MNE_DIV_fif:
//            *(f64*)(stack+*(ip+0)) = (f64)(*(i64*)(stack+*(ip+1))) / (f64)(*(f64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.newline();
            ip += 4;
            goto read_inst;

        case MNE_DIV_ffi:
//            *(f64*)(stack+*(ip+0)) = (f64)(*(f64*)(stack+*(ip+1))) / (f64)(*(i64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.newline();
            ip += 4;
            goto read_inst;

        case MNE_DIV_fii:
//            *(f64*)(stack+*(ip+0)) = (i64)(*(i64*)(stack+*(ip+1))) / (i64)(*(i64*)(stack+*(ip+2)));
//            ip=inst+*(ip+3);
            s.newline();
            ip += 4;
            goto read_inst;
        


        case MNE_BEG:
        {
    //      printf("MNE_BEG\n");
            s.append("InitArgs[");
            u32 argc = *ip++;
            for (u32 i=0; i<argc; i++)
            {
                u32 lo=*ip++;
                u32 hi=*ip++;
                u32 loc =*ip++;
                assert(lo == 0);
                if (hi == REGT_FLT)
                {
                    s.rreg(loc);
                }
                else if (hi == REGT_INT)
                {
                    s.ireg(loc);
                }
                else
                {
                    assert(false);
                }
                if (i+1<argc)
                    s.append(", ");
            }
            s.append("]; ");
            s.gotol(*(ip));
            s.append(";");
            s.newline();
            ip++;
            goto read_inst;
        }

        case MNE_RET:
        {
            s.append("Return[");

            u32 retc = *ip++;
            assert(retc == 1);
            for (u32 i=0; i<retc; i++)
            {
                u32 lo = *ip++;
                u32 hi = *ip++;
                u32 loc =*ip++;
                assert(lo == 0);
                if (hi == REGT_FLT)
                {
                    s.rreg(loc);
                }
                else if (hi == REGT_INT)
                {
                    s.ireg(loc);
                }
                else
                {
                    assert(false);
                }
                if (i+1<retc)
                    s.append(", ");
            }
            s.append("];");
            s.newline();
            ip++;
            goto read_inst;
        }
         
        default:
        {
            assert(false);
            goto read_inst;
        }
    }
}


ex apply_compiledfunction(ex E)
{
    uex e(E);
    f64 fr;
    i64 ir;

    u32 * inst = (u32*)ebin_data(echild(E,0,1));
    char * stack = (char*)ebin_data(echild(E,0,2));
    u32 * ip = inst;

read_inst:
  
    u32 mne = *ip++;
    switch (mne)
    {
        case MNE_SIN_ff: //printf("MNE_SIN_ff\n");
            *(f64*)(stack+*(ip+0)) = sin((f64)(*(f64*)(stack+*(ip+1))));
            ip=inst+*(ip+2);
            goto read_inst;
        case MNE_SIN_fi: //printf("MNE_SIN_fi\n");
            *(f64*)(stack+*(ip+0)) = sin((f64)(*(i64*)(stack+*(ip+1))));
            ip=inst+*(ip+2);
            goto read_inst;

        case MNE_COS_ff: //printf("MNE_COS_ff\n");
            *(f64*)(stack+*(ip+0)) = cos((f64)(*(f64*)(stack+*(ip+1))));
            ip=inst+*(ip+2);
            goto read_inst;
        case MNE_COS_fi: //printf("MNE_COS_fi\n");
            *(f64*)(stack+*(ip+0)) = cos((f64)(*(i64*)(stack+*(ip+1))));
            ip=inst+*(ip+2);
            goto read_inst;


        case MNE_ADD_fff: /*printf("MNE_ADD_fff\n");*/
            *(f64*)(stack+*(ip+0)) = (f64)(*(f64*)(stack+*(ip+1))) + (f64)(*(f64*)(stack+*(ip+2)));
            ip=inst+*(ip+3);
            goto read_inst;

        case MNE_ADD_fif: //printf("MNE_ADD_fif\n");
            *(f64*)(stack+*(ip+0)) = (f64)(*(i64*)(stack+*(ip+1))) + (f64)(*(f64*)(stack+*(ip+2)));
            ip=inst+*(ip+3);
            goto read_inst;

        case MNE_ADD_ffi: //printf("MNE_ADD_fif\n");
            *(f64*)(stack+*(ip+0)) = (f64)(*(f64*)(stack+*(ip+1))) + (f64)(*(i64*)(stack+*(ip+2)));
            ip=inst+*(ip+3);
            goto read_inst;

        case MNE_ADD_iii: //printf("MNE_ADD_iii\n");
            *(i64*)(stack+*(ip+0)) = (i64)(*(i64*)(stack+*(ip+1))) + (i64)(*(i64*)(stack+*(ip+2)));
            ip=inst+*(ip+3);
            goto read_inst;



        case MNE_SUB_fff:
            *(f64*)(stack+*(ip+0)) = (f64)(*(f64*)(stack+*(ip+1))) - (f64)(*(f64*)(stack+*(ip+2)));
            ip=inst+*(ip+3);
            goto read_inst;

        case MNE_SUB_fif:
            *(f64*)(stack+*(ip+0)) = (f64)(*(i64*)(stack+*(ip+1))) - (f64)(*(f64*)(stack+*(ip+2)));
            ip=inst+*(ip+3);
            goto read_inst;

        case MNE_SUB_ffi:
            *(f64*)(stack+*(ip+0)) = (f64)(*(f64*)(stack+*(ip+1))) - (f64)(*(i64*)(stack+*(ip+2)));
            ip=inst+*(ip+3);
            goto read_inst;

        case MNE_SUB_iii:
            *(i64*)(stack+*(ip+0)) = (i64)(*(i64*)(stack+*(ip+1))) - (i64)(*(i64*)(stack+*(ip+2)));
            ip=inst+*(ip+3);
            goto read_inst;


        case MNE_MUL_fff: //printf("MNE_MULL_fff\n");
            *(f64*)(stack+*(ip+0)) = fr = (f64)(*(f64*)(stack+*(ip+1))) * (f64)(*(f64*)(stack+*(ip+2)));
/*
            if (!std::isfinite(fr))
            {
                return _apply_compiledfunction_error(e.release(), (ip - inst) - 1);
            }
*/
            ip=inst+*(ip+3);
            goto read_inst;

        case MNE_MUL_fif: //printf("MNE_MULL_fif\n");
            *(f64*)(stack+*(ip+0)) = (f64)(*(i64*)(stack+*(ip+1))) * (f64)(*(f64*)(stack+*(ip+2)));
            ip=inst+*(ip+3);
            goto read_inst;

        case MNE_MUL_ffi: //printf("MNE_MULL_ffi\n");
            *(f64*)(stack+*(ip+0)) = (f64)(*(f64*)(stack+*(ip+1))) * (f64)(*(i64*)(stack+*(ip+2)));
            ip=inst+*(ip+3);
            goto read_inst;

        case MNE_MUL_iii: //printf("MNE_MULL_iii\n");
            *(i64*)(stack+*(ip+0)) = (i64)(*(i64*)(stack+*(ip+1))) * (i64)(*(i64*)(stack+*(ip+2)));
            ip=inst+*(ip+3);
            goto read_inst;



        case MNE_DIV_fff:
            *(f64*)(stack+*(ip+0)) = (f64)(*(f64*)(stack+*(ip+1))) / (f64)(*(f64*)(stack+*(ip+2)));
            ip=inst+*(ip+3);
            goto read_inst;

        case MNE_DIV_fif:
            *(f64*)(stack+*(ip+0)) = (f64)(*(i64*)(stack+*(ip+1))) / (f64)(*(f64*)(stack+*(ip+2)));
            ip=inst+*(ip+3);
            goto read_inst;

        case MNE_DIV_ffi:
            *(f64*)(stack+*(ip+0)) = (f64)(*(f64*)(stack+*(ip+1))) / (f64)(*(i64*)(stack+*(ip+2)));
            ip=inst+*(ip+3);
            goto read_inst;

        case MNE_DIV_fii:
            *(f64*)(stack+*(ip+0)) = (i64)(*(i64*)(stack+*(ip+1))) / (i64)(*(i64*)(stack+*(ip+2)));
            ip=inst+*(ip+3);
            goto read_inst;
        


        case MNE_BEG:
        {
    //      printf("MNE_BEG\n");
            u32 argc=*ip++;
            for (u32 i=0; i<argc; i++)
            {
                u32 lo=*ip++;
                u32 hi=*ip++;
                u32 loc =*ip++;
                assert(lo == 0);
                if (hi == REGT_FLT)
                {
                    if (eis_double(echild(E,i+1)))
                    {
                        *(f64*)(stack+loc) = edouble_number(echild(E,i+1));
                    }
                    else if (eis_int(echild(E,i+1)))
                    {
                        *(f64*)(stack+loc)=fmpz_get_d(eint_data(echild(E,i+1)));
                    }
                    else
                    {
                        return _apply_compiledfunction_error_arg_flt(e.release(), i+1);
                    }
                }
                else if (hi == REGT_INT)
                {
                    if (eis_int(echild(E,i+1)))
                    {
                        if (fmpz_fits_si(eint_data(echild(E,i+1))))
                        {
                            *(i64*)(stack+loc)=fmpz_get_si(eint_data(echild(E,i+1)));
                        }
                        else
                        {
                            return _apply_compiledfunction_error_arg_int(e.release(), i+1);
                        }
                    }
                    else if (eis_double(echild(E,i+1)))
                    {
                        *(i64*)(stack+loc) = edouble_number(echild(E,i+1));
                    }
                    else
                    {
                        return _apply_compiledfunction_error_arg_int(e.release(), i+1);
                    }
                }
                else
                {
                    assert(false);
                }
            }

            ip = inst+*ip;
            goto read_inst;
        }

        case MNE_RET:
        {
            u32 retc = *ip++;
            assert(retc == 1);
            for (u32 i=0; i<retc; i++)
            {
                u32 lo = *ip++;
                u32 hi = *ip++;
                u32 loc =*ip++;
                if (hi == REGT_FLT && lo == 0)
                {
//                    printf("ret(%d): %f\n",i,(f64)(*(f64*)(stack+loc)));
                    return emake_double((f64)(*(f64*)(stack+loc)));
                }
                else if (hi == REGT_INT && lo == 0)
                {
//                    printf("ret(%d): %d\n",i,(i64)(*(i64*)(stack+loc)));
                    return emake_int_si((i64)(*(i64*)(stack+loc)));
                }
                else
                {
                    assert(false);
                    goto read_inst;
                }
            }
            assert(false);
            goto read_inst;
        }
         
        default:
        {
            assert(false);
            goto read_inst;
        }
    }
}




ex dcode_sCompile(er e)
{
//std::cout << "dcode_sCompile: " << ex_tostring_full(e) << std::endl;
    assert(ehas_head_sym(e, gs.sym_sCompile.get()));

    if (elength(e) != 2)
    {
        _gen_message(gs.sym_sCompile.get(), "argx2", NULL, gs.sym_sBlock.copy(), emake_int_ui(elength(e)));
        return ecopy(e);
    }

    er e1 = echild(e, 1);
    if (!ehas_head_sym(e1, gs.sym_sList.get()))
    {
        _gen_message(gs.sym_sCompile.get(), "arglist", "argument specification `1` is not a List.", ecopy(e1));
        return ecopy(e);
    }

    for (size_t i = 1; i <= elength(e1); i++)
    {
        if (  !ehas_head_sym_length(echild(e1,i), gs.sym_sPattern.get(), 2)
            || !eis_sym(echild(e1,i,1))
            || exprtype_to_rtype(echild(e1,i,2))==0
           )
        {
            _gen_message(gs.sym_sCompile.get(), "badarg", "argument `1` is invalid.", ecopychild(e1,i));
            return ecopy(e);        
        }
    }

    u64 sp = 0;
    u64 argc = elength(e1);
    std::vector<uex> arglist;
    std::vector<u64> vlist;
    std::vector<u64> rlist;
    for (size_t i = 1; i <= elength(e1); i++)
    {
        arglist.push_back(uex(ecopychild(e1,i,1)));
      vlist.push_back(sym_getc(echild(e1,i,1)));
      vlist.push_back(exprtype_to_rtype(echild(e1,i,2)));
      vlist.push_back(sp);
      sp++;
    }
    assert(sp=argc);
    
    
    printf("\n ******* making program list ******* \n");
    
    CProgram prog;
    prog.kdata.kexprl = emake_node(ecopy(gs.sym_sList.get()));

    std::list<PNode> prognodes;
    u64 retc = expr_compilelist(prognodes,rlist,vlist,prog.kdata,echild(e,2),argc)-argc;
    assert(rlist.size() == retc);
    PNode x(PNODET_BEG);
    for (u64 i=0; i<argc; i++) {x.def.push_back(Register(vlist.at(3*i+1),i,0));}
    prognodes.push_front(x);
    PNode y(PNODET_RET);
    for (u64 i=0; i<retc; i++) {y.use.push_back(Register(rlist.at(i),argc+i,0));}
    prognodes.push_back(y);

    printf("\n ******* printing program list before move ******* \n");
    for (std::list<PNode>::iterator i = prognodes.begin(); i != prognodes.end(); i++) {pnPrintOut(*i);}
    printf("\n");

    prog.node.reserve(10+prognodes.size());
    for (std::list<PNode>::iterator i = prognodes.begin(), end = prognodes.end(); i != end; ++i) {
      prog.node.push_back(*i);
    }

    printf("\n ******* printing program list after move ******* \n");
    for (std::list<PNode>::iterator i = prognodes.begin(); i != prognodes.end(); i++) {pnPrintOut(*i);}
    printf("\n");

    
    printf("\n ******* printing program \n");
    cpPrintOut(prog);

    printf("\n ******* compute_pred_succ \n");
    compute_pred_succ(prog);
    cpPrintOut(prog);

    printf("\n ******* compute_reverse_post_order \n");
    compute_reverse_post_order(prog);
    cpPrintOut(prog);

    printf("\n ******* compute_idom \n");
    compute_idom(prog);
    cpPrintOut(prog);

    printf("\n ******* compute_df \n");
    compute_df(prog);
    cpPrintOut(prog);
    
    printf("\n ******* compute_ssa \n");
    compute_ssa(prog);
    cpPrintOut(prog);
    
    printf("\n ******* optimize_ssa \n");
    optimize_ssa(prog);
    cpPrintOut(prog);

    printf("\n ******* optimize_regalloc \n");
    optimize_regalloc(prog);
    cpPrintOut(prog);

    printf("\n ******* optimize_makecode \n");
    ex ce = optimize_makecode(prog);
    cpPrintOut(prog);

    printf("\n *******    done                 ******* \n");
    eclear(prog.kdata.kexprl);


    printf("\noriginal program: "); std::cout << ex_tostring_full(e); printf("\n");
//    ce = emake_node(ce,emake_cint(3),emake_cint(5));
    printf("code: "); std::cout << ex_tostring_full(etor(ce)); printf("\n");
/*
    printf("*******testing compiled function on inputs (3,5)*******\n");
    
    for (i64 i=0; i<1; i++) {
      compile_run(ce);
    }
*/
    
    ce = eappend(ce, emake_node(gs.sym_sFunction.copy(),
                                 emake_node(gs.sym_sList.copy(), arglist),
                                 ecopychild(e,2)));

    return ce;
}




ex scode_sCompiledFunction(er e)
{
//std::cout << "scode_sCompiledFunction: " << ex_tostring_full(e) << std::endl;
    assert(eis_node(e));

    er h = echild(e,0);
    if (ehas_head_sym_length(h, gs.sym_sCompiledFunction.get(), 3))
    {
        return apply_compiledfunction(ecopy(e));
    }
    else
    {
        return ecopy(e);
    }
}
