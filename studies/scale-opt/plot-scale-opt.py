#from ROOT import TH2D, TCanvas, gROOT, gStyle, TLine
#import ROOT
from array import array as arr
import sys

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
#sns.set_theme()
sns.set_style("whitegrid",{'grid.linestyle':''})

def fix(df_):

    vlow=1.
    count=0
    for i in range(1, 7):
        ilow = i - vlow
        vhigh=1.
        for j in range(1, 7):
            jup = j - vhigh

            exist=((df_['lfrc'] == round(ilow,2)) & (df_['ufrc'] == round(jup,2))).any()
            
            #print("ilow : ", round(ilow,2), " ; jup : ", round(jup,2)," is in dataframe : ", exist )
            
            if not exist:
                df_ = df_._append(
                    pd.Series(
                        {
                            'calo'  : np.nan,
                            'xtal'  : np.nan,
                            'lfrc'  : round(ilow,2),
                            'ufrc'  : round(jup,2),
                            'range' : np.nan,
                            'scale' : np.nan,
                            'dist'  : np.nan
                        }
                    ),
                    ignore_index=True
                )
            
            vhigh+=0.8
            count+=1;
        vlow+=0.8
        
    return df_

pass

def dummy(df_, val=[0.5,1.2]):

    for ival in val:
        df_ = df_.replace(ival,np.nan)

    return df_

if __name__=="__main__":
    
    filename = sys.argv[1]
    outpath  = sys.argv[2]
    #scale_calo10-xtal36_rw_run4f.txt
    config=filename.split('_')[1]

    cols=['calo','xtal','lfrc','ufrc','range','scale','dist']
    df = pd.read_csv(filename, sep=" ", header=None,names=cols)
    df = df.reset_index()
    frange=df["range"][0]
    
    #for index, row in df.iterrows():
    #    print(row['lfrc'],row['ufrc'])

    # fix missign data
    df = fix(df)
    #print(df)
    
    # x ; y
    data_matrix = df.pivot(index="ufrc",columns="lfrc",values="scale")
    #print(data_matrix)

    # statistics
    sdf = dummy(df)
    mean = round(sdf['scale'].mean(), 3)
    std =  round(sdf['scale'].std(), 3)
    print(mean)
    #print("Mean : ", mean)
    #print("S.D : ", std)

    # draw heatmap
    f, ax = plt.subplots(figsize=(9, 6))
    
    hm = sns.heatmap(data_matrix, annot=True, fmt='.2f', linewidths=.5, annot_kws={"size": 15}, vmin=0, vmax=1.2, ax=ax)
    hm.invert_yaxis()
    hm.set_xlabel("Lower edge range (fraction)", fontsize = 15)
    hm.set_ylabel("Upper edge range (fraction)", fontsize = 15)
    hm.figure.axes[-1].set_ylabel('KS-optimized scale', size=15)
    plt.title("%s ; full range : %s MeV; Mean scale : %s" %(config,frange, mean), fontsize=20)
    #hm.text(0.6, 0.2, "Mean : ", mean )
    #plt.show()
    plt.savefig('%s/%s.png' %(outpath,config))
