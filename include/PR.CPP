//Reshenie SLAU - metod polnogo iskl'ucheniya peremennih (po prav. pr'amoug.)
#include <iostream.h>
#include <conio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef PR_INCLUDED
#include "pr.h"
#define PR_INCLUDED
#endif

void SLAU_rectangle(double **A,int n,int m,double &X,char mode)
{
 int i=0,j=0,k=0,p=0,t=0,ir=0,jr=0;
 double **An=NULL,**Ap=NULL,razresh=1;
 NewMatr(Ap,n,m);
 NewMatr(An,n,m);
 Copy(A,Ap,n,m);

 for(k=0; k<n; k++)
 {
  Print(Ap,n,m);
  if (mode=='m')
  {
   for(i=0; ; i++) {
    cout<<"\nVvedite noviy razresh. element:\n";
    razresh = InputNum();
    if (Scan(Ap,n,razresh,ir,jr))
    { razresh = Ap[ir][jr];  break;}
    else if (i <= 5)
     cout<<"Danniy el-t ne naiden v matritse. Povtorite vvod!\n";
    else {
     cout<<"Danniy el-t ne naiden v matritse. Vvedite ego koordinati:\n";
     ir = InputNum();   jr = InputNum();
     if(ir>0 && jr>0 && ir<n && jr<m)  {razresh = Ap[ir][jr]; break;}
    }
   }
  }
  else if (mode=='a')
  {
   ir = k;	jr = k;
   razresh = Ap[ir][jr];
  }
  //------------------- nachalo algoritma polnogo iskl'ucheniya peremennih --
  for(i=0; i<n; i++)
  {
   for(j=0; j<m; j++)
   {
    if (i == ir)
    {
     An[i][j] = Ap[i][j] / razresh;
    }
    else if (j == jr)
	  An[i][j] = 0;
    else		//po pravilu pr'amougol'nika
    {
     if (i < ir && j < jr)
     {
      An[i][j] = Pryam(Ap[i][j],Ap[ir][j],Ap[i][jr],Ap[ir][jr],razresh);
     }
     else if (i < ir && j > jr)
     {
      An[i][j] = Pryam(Ap[ir][j],Ap[ir][jr],Ap[i][j],Ap[i][jr],razresh);
     }
     else if (i > ir && j > jr)
     {
      An[i][j] = Pryam(Ap[ir][jr],Ap[ir][j],Ap[i][jr],Ap[i][j],razresh);
     }
     else if (i > ir && j < jr)
     {
      An[i][j] = Pryam(Ap[i][jr],Ap[i][j],Ap[ir][jr],Ap[ir][j],razresh);
     }
    }
    Print(An,n,m);
   }
  }
  Copy(An,Ap,n,m);   Clear(An,n,m);
 }
 //--------------------------------- konets osnovnogo algoritma -------------

  for(i=0; i<n; i++)	//rasstanovka korney v pravil'nom por'adke
  {
   for(j=0; j<m; j++)
   {
    if (Ap[i][j] == 1)
    {
     X[j] = Ap[i][m-1];   break;
    }
   }
  }
  delete[] Ap;  delete[] An;

}
//===========================================================================


double InputNum()	//Vvod chisla s proverkoy
  {
   char elem[10];
   while(1) {
    cin>>elem;
    double num = atof(elem);
    if (num || elem[0]=='0') return num;
    cout<<"\nNeverniy vvod! Vvedite chislo ('x' - vihod):\n";
    if (elem[0]=='x') exit(1);
   }
  }
//------------------------------------------------------

void Copy(double **(&B),double **(&C),int n,int m)
{
 if (!m) n=m;
 for (int i=0; i<n; i++)
  for (int j=0; j<m; j++)
  {
   C[i][j] = B[i][j];
  }
}
//-----------------------------------------------

void Print(double **(&B),int n,int m)
{
 if (!m) n=m;
 for (int i=0; i<n; i++)
 {
  for (int j=0; j<m; j++)
   printf("%8.3f",B[i][j]);
  cout<<"\n\n";
 }
 cout<<"\n";
}
//-------------------------------------------

void Input(double **(&B),int n,int m)
{
 if (!m) n=m;
 for (int i=0; i<n; i++)
 {
  for (int j=0; j<m; j++)
   B[i][j]=InputNum();
 }
}
//---------------------------------------------

void NewMatr(double **(&B),int n,int m)
{
 if (!m) n=m;
 B = new double* [n];
 for (int i=0; i<n; i++)
 {
  B[i] = new double [m];
  for (int j=0; j<m; j++)
   B[i][j]=0; 
 }
}
//--------------------------------------------

void Clear(double **(&B),int n,int m)
{
 if (!m) n=m;
 for (int i=0; i<n; i++)
  for (int j=0; j<m; j++)
  {
   if (j==i)  B[i][j]=1;
   else B[i][j]=0;
  }
}

//---------------------------------------------------------------------------

double Pryam(double a,double c,double b,double d,double razresh)
 {
  double rez=0;
  if (a == razresh || d == razresh)
  {
   rez = (a * d - b * c) / razresh;
  }
  else if (b == razresh || c == razresh)
  {
   rez = (b * c - a * d) / razresh;
  }
  return rez;
 }
//--------------------------------------------------------------------

int Scan(double **B,int n,double elem, int &ix,int &jx)
{
 int flag=0;
 for(int i=0; i<n; i++)
 {
  for(int j=0; j<n; j++)
  {
   if (fabs(B[i][j] - elem) < epsilon)
   {
    ix = i; jx = j;  return 1;
   }
  }
 }
 return 0;
}
