void test(){

int ntot = 5;
int charge[5] = {1,-1,1,1,-1};

for (int iP1 = 0; iP1 <ntot; iP1 ++){
  if (charge[iP1] < 0) continue;
    for(int iN1 = 0; iN1 <ntot; iN1++){
      if (charge[iN1] > 0) continue;
//      cout<<"pair"<<iN1<<","<<iP1<<endl;
      for(int iP2 = iP1 + 1; iP2 <ntot; iP2++){
        if (charge[iP2] < 0) continue;
        cout<<iP1<<","<<iN1<<","<<iP2<<endl;
//        cout<<charge[iP1]<<","<<charge[iN1]<<","<<charge[iP2]<<endl;
      }
      for(int iN2 = iN1 + 1; iN2 <ntot; iN2++){
        if (charge[iN2] > 0) continue;
        cout<<iP1<<","<<iN1<<","<<iN2<<endl;
//        cout<<charge[iP1]<<","<<charge[iN1]<<","<<charge[iN2]<<endl;
      }
    }
}
cout<<"next"<<endl;

for (int i1 = 0; i1 <ntot; i1 ++){
    for(int i2 = 0; i2 <ntot; i2++){
      if (charge[i1] * charge[i2] > 0 )  continue;
//      cout<<"pair"<<i1<<","<<i2<<endl;
      for(int i3 = i1 + 1; i3 <ntot; i3++){
        if (charge[i2] * charge[i3] > 0) continue;
        cout<<i1<<","<<i2<<","<<i3<<endl;
//        cout<<charge[i1]<<","<<charge[i2]<<","<<charge[i3]<<endl;
      }
    }
}
}
